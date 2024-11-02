// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitMovementData.h"

// Sets default values for this component's properties
UUnitMovementData::UUnitMovementData()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUnitMovementData::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UUnitMovementData::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UUnitMovementData::SetMovementMap(TMap<uint8, FTerrainInfo> NewTerrainData)
{
	MoveCostMap = NewTerrainData;
}

uint8 UUnitMovementData::GetMoveCostInfo(uint8 TileType)
{
	if (MoveCostMap.Contains(TileType))
	{
		return MoveCostMap[TileType].MoveCost;	// found the move cost
	}

	return 255;	// missing data from map
}

FTerrainInfo UUnitMovementData::GetTerrainPassingInfo(uint8 TileType)
{
	if (MoveCostMap.Contains(TileType))
	{
		return MoveCostMap[TileType];	// found the terrain data
	}

	return FTerrainInfo();	// missing data from map
}

