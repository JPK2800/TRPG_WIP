// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameTile.h"
#include "Components/ActorComponent.h"
#include "UnitMovementData.generated.h"


UCLASS(Blueprintable)
class TRPG_API UUnitMovementData : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUnitMovementData();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	TMap<uint8, FTerrainInfo> MoveCostMap = TMap<uint8, FTerrainInfo>();	// Map where keys are tile types (enum in blueprint) and values are FTerrainInfo structs (precalculated by unit on start)

public:

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void InitializeMovementMap();			// blueprint function to initialize the movement map

	UFUNCTION(BlueprintCallable)
	void SetMovementMap(TMap<uint8, FTerrainInfo> NewTerrainData);

	uint8 GetMoveCostInfo(uint8 TileType);	// Gets the move cost for a tile type for this specific unit

	FTerrainInfo GetTerrainPassingInfo(uint8 TileType); // Gets all terrain passing info for a tile type for this specific unit
		
};
