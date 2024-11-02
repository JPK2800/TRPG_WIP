// Fill out your copyright notice in the Description page of Project Settings.

#include "EventDataActor.h"
#include "CombatGameMode.h"

// Sets default values
AEventDataActor::AEventDataActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FirstCombatPhase = ECombatPhase::TRANSITION_PLAYER_PHASE;
}

// Called when the game starts or when spawned
void AEventDataActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEventDataActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEventDataActor::CustomTrigger(uint8 CustomTriggerId)
{
}

bool AEventDataActor::EventReady(TEnumAsByte<ECombatPhase> TargetPhase, uint8 TurnNumber, ACombatEvent*& EventFound)
{
	uint8 bestPriorityEvent = 255;
	ACombatEvent* bestEventToFire = nullptr;

	for (ACombatEvent* combatEvent : EventsToTrigger)
	{

		if (combatEvent->bEventCompleted)
		{
			continue;
		}

		auto& triggerType = combatEvent->EventTriggerType;
		auto& triggerTurn = combatEvent->TurnToTrigger;
		auto& customTriggerId = combatEvent->CustomTriggerId;
		auto& eventPriority = combatEvent->SameTriggerPriority;

		switch (triggerType) {
		case (EEventTriggerType::BeforePreparations):
			// trigger "before preparation" events immediately
			if (eventPriority < bestPriorityEvent)
			{
				bestEventToFire = combatEvent;
				bestPriorityEvent = eventPriority;
			}
			break;
		case (EEventTriggerType::BeforePlayerPhase):
			// trigger before a transition to the player phase
			if (TargetPhase == ECombatPhase::TRANSITION_PLAYER_PHASE) {
				if (eventPriority < bestPriorityEvent && TurnNumber >= triggerTurn)
				{
					bestEventToFire = combatEvent;
					bestPriorityEvent = eventPriority;
				}
			}
			break;
		case (EEventTriggerType::BeforePartnerPhase):
			// trigger before a transition to the ally phase
			if (TargetPhase == ECombatPhase::TRANSITION_PARTNER_PHASE) {
				if (eventPriority < bestPriorityEvent && TurnNumber >= triggerTurn)
				{
					bestEventToFire = combatEvent;
					bestPriorityEvent = eventPriority;
				}
			}
			break;
		case (EEventTriggerType::BeforeEnemyPhase):
			// trigger before a transition to the enemy phase
			if (TargetPhase == ECombatPhase::TRANSITION_ENEMY_PHASE) {
				if (eventPriority < bestPriorityEvent && TurnNumber >= triggerTurn)
				{
					bestEventToFire = combatEvent;
					bestPriorityEvent = eventPriority;
				}
			}
			break;
		case (EEventTriggerType::BeforeNpcPhase):
			// trigger before a transition to the npc phase
			if (TargetPhase == ECombatPhase::TRANSITION_NPC_PHASE) {
				if (eventPriority < bestPriorityEvent && TurnNumber >= triggerTurn)
				{
					bestEventToFire = combatEvent;
					bestPriorityEvent = eventPriority;
				}
			}
			break;
		case (EEventTriggerType::EndOfBattle):
			// trigger before a transition to theend of battle
			if (TargetPhase == ECombatPhase::AFTER_COMBAT) {
				if (eventPriority < bestPriorityEvent)
				{
					bestEventToFire = combatEvent;
					bestPriorityEvent = eventPriority;
				}
			}
			break;
		case (EEventTriggerType::PlayerDefeated):
			// trigger before a transition to the game over phase
			if (TargetPhase == ECombatPhase::GAME_OVER) {
				if (eventPriority < bestPriorityEvent)
				{
					bestEventToFire = combatEvent;
					bestPriorityEvent = eventPriority;
				}
			}
			break;
		case (EEventTriggerType::CustomEvent):
			// Custom events are triggered manually with the CustomTrigger() function
			break;
		}
	}

	if (bestEventToFire)
	{
		EventFound = bestEventToFire;
		return true;
	}

	return false;
}

void AEventDataActor::TriggerBeginEvent(ACombatEvent* Event)
{
	if (Event)
	{
		CurrentActiveEvent = Event;
		OnEventBegin.Broadcast(Event);
		CurrentActiveEvent->OnEventEnded.AddDynamic(this, &AEventDataActor::TriggerEndCurrentEvent);

		CurrentActiveEvent->TriggerEvent();
	}
}

void AEventDataActor::TriggerEndCurrentEvent()
{
	OnEventEnd.Broadcast(CurrentActiveEvent);
	if (CurrentActiveEvent)
	{
		CurrentActiveEvent->OnEventEnded.RemoveDynamic(this, &AEventDataActor::TriggerEndCurrentEvent);
		CurrentActiveEvent = nullptr;
	}
}

