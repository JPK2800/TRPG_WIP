// Fill out your copyright notice in the Description page of Project Settings.


#include "GameTile.h"
#include "GameUnit.h"


// Sets default values
AGameTile::AGameTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Setup root scene component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	// Set the root component of the actor
	RootComponent = RootSceneComponent;

	// Create the mesh for tracing neighbors
	PlaneTraceComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneTraceMesh"));
	PlaneTraceComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PlaneTraceComponent->SetCollisionObjectType(ECC_WorldStatic); // sets type to world static
	PlaneTraceComponent->SetCollisionResponseToAllChannels(ECR_Ignore); // ignore everything initially
	PlaneTraceComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap); // allow overlap with trace channel 1
	PlaneTraceComponent->SetupAttachment(GetRootComponent());

	UnitPositionComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitPositionComponent"));
	UnitPositionComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AGameTile::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeLinkToNeighbors();
	TerrainTypeByte = GetTerrainTypeByte();
}

// Called every frame
void AGameTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AGameTile::GetIsNavigable()
{
	return IsNavigable;
}

bool AGameTile::GetIsAttackable()
{
	return IsAttackable;
}

bool AGameTile::GetIsInteractable()
{
	return IsInteractable;
}

void AGameTile::TriggerTileHover()
{
	OnTileHovered.Broadcast();
}

void AGameTile::TriggerTileUnhover()
{
	OnTileUnhovered.Broadcast();
}

void AGameTile::TriggerTileSelected()
{
	OnTileSelected.Broadcast();
}

void AGameTile::TriggerTileUnselected()
{
	OnTileUnselected.Broadcast();
}

void AGameTile::TriggerTileNavigable(bool Toggle)
{
	IsNavigable = Toggle;

	if (Toggle)
	{
		OnTileSetNavigable.Broadcast();
	}
	else
	{
		OnTileUnsetNavigable.Broadcast();
	}
}

void AGameTile::TriggerTileAttackable(bool Toggle)
{
	IsAttackable = Toggle;

	if (Toggle)
	{
		OnTileSetAttackable.Broadcast();
	}
	else
	{
		OnTileUnsetAttackable.Broadcast();
	}
}

void AGameTile::TriggerTileInteractable(bool Toggle)
{
	IsInteractable = Toggle;

	if (Toggle)
	{
		OnTileSetInteractable.Broadcast();
	}
	else
	{
		OnTileUnsetInteractable.Broadcast();
	}
}

void AGameTile::TriggerTilePathing(bool Toggle, ECardinalDirections PastTileDirection, ECardinalDirections NextTileDirection)
{
	if (Toggle)
	{
		OnTileSetRoutable.Broadcast(PastTileDirection, NextTileDirection);
	}
	else
	{
		OnTileUnsetRoutable.Broadcast();
	}
}

void AGameTile::SetUnitOnTile(AGameUnit* Unit, ECardinalDirections Direction)
{
	if (!Unit)
	{
		CurrentUnit = nullptr;
		return;
	}

	CurrentUnit = Unit;
}

AGameTile* AGameTile::GetNorthTile()
{
	return NorthTile;
}

AGameTile* AGameTile::GetWestTile()
{
	return WestTile;
}

AGameTile* AGameTile::GetSouthTile()
{
	return SouthTile;
}

AGameTile* AGameTile::GetEastTile()
{
	return EastTile;
}

bool AGameTile::GetUnitOnTile(AGameUnit*& Unit)
{
	Unit = CurrentUnit;
	return CurrentUnit != nullptr;
}

bool AGameTile::GetTilesAreAdjacent(AGameTile* A, AGameTile* B, bool& bIsNorthTile, bool& bIsEastTile, bool& bIsSouthTile, bool& bIsWestTile)
{
	if (!A || !B)
	{
		return false;
	}

	bIsNorthTile = A->GetNorthTile() == B;
	bIsEastTile = A->GetEastTile() == B;
	bIsSouthTile = A->GetSouthTile() == B;
	bIsWestTile = A->GetWestTile() == B;

	return bIsNorthTile || bIsEastTile || bIsSouthTile || bIsWestTile;
}

