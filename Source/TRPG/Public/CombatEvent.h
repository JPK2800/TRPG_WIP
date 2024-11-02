// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatEvent.generated.h"


UENUM(BlueprintType)
enum EEventTriggerType : uint8
{
	NoTrigger			= 0		UMETA(DisplayName = "NONE"),	// This action does not trigger
	BeforePreparations	= 1		UMETA(DisplayName = "BeforePreparations"),	// This action triggers when the map is loaded and before player preparations
	BeforePlayerPhase	= 2		UMETA(DisplayName = "BeforePlayerPhase"),	// This action triggers before the player phase
	BeforePartnerPhase	= 3		UMETA(DisplayName = "BeforePartnerPhase"),	// This action triggers before the ally phase
	BeforeEnemyPhase	= 4		UMETA(DisplayName = "BeforeEnemyPhase"),	// This action triggers before the enemy phase
	BeforeNpcPhase		= 5		UMETA(DisplayName = "BeforeNpcPhase"),		// This action triggers before the neutral phase
	EndOfBattle			= 6		UMETA(DisplayName = "BattleEnd"),			// This action triggers when the battle ends
	PlayerDefeated		= 7		UMETA(DisplayName = "PlayerDefeated"),		// This action triggers when the player is defeated
	CustomEvent			= 8		UMETA(DisplayName = "CustomEvent"),			// This action triggers when the user calls the EventDataActor CustomTrigger function with a matching CustomTriggerId.

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventEnded);

UCLASS(Blueprintable)
class TRPG_API ACombatEvent : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACombatEvent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Trigger")
	TEnumAsByte<EEventTriggerType> EventTriggerType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Trigger")
	uint8 TurnToTrigger = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Trigger")
	uint8 CustomTriggerId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Trigger")
	uint8 SameTriggerPriority = 128;	// Priority in case there's multiple events with the same trigger. Lower number = higher priority.

	UPROPERTY(BlueprintAssignable)
	FEventEnded OnEventEnded;

	bool bEventCompleted = false;

public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void TriggerEvent();	// Override in a child blueprint. Called from EventDataActor.

	UFUNCTION(BlueprintCallable)
	void TriggerEventCompleted();	// Call this function to end the event and resume phase-events (or move onto the next event if one is queued in EventDataActor)
};
