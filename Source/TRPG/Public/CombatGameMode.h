// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "GameTile.h"
#include "GameUnit.h"
#include "CombatEvent.h"
#include "GameFramework/GameModeBase.h"
#include "CombatGameMode.generated.h"

class AEventDataActor;
class ATileControlPawn;

// Enum for all combat phases (and phase transitions)
UENUM(BlueprintType)
enum ECombatPhase : uint8
{
	NO_PHASE					= 0					UMETA(DisplayName = "NO PHASE"),			
	BEFORE_COMBAT				= 1					UMETA(DisplayName = "BEFORE_COMBAT"),			
	AFTER_COMBAT				= 2					UMETA(DisplayName = "AFTER_COMBAT"),
	GAME_OVER					= 3					UMETA(DisplayName = "GAME_OVER"),
	TRANSITION_PLAYER_PHASE		= 4					UMETA(DisplayName = "TRANSITION_PLAYER_PHASE"),
	PLAYER_PHASE				= 5					UMETA(DisplayName = "PLAYER_PHASE"),
	TRANSITION_PARTNER_PHASE	= 6					UMETA(DisplayName = "TRANSITION_PARTNER_PHASE"),
	PARTNER_PHASE				= 7					UMETA(DisplayName = "PARTNER_PHASE"),
	TRANSITION_ENEMY_PHASE		= 8					UMETA(DisplayName = "TRANSITION_ENEMY_PHASE"),
	ENEMY_PHASE					= 9					UMETA(DisplayName = "ENEMY_PHASE"),
	TRANSITION_NPC_PHASE		= 10				UMETA(DisplayName = "TRANSITION_NPC_PHASE"),
	NPC_PHASE					= 11				UMETA(DisplayName = "NPC_PHASE")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTriggerPhase, ECombatPhase, NewPhase, ECombatPhase, PreviousPhase, uint8, TurnNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPausePhase, bool, ToggledPause);

// Combat game mode for phase-based gameplay.
UCLASS()
class TRPG_API ACombatGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	virtual void BeginPlay() override;

public:

	UPROPERTY(BlueprintAssignable, Category = "Phases")
	FTriggerPhase OnTriggerPhase;					// Fires when a new phase is set

	UPROPERTY(BlueprintAssignable, Category = "Phases")
	FPausePhase OnPausePhase;						// Fires when the phase is paused for an event

protected:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Phases")
	TEnumAsByte<ECombatPhase> CurrentCombatPhase = ECombatPhase::NO_PHASE;	// The current combat phase

	AEventDataActor* EventData;				// Event Data actor found on each combat map with the LinkToEventDataActor function.

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Phases")
	uint8 TurnNumber = 1;					// Current turn number. Increments after each phase-loop. Starts at 1. 

	bool IsPausedForEvent = false;			// True when the phase logic is paused for a dialogue event or scripted event
	ACombatEvent* CurrentEvent;				// Event keeping the phase loop paused
	ECombatPhase QueuedPhaseAfterEvent;		// Phase to transition to after the event(s) are completed

public:
	virtual void BeginFirstPhase();			// Triggers the before-combat phase once the player controller successfully binds to listen to phase change events

	UFUNCTION(BlueprintCallable)
	virtual void BeginNextCombatPhase();	// Triggers the next combat phase in the main gameplay loop. Also starts the loop when first called. 

	virtual void EndCombatPhases();			// Ends the combat phase logic when the level is over

	virtual void BeginPauseForEvent(ACombatEvent* Event, ECombatPhase PausedCombatPhase);		// Triggers a pause in phase logic for a dialogue event or scripted event

	UFUNCTION()
	virtual void EndPauseForEvent();		// Ends the phase logic pause

	UFUNCTION(BlueprintCallable)
	ATileControlPawn* GetControlPawn();		// Gets the control pawn

protected:

	virtual bool LinkToEventDataActor();	// Links to the event data actor. Every phase change requires an event check. 

	UFUNCTION(BlueprintCallable, Category="Combat")
	virtual void TriggerPreCombatLogic();	// Triggers pre-combat events, like dialogue, before calling BeginNextCombatPhase() to begin. This is called on the level start.

	virtual void PrepareUnitsOnPhaseShift(ECombatPhase NewPhase, ECombatPhase PreviousPhase);	// Updates unit states for the new phase.

	// Counts the number of units for each faction. 
	virtual void CountUnitsByAllegiance(uint8& PlayerUnitCount, uint8& PartnerUnitCount, uint8& EnemyUnitCount, uint8& NpcUnitCount); // Returns the unit counts for each unit allegiance.

	virtual void ActivateCombatPhase(ECombatPhase CombatPhase);
};
