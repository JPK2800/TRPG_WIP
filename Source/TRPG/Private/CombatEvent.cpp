// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatEvent.h"

// Sets default values
ACombatEvent::ACombatEvent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACombatEvent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACombatEvent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACombatEvent::TriggerEventCompleted()
{
	bEventCompleted = true;
	OnEventEnded.Broadcast();
}

