// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatEvent.h"
#include "GameFramework/Actor.h"
#include "EventDataActor.generated.h"

enum ECombatPhase : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventBegin, ACombatEvent*, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventEnd, ACombatEvent*, Event);


// Event data actor that stores event logic
// Game mode will check with this acter for every phase transition
UCLASS(Blueprintable)
class TRPG_API AEventDataActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEventDataActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phases")
	TEnumAsByte<ECombatPhase> FirstCombatPhase;	// Controls who acts first in the phase cycle. 

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<ACombatEvent*> EventsToTrigger;	// All combat events to trigger - should be references to real objects in the level

	ACombatEvent* CurrentActiveEvent;

	UPROPERTY(BlueprintAssignable)
	FEventBegin OnEventBegin;

	UPROPERTY(BlueprintAssignable)
	FEventEnd OnEventEnd;

public:

	UFUNCTION(BlueprintCallable)
	virtual void CustomTrigger(uint8 CustomTriggerId);	// Fires a custom trigger for combat events that require non-standard triggers

	virtual bool EventReady(TEnumAsByte<ECombatPhase> TargetPhase, uint8 TurnNumber, ACombatEvent*& EventFound);	// Returns true if there is a combat event to trigger before changing to this phase

	UFUNCTION()
	virtual void TriggerBeginEvent(ACombatEvent* Event);

	UFUNCTION()
	virtual void TriggerEndCurrentEvent();

};