void AGameTile::InitializeLinkToNeighbors()
{
	FVector actorLoc = GetActorLocation();
	FVector northTraceLoc1, northTraceLoc2, westTraceLoc1, westTraceLoc2, 
		eastTraceLoc1, eastTraceLoc2, southTraceLoc1, southTraceLoc2;

	northTraceLoc1 = FVector(actorLoc.X + AdjacentTileDistance, actorLoc.Y, actorLoc.Z - (AdjacentTileVerticalRange / 2));
	northTraceLoc2 = FVector(actorLoc.X + AdjacentTileDistance, actorLoc.Y, actorLoc.Z + (AdjacentTileVerticalRange / 2));

	westTraceLoc1 = FVector(actorLoc.X, actorLoc.Y - AdjacentTileDistance, actorLoc.Z - (AdjacentTileVerticalRange / 2));
	westTraceLoc2 = FVector(actorLoc.X, actorLoc.Y - AdjacentTileDistance, actorLoc.Z + (AdjacentTileVerticalRange / 2));

	eastTraceLoc1 = FVector(actorLoc.X, actorLoc.Y + AdjacentTileDistance, actorLoc.Z - (AdjacentTileVerticalRange / 2));
	eastTraceLoc2 = FVector(actorLoc.X, actorLoc.Y + AdjacentTileDistance, actorLoc.Z + (AdjacentTileVerticalRange / 2));

	southTraceLoc1 = FVector(actorLoc.X - AdjacentTileDistance, actorLoc.Y, actorLoc.Z - (AdjacentTileVerticalRange / 2));
	southTraceLoc2 = FVector(actorLoc.X - AdjacentTileDistance, actorLoc.Y, actorLoc.Z + (AdjacentTileVerticalRange / 2));

	FHitResult northTraceResult, westTraceResult, eastTraceResult, southTraceResult;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this); // Optional, to ignore the actor making the trace

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);  // Only trace against static objects


	// Execute the north line trace
	bool bNorthHit = GetWorld()->LineTraceSingleByObjectType(
		northTraceResult,
		northTraceLoc1,
		northTraceLoc2,
		ObjectQueryParams,
		CollisionParams
	);

	// Execute the west line trace
	bool bWestHit = GetWorld()->LineTraceSingleByObjectType(
		westTraceResult,
		westTraceLoc1,
		westTraceLoc2,
		ObjectQueryParams,
		CollisionParams
	);

	// Execute the east line trace
	bool bEastHit = GetWorld()->LineTraceSingleByObjectType(
		eastTraceResult,
		eastTraceLoc1,
		eastTraceLoc2,
		ObjectQueryParams,
		CollisionParams
	);

	// Execute the east line trace
	bool bSouthHit = GetWorld()->LineTraceSingleByObjectType(
		southTraceResult,
		southTraceLoc1,
		southTraceLoc2,
		ObjectQueryParams,
		CollisionParams
	);

	// If the hit actor is a tile, then save it as an adjacent tile
	if (bNorthHit)
	{
		auto* northHitTraceOwner = northTraceResult.GetActor();
		if (auto* northTile = Cast<AGameTile>(northHitTraceOwner))
		{
			NorthTile = northTile;
		}
	}

	if (bWestHit)
	{
		auto* westHitTraceOwner = westTraceResult.GetActor();
		if (auto* westTile = Cast<AGameTile>(westHitTraceOwner))
		{
			WestTile = westTile;
		}
	}

	if (bEastHit)
	{
		auto* eastHitTraceOwner = eastTraceResult.GetActor();
		if (auto* eastTile = Cast<AGameTile>(eastHitTraceOwner))
		{
			EastTile = eastTile;
		}
	}

	if (bSouthHit)
	{
		auto* southHitTraceOwner = southTraceResult.GetActor();
		if (auto* southTile = Cast<AGameTile>(southHitTraceOwner))
		{
			SouthTile = southTile;
		}
	}
}

const uint8 AGameTile::GetTerrainTypeAsByte(AGameTile* Tile)
{
	return Tile->TerrainTypeByte;
}



