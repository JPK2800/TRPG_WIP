// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatGameMode.h"
#include "TileControlPawn.h"
#include "EventDataActor.h"

void ACombatGameMode::BeginPlay()
{
	Super::BeginPlay();

}

void ACombatGameMode::BeginFirstPhase()
{
	LinkToEventDataActor();

	TriggerPreCombatLogic();
}

void ACombatGameMode::BeginNextCombatPhase()
{
	if (!EventData)
	{
		UE_LOG(LogTemp, Warning, TEXT("CALLED BeginNextCombatPhase IN CombatGameMode BUT EventData WAS NULL!"));
		return;
	}

	switch (CurrentCombatPhase)
	{
		case(NO_PHASE):
		case(BEFORE_COMBAT):
			ActivateCombatPhase(EventData->FirstCombatPhase);
			return;
		case(AFTER_COMBAT):
			// do nothing
			return;
		case(TRANSITION_PLAYER_PHASE):
			ActivateCombatPhase(PLAYER_PHASE);
			return;
		case(TRANSITION_PARTNER_PHASE):
			ActivateCombatPhase(PARTNER_PHASE);
			return;
		case(TRANSITION_ENEMY_PHASE):
			ActivateCombatPhase(ENEMY_PHASE);
			return;
		case(TRANSITION_NPC_PHASE):
			ActivateCombatPhase(NPC_PHASE);
			return;
	}

	uint8 playerUnitCount, partnerUnitCount, enemyUnitCount, npcUnitCount;
	CountUnitsByAllegiance(playerUnitCount, partnerUnitCount, enemyUnitCount, npcUnitCount);

	// cycles between the different phases in order from PLAYER -> PARTNER -> ENEMY -> NPC. Skips phases that have no units.
	if (partnerUnitCount > 0 && CurrentCombatPhase < TRANSITION_PARTNER_PHASE) 
	{
		ActivateCombatPhase(TRANSITION_PARTNER_PHASE);
		return;
	}
	else if (enemyUnitCount > 0 && CurrentCombatPhase < TRANSITION_ENEMY_PHASE)
	{
		ActivateCombatPhase(TRANSITION_ENEMY_PHASE);
		return;
	}
	else if (npcUnitCount > 0 && CurrentCombatPhase < TRANSITION_NPC_PHASE)
	{
		ActivateCombatPhase(TRANSITION_NPC_PHASE);
		return;
	}
	else if (playerUnitCount > 0)
	{
		ActivateCombatPhase(TRANSITION_PLAYER_PHASE);
		return;
	}
	else if (partnerUnitCount > 0)
	{
		ActivateCombatPhase(TRANSITION_PARTNER_PHASE);
		return;
	}
	else if (enemyUnitCount > 0)
	{
		ActivateCombatPhase(TRANSITION_ENEMY_PHASE);
		return;
	}
	else if (npcUnitCount > 0)
	{
		ActivateCombatPhase(TRANSITION_NPC_PHASE);
		return;
	}
	else 
	{
		ActivateCombatPhase(GAME_OVER);
	}

}

void ACombatGameMode::EndCombatPhases()
{
	ActivateCombatPhase(ECombatPhase::AFTER_COMBAT);
}

void ACombatGameMode::BeginPauseForEvent(ACombatEvent* Event, ECombatPhase PausedCombatPhase)
{
	IsPausedForEvent = true;
	CurrentCombatPhase = ECombatPhase::NO_PHASE;
	QueuedPhaseAfterEvent = PausedCombatPhase;
	OnPausePhase.Broadcast(true);
	CurrentEvent = Event;
	CurrentEvent->OnEventEnded.AddDynamic(this, &ACombatGameMode::EndPauseForEvent);	// end pause when event ends
}

void ACombatGameMode::EndPauseForEvent()
{
	IsPausedForEvent = false;
	CurrentEvent->OnEventEnded.RemoveDynamic(this, &ACombatGameMode::EndPauseForEvent);	// remove binding to this event
	OnPausePhase.Broadcast(false);

	ActivateCombatPhase(QueuedPhaseAfterEvent);
}

ATileControlPawn* ACombatGameMode::GetControlPawn()
{
	auto* tileControlPawn = UGameplayStatics::GetActorOfClass(GetWorld(), ATileControlPawn::StaticClass());
	if (tileControlPawn)
	{
		return Cast<ATileControlPawn>(tileControlPawn);
	}
	return nullptr;
}

bool ACombatGameMode::LinkToEventDataActor()
{
	AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AEventDataActor::StaticClass());
	if (foundActor)
	{
		EventData = Cast<AEventDataActor>(foundActor);

		if (EventData)
			return true;
	}

	return false;
}

void ACombatGameMode::TriggerPreCombatLogic()
{
	ActivateCombatPhase(ECombatPhase::BEFORE_COMBAT);
}

