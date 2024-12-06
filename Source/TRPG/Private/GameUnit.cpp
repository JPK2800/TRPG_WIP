// Fill out your copyright notice in the Description page of Project Settings.

#include "GameUnit.h"
#include "GameTile.h"
#include "UnitMovementData.h"

// Sets default values
AGameUnit::AGameUnit()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGameUnit::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeSetUnitOnInitialTile();

	InitializeUnitMovementData();
}

// Called every frame
void AGameUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGameUnit::ActivateUnitOnPhaseStart()
{
	OnUnitActivation.Broadcast(true);

	ResetUnitMovementAndActions();
}

void AGameUnit::DeactivateUnitOnPhaseEnd()
{
	OnUnitActivation.Broadcast(false);

	SetUnitGray(false);	// Remove grayscale when it's not the unit's turn
}

void AGameUnit::SetUnitLocAndRot(AGameTile* TargetTile, ECardinalDirections TargetDirection)
{
	if (!TargetTile)
		return;

	if (CurrentUnitTile)
	{
		CurrentUnitTile->SetUnitOnTile(nullptr, ECardinalDirections::NONE); // remove this unit from previous tile data
	}

	CurrentUnitTile = TargetTile;
	SetCurrentUnitDirection(TargetDirection);

	this->SetActorLocation(TargetTile->UnitPositionComponent->GetComponentLocation(), false, nullptr, ETeleportType::None);

	switch (CurrentDirection)
	{
		case (ECardinalDirections::UP_DIR):
			SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
			break;
		case (ECardinalDirections::RIGHT_DIR):
			SetActorRotation(FRotator(0.0f, 90.0f, 0.0f));
			break;
		case (ECardinalDirections::DOWN_DIR):
			SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
			break;
		case (ECardinalDirections::LEFT_DIR):
			SetActorRotation(FRotator(0.0f, 270.0f, 0.0f));
			break;
		default:
			break;
	}

	if (CurrentUnitTile)
	{
		CurrentUnitTile->SetUnitOnTile(this, TargetDirection); // remove this unit from previous tile data
	}

	OnUnitConfirmOnTile.Broadcast(TargetTile);
}

AGameTile* AGameUnit::GetCurrentUnitTile()
{
	return CurrentUnitTile;
}

ECardinalDirections AGameUnit::GetCurrentUnitDirection()
{

	return CurrentDirection;

}

void AGameUnit::SetCurrentUnitDirection(ECardinalDirections NewDir)
{
	CurrentDirection = NewDir;

	switch (NewDir)
	{
	case (ECardinalDirections::UP_DIR):
		SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
		break;
	case (ECardinalDirections::RIGHT_DIR):
		SetActorRotation(FRotator(0.0f, 90.0f, 0.0f));
		break;
	case (ECardinalDirections::DOWN_DIR):
		SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
		break;
	case (ECardinalDirections::LEFT_DIR):
		SetActorRotation(FRotator(0.0f, 270.0f, 0.0f));
		break;
	}
}

void AGameUnit::SetUnitRemainingSpaces(uint8 NewRemainingSpaces)
{
	RemainingMovementSpaces = NewRemainingSpaces;

	if (ReadyToSetUnitGray())
	{
		SetUnitGray(true);
	}
}

uint8 AGameUnit::GetUnitRemainingSpaces(AGameUnit*& Unit)
{
	return Unit->RemainingMovementSpaces;
}

void AGameUnit::SetUnitRemainingActions(uint8 NewRemainingActions)
{
	RemainingActions = NewRemainingActions;

	if (ReadyToSetUnitGray())
	{
		SetUnitGray(true);
	}
}

uint8 AGameUnit::GetUnitRemainingActions(AGameUnit*& Unit)
{
	return Unit->RemainingActions;
}

uint8 AGameUnit::GetUnitMovementForTile(uint8 TerrainType)
{
	if (MovementDataComponent)
	{
		return MovementDataComponent->GetMoveCostInfo(TerrainType);
	}
	return 255;
}