void ACombatGameMode::PrepareUnitsOnPhaseShift(ECombatPhase NewPhase, ECombatPhase PreviousPhase)
{
	// Find all units
	TArray<AGameUnit*> playerUnits = TArray<AGameUnit*>(), partnerUnits = TArray<AGameUnit*>(), enemyUnits = TArray<AGameUnit*>(), npcUnits = TArray<AGameUnit*>();
	TArray<AActor*> foundUnitActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameUnit::StaticClass(), foundUnitActors);

	// Sort units by allegiance
	for (auto* actor : foundUnitActors)
	{
		AGameUnit* foundUnit = Cast<AGameUnit>(actor);
		if (foundUnit)
		{
			uint8 unitFaction = foundUnit->UnitFaction;
			switch (unitFaction)
			{
				case (EUnitFaction::PLAYER):
					playerUnits.Add(foundUnit);
					break;
				case (EUnitFaction::PARTNER):
					partnerUnits.Add(foundUnit);
					break;
				case (EUnitFaction::ENEMY):
					enemyUnits.Add(foundUnit);
					break;
				case (EUnitFaction::NPC):
					npcUnits.Add(foundUnit);
					break;
			}
		}
	}

	// Deactivate units from the previously active phase
	switch (PreviousPhase) {
		case (PLAYER_PHASE):
			for (auto* unit : playerUnits)
			{
				unit->DeactivateUnitOnPhaseEnd();
			}
			break;
		case (PARTNER_PHASE):
			for (auto* unit : partnerUnits)
			{
				unit->DeactivateUnitOnPhaseEnd();
			}
			break;
	
		case (ENEMY_PHASE):
			for (auto* unit : enemyUnits)
			{
				unit->DeactivateUnitOnPhaseEnd();
			}
			break;
		case (NPC_PHASE):
			for (auto* unit : npcUnits)
			{
				unit->DeactivateUnitOnPhaseEnd();
			}
			break;
	}

	// Activate units from the previously active phase
	switch (NewPhase) {
		case (PLAYER_PHASE):
			for (auto* unit : playerUnits)
			{
				unit->ActivateUnitOnPhaseStart();
			}
			break;
		case (PARTNER_PHASE):
			for (auto* unit : partnerUnits)
			{
				unit->ActivateUnitOnPhaseStart();
			}
			break;

		case (ENEMY_PHASE):
			for (auto* unit : enemyUnits)
			{
				unit->ActivateUnitOnPhaseStart();
			}
			break;
		case (NPC_PHASE):
			for (auto* unit : npcUnits)
			{
				unit->ActivateUnitOnPhaseStart();
			}
			break;
	}
	
}

void ACombatGameMode::CountUnitsByAllegiance(uint8& PlayerUnitCount, uint8& PartnerUnitCount, uint8& EnemyUnitCount, uint8& NpcUnitCount)
{
	uint8 playerCount = 0, partnerCount = 0, enemyCount = 0, npcCount = 0;
	TArray<AActor*> foundUnitActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameUnit::StaticClass(), foundUnitActors);

	for (auto* actor : foundUnitActors)
	{
		AGameUnit* foundUnit = Cast<AGameUnit>(actor);
		if (foundUnit)
		{
			uint8 unitFaction = foundUnit->UnitFaction;
			switch (unitFaction)
			{
			case (EUnitFaction::PLAYER):
				playerCount++;
				break;
			case (EUnitFaction::PARTNER):
				partnerCount++;
				break;
			case (EUnitFaction::ENEMY):
				enemyCount++;
				break;
			case (EUnitFaction::NPC):
				npcCount++;
				break;
			}
		}
	}

	PlayerUnitCount = playerCount;
	PartnerUnitCount = partnerCount;
	EnemyUnitCount = enemyCount;
	NpcUnitCount = npcCount;

}

void ACombatGameMode::ActivateCombatPhase(ECombatPhase CombatPhase)
{
	// Check for events to trigger from EventData. Queue the phase if there are any events - otherwise activate the new phase directly.

	if (EventData)
	{
		ACombatEvent* foundEvent;
		bool isEventFound = EventData->EventReady(CombatPhase, TurnNumber, foundEvent);
		if (isEventFound)
		{
			// Pause phase logic until this event ends
			BeginPauseForEvent(foundEvent, CombatPhase);
			// Start event
			EventData->TriggerBeginEvent(foundEvent);
			return;
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("ActivateCombatPhase called without an EventData object linked!"));
	}

	// Trigger new phase if no special event was triggered
	OnTriggerPhase.Broadcast(CombatPhase, CurrentCombatPhase, TurnNumber);

	if (CurrentCombatPhase != CombatPhase)
	{
		// only reset units if this is a new phase
		PrepareUnitsOnPhaseShift(CombatPhase, CurrentCombatPhase);
	}

	CurrentCombatPhase = CombatPhase;

}