void AGameUnit::GetUnitsInRange(const uint8 MinRange, const uint8 MaxRange, const TArray<TEnumAsByte<EUnitFaction>> TargetFactions, AGameTile* CurrentTile, TArray<AGameTile*> SearchedTiles, TArray<AGameUnit*>& FoundUnits, const uint8 SearchDepth )
{
	if (SearchDepth > MaxRange || !CurrentTile || SearchedTiles.Contains(CurrentTile))
	{
		// out of range
		return;
	}

	if (SearchDepth >= MinRange && SearchDepth <= MaxRange)
	{
		AGameUnit* checkUnit;
		if (CurrentTile->GetUnitOnTile(checkUnit))
		{
			if (TargetFactions.Contains(checkUnit->UnitFaction))
			{
				// Unit has the correct faction, add to found unit array
				FoundUnits.AddUnique(checkUnit);
			}
		}
	}
	SearchedTiles.Add(CurrentTile);

	// Check north/south/east/west tiles
	GetUnitsInRange(MinRange, MaxRange, TargetFactions, CurrentTile->GetNorthTile(), SearchedTiles, FoundUnits, SearchDepth + 1);
	GetUnitsInRange(MinRange, MaxRange, TargetFactions, CurrentTile->GetSouthTile(), SearchedTiles, FoundUnits, SearchDepth + 1);
	GetUnitsInRange(MinRange, MaxRange, TargetFactions, CurrentTile->GetEastTile(), SearchedTiles, FoundUnits, SearchDepth + 1);
	GetUnitsInRange(MinRange, MaxRange, TargetFactions, CurrentTile->GetWestTile(), SearchedTiles, FoundUnits, SearchDepth + 1);

}

bool AGameUnit::ReadyToSetUnitGray()
{
	return RemainingMovementSpaces == 0 && RemainingActions == 0;
}

void AGameUnit::SortGameUnitsByLoc(TArray<AGameUnit*>& Units)
{
	
	Algo::Sort(Units, [](AGameUnit*& A, AGameUnit*& B)
		{
			FVector locA = A->GetActorLocation();
			FVector locB = B->GetActorLocation();
			// sort by Y first, then X
			if (locA.Y > locB.Y)
				return true;
			if (locA.Y < locB.Y)
				return false;

			// sort by X now
			if (locA.X < locB.X)
				return true;

			return false;
		});
}

void AGameUnit::InitializeSetUnitOnInitialTile()
{
	FVector actorLoc = GetActorLocation();
	FVector traceLoc1, traceLoc2;
	TArray<FHitResult> traceResults;

	FCollisionQueryParams CollisionParams;

	traceLoc1 = FVector(actorLoc.X, actorLoc.Y, actorLoc.Z - (InitialTileVerticalRange / 2));
	traceLoc2 = FVector(actorLoc.X, actorLoc.Y, actorLoc.Z + (InitialTileVerticalRange / 2));

	// Execute the north line trace
	bool bTraceHit = GetWorld()->LineTraceMultiByChannel(
		traceResults,
		traceLoc1,
		traceLoc2,
		ECC_GameTraceChannel1, // Use the custom game trace channel
		CollisionParams
	);

	if (bTraceHit)
	{
		for (auto traceResult : traceResults)
		{
			auto* hitTraceOwner = traceResult.Component->GetOwner();
			if (auto* tile = Cast<AGameTile>(hitTraceOwner))
			{
				float yaw = GetActorRotation().Yaw;

				if ((yaw < 45.0f && yaw > -45) || (yaw > 315.0f || yaw < -315.0f))
				{
					SetUnitLocAndRot(tile, ECardinalDirections::UP_DIR);
				}
				else if (yaw < 135.0f && yaw > -135.0f)
				{
					SetUnitLocAndRot(tile, ECardinalDirections::RIGHT_DIR);
				}
				else if (yaw < 225.0f && yaw > -225)
				{
					SetUnitLocAndRot(tile, ECardinalDirections::DOWN_DIR);
				}
				else {
					SetUnitLocAndRot(tile, ECardinalDirections::LEFT_DIR);
				}

				return;
			}
		}
		
	}
}

void AGameUnit::InitializeUnitMovementData()
{
	auto* component = GetComponentByClass<UUnitMovementData>();
	if (component)
	{
		MovementDataComponent = component;
	}
}

