// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPathControl.h"

// Sets default values for this component's properties
UPlayerPathControl::UPlayerPathControl()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerPathControl::BeginPlay()
{
	Super::BeginPlay();

	BindToTileControlPawn();
	
}


// Called every frame
void UPlayerPathControl::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerPathControl::BindToTileControlPawn()
{
	if (!GetOwner())
		return;

	auto* tileControlPawn = Cast<ATileControlPawn>(GetOwner());
	if (tileControlPawn)
	{
		TileControlPawn = tileControlPawn;
		TileControlPawn->OnTileHover.AddDynamic(this, &UPlayerPathControl::TileHovered);
		TileControlPawn->OnUnitTileSelected.AddDynamic(this, &UPlayerPathControl::TileUnitSelected);
		TileControlPawn->OnTileMovementConfirm.AddDynamic(this, &UPlayerPathControl::BeginTravelingOnCurrentPath);
		TileControlPawn->OnCancelUnitMovementAndAction.AddDynamic(this, &UPlayerPathControl::CancelTravelingOnCurrentPath);

	}
}

void UPlayerPathControl::TileUnitSelected(AGameTile* Tile, AGameUnit* Unit)
{
	if (Tile && Unit)
	{
		SetIsPathingUnit(true, Tile, Unit->GetCurrentUnitDirection(), AGameUnit::GetUnitRemainingSpaces(Unit));
		TileHovered(Tile);
		SelectedUnit = Unit;
	}
	else
	{
		TileUnitUnselected();
		SelectedUnit = nullptr;
	}
}

void UPlayerPathControl::TileUnitUnselected()
{
	MaxMovementDist = 0;
	for (auto* tile : CurrentPath)
	{
		tile->TriggerTilePathing(false, ECardinalDirections::NONE, ECardinalDirections::NONE);
	}
	CurrentPath.Empty();
	StartingTile = nullptr;
	SelectedUnit = nullptr;
	IsPathing = false;
}

void UPlayerPathControl::SetIsPathingUnit(const bool IsPathingUnit, AGameTile* StartingUnitTile, const ECardinalDirections StartingUnitPathDirection, const uint8 MaxMovement)
{
	IsPathing = IsPathingUnit;
	StartingTile = StartingUnitTile;
	MaxMovementDist = MaxMovement;

	CurrentPath.Empty();
	if (IsPathingUnit)
		CurrentPath.Add(StartingTile);
}

void UPlayerPathControl::TileHovered(AGameTile* Tile)
{
	if (!IsPathing || !Tile->GetIsNavigable()) // Not ready for navigation or hovered tile is invalid for pathing
	{
		return;
	}

	auto* lastPathTile = CurrentPath.Top();
	if (!lastPathTile)
	{
		return; // the path should not be empty when pathing
	}

	const TArray<AGameTile*> prevPath = CurrentPath;
	TArray<AGameTile*> newPath;
	bool successFindingPath;

	if (CurrentPath.Contains(Tile))
	{
		// the current path already has this tile - trim the path so this is the end of the route
		bool tileRemovalComplete = false;
		newPath = prevPath;
		while (newPath.Top() != Tile)
		{
			newPath.Pop();
		}
		CurrentPath = newPath;
		UpdateTileDisplaysForNewPath(CurrentPath, prevPath);
		return;
	}

	if (CurrentPath.Num() <= MaxMovementDist)
	{
		// allowed to immediately allocate new tiles still
		
		bool isNorthTile, isSouthTile, isEastTile, isWestTile;
		if (AGameTile::GetTilesAreAdjacent(lastPathTile, Tile, isNorthTile, isEastTile, isSouthTile, isWestTile))
		{
			// The new tile is adjacent to the last tile - easy append
			CurrentPath.Add(Tile);
			UpdateTileDisplaysForNewPath(CurrentPath, prevPath);
			return;
		}
		
		// the new tile is not adjacent to the last tile
		// check if a new path can be auto-generated starting from the last existing tile to get to reach this new tile
		successFindingPath = CalculateNewPathToReachTile(CurrentPath, Tile, newPath);
		if (successFindingPath)
		{
			CurrentPath = newPath;
			UpdateTileDisplaysForNewPath(CurrentPath, prevPath);
			return;
		}
	}

	// if the path cannot just add tiles to reach the destination, backtrack one tile at a tile and keep checking if the new path can be auto-generated
	// This should eventually work. Worst case involves calculating from the starting tile.
	TArray<AGameTile*> trimmedPath = prevPath;
	while (trimmedPath.Num() > 0)
	{
		trimmedPath.Pop();
		successFindingPath = CalculateNewPathToReachTile(trimmedPath, Tile, newPath);
		if (successFindingPath)
		{
			CurrentPath = newPath;
			UpdateTileDisplaysForNewPath(CurrentPath, prevPath);
			return;
		}
	}

	// couldn't find a new path - do nothing and leave the current path as-is
	return;
}

void UPlayerPathControl::UpdateTileDisplaysForNewPath(const TArray<AGameTile*> NewPath, const TArray<AGameTile*> PrevPath)
{
	for (auto* tile : PrevPath)
	{
		if (!NewPath.Contains(tile))
		{
			tile->TriggerTilePathing(false, ECardinalDirections::NONE, ECardinalDirections::NONE);
		}
	}
	for (int i = 0; i < NewPath.Num(); i++)
	{
		// NewPath is ignored and we need to report origin and target directions for each tile
		ECardinalDirections origin, target;
		AGameTile* currentTile = NewPath[i];

		AGameTile* northTile, *southTile, *eastTile, *westTile;
		northTile = currentTile->GetNorthTile();
		southTile = currentTile->GetSouthTile();
		eastTile = currentTile->GetEastTile();
		westTile = currentTile->GetWestTile();

		if (i == 0)
		{
			// this is the first tile
			origin = ECardinalDirections::NONE;
		}
		else
		{
			// there is a tile before this one
			AGameTile* prevTile = NewPath[i - 1];
			if (southTile == prevTile)
			{
				// the path started south-to-north
				origin = ECardinalDirections::DOWN_DIR;
			}
			else if (eastTile == prevTile)
			{
				// the path started east-to-west
				origin = ECardinalDirections::RIGHT_DIR;
			}
			else if (westTile == prevTile)
			{
				// the path started west-to-east
				origin = ECardinalDirections::LEFT_DIR;
			}
			else if (northTile == prevTile)
			{
				// the path started north-to-south
				origin = ECardinalDirections::UP_DIR;
			}
			else
			{
				// no idea where the path came from, tile-linking error maybe
				origin = ECardinalDirections::NONE;
			}
		}
		if (NewPath.Num() - 1 == i) 
		{
			// this is the last tile
			target = ECardinalDirections::NONE;
		}
		else
		{
			// there is a tile after this one
			auto* nextTile = NewPath[i + 1];
			if (southTile == nextTile)
			{
				// the path continues to the south
				target = ECardinalDirections::DOWN_DIR;
			}
			else if (eastTile == nextTile)
			{
				// the path continues to the east
				target = ECardinalDirections::RIGHT_DIR;

			}
			else if (westTile == nextTile)
			{
				// the path continues to the west
				target = ECardinalDirections::LEFT_DIR;

			}
			else if (northTile == nextTile)
			{
				// the path continues to the north
				target = ECardinalDirections::UP_DIR;
			}
		}

		currentTile->TriggerTilePathing(true, origin, target);	// signal to display arrows to the tile
	}
}

bool UPlayerPathControl::CalculateNewPathToReachTile(const TArray<AGameTile*> Path, const AGameTile* TargetTile, TArray<AGameTile*>& NewPath)
{
	if (Path.Num() == 0)
		return false; // Path should always at least have a starting tile

	AGameTile* lastTile = Path.Top();

	TArray<AGameTile*> newPath;
	uint8 newPathLen;
	if (FindPathToReachTileLoop(lastTile, TargetTile, 0, MaxMovementDist - Path.Num() + 1, Path, newPath, newPathLen))
	{
		NewPath = newPath;
		return true;
	}

	return false;
}

bool UPlayerPathControl::FindPathToReachTileLoop(AGameTile* CurrentTile, const AGameTile* TargetTile, 
	const uint8 PathLength, const uint8 MaxPathLength, const TArray<AGameTile*> SearchPath, TArray<AGameTile*>& FoundPath, uint8& FoundPathLength)
{
	if (!CurrentTile || PathLength > MaxPathLength || (SearchPath.Contains(CurrentTile) && PathLength != 0) || !CurrentTile->GetIsNavigable())
	{
		return false;
	}

	if (CurrentTile == TargetTile)
	{
		// path is found
		FoundPath = SearchPath;
		FoundPath.Add(CurrentTile);
		FoundPathLength = PathLength;
		return true;
	}

	TArray<AGameTile*> newCurrentPath = SearchPath;

	if (!SearchPath.Contains(CurrentTile)) // only add this tile to the path if this isn't the first loop (avoids a duplicate in FoundPath)
		newCurrentPath.Add(CurrentTile);

	// continuing to find the best path
	auto* northTile = CurrentTile->GetNorthTile();
	auto* southTile = CurrentTile->GetSouthTile();
	auto* eastTile = CurrentTile->GetEastTile();
	auto* westTile = CurrentTile->GetWestTile();

	TArray<AGameTile*> northPath, southPath, eastPath, westPath;
	uint8 northPathLen = 255, southPathLen = 255, eastPathLen = 255, westPathLen = 255;
	bool isPathNorth, isPathSouth, isPathEast, isPathWest;

	isPathNorth = FindPathToReachTileLoop(northTile, TargetTile, PathLength + 1, MaxPathLength, newCurrentPath, northPath, northPathLen);
	isPathSouth = FindPathToReachTileLoop(southTile, TargetTile, PathLength + 1, MaxPathLength, newCurrentPath, southPath, southPathLen);
	isPathEast = FindPathToReachTileLoop(eastTile, TargetTile, PathLength + 1, MaxPathLength, newCurrentPath, eastPath, eastPathLen);
	isPathWest = FindPathToReachTileLoop(westTile, TargetTile, PathLength + 1, MaxPathLength, newCurrentPath, westPath, westPathLen);

	// the path priority depends on the current camera rotation. prioritize paths where units move horizontally before vertically since it is easier to visualize from the top-down diagonal view
	ECardinalDirections currentCamSetting = TileControlPawn->GetCurrentCameraRotation();

	if (currentCamSetting == ECardinalDirections::UP_DIR || currentCamSetting == ECardinalDirections::DOWN_DIR)
	{
		// priority is west/east/south/north
		if (isPathWest && westPathLen <= FMath::Min3(eastPathLen, southPathLen, northPathLen))
		{
			FoundPath = westPath;
			FoundPathLength = westPathLen;
			return true;
		}
		else if (isPathEast && eastPathLen <= FMath::Min(southPathLen, northPathLen))
		{
			FoundPath = eastPath;
			FoundPathLength = eastPathLen;
			return true;
		}
		else if (isPathSouth && southPathLen <= northPathLen)
		{
			FoundPath = southPath;
			FoundPathLength = southPathLen;
			return true;
		}
		else if (isPathNorth)
		{
			FoundPath = northPath;
			FoundPathLength = northPathLen;
			return true;
		}
	}
	else
	{
		// priority is north/south/east/west

		if (isPathNorth && northPathLen <= FMath::Min3(southPathLen, eastPathLen, westPathLen))
		{
			FoundPath = northPath;
			FoundPathLength = northPathLen;
			return true;
		}
		else if (isPathSouth && southPathLen <= FMath::Min(eastPathLen, westPathLen))
		{
			FoundPath = southPath;
			FoundPathLength = southPathLen;
			return true;
		}
		else if (isPathEast && eastPathLen <= westPathLen)
		{
			FoundPath = eastPath;
			FoundPathLength = eastPathLen;
			return true;
		}
		else if (isPathWest)
		{
			FoundPath = westPath;
			FoundPathLength = westPathLen;
			return true;
		}
	}

	// no path found
	FoundPathLength = 255;
	return false;
}

void UPlayerPathControl::UnitMovedToTile(AGameTile* Tile)
{
	ContinueTravelingOnCurrentPath();
}

void UPlayerPathControl::BeginTravelingOnCurrentPath()
{
	if (!SelectedUnit)
	{
		return; // cannot move a null unit
	}

	TravelPathTilesTraveled = 0;

	if (CurrentPath.Num() <= 0)
	{
		return; // need at least one tile to traverse
	}

	AGameTile* firstTile = CurrentPath[0];
	IsUnitMoving = true;

	SelectedUnit->OnTraveledToTile.AddDynamic(this, &UPlayerPathControl::UnitMovedToTile);	// begin listening to unit travel progress
	bool isNorth, isSouth, isEast, isWest;
	bool isAdj = AGameTile::GetTilesAreAdjacent(SelectedUnit->GetCurrentUnitTile(), firstTile, isNorth, isEast, isSouth, isWest);
	ECardinalDirections newDir = isNorth ? ECardinalDirections::UP_DIR : (isEast ? ECardinalDirections::RIGHT_DIR : (isSouth ? ECardinalDirections::DOWN_DIR : ECardinalDirections::LEFT_DIR));
	TileControlPawn->SetUnitMovingToTile(SelectedUnit, firstTile, newDir);
}

void UPlayerPathControl::ContinueTravelingOnCurrentPath()
{
	TravelPathTilesTraveled++;

	if (CurrentPath.IsValidIndex(TravelPathTilesTraveled))
	{
		// go to next tile
		AGameTile* nextTile = CurrentPath[TravelPathTilesTraveled];
		bool isNorth, isSouth, isEast, isWest;
		bool isAdj = AGameTile::GetTilesAreAdjacent(CurrentPath[TravelPathTilesTraveled-1], nextTile, isNorth, isEast, isSouth, isWest);
		ECardinalDirections newDir = isNorth ? ECardinalDirections::UP_DIR : (isEast ? ECardinalDirections::RIGHT_DIR : (isSouth ? ECardinalDirections::DOWN_DIR : ECardinalDirections::LEFT_DIR));
		TileControlPawn->SetUnitMovingToTile(SelectedUnit, nextTile, newDir);
	}
	else
	{
		EndTravelingOnCurrentPath();
	}
}

void UPlayerPathControl::EndTravelingOnCurrentPath()
{
	// completed travels
	IsUnitMoving = false;
	TileControlPawn->SetUnitMovedToTile(SelectedUnit, CurrentPath.Top());
	SelectedUnit->OnTraveledToTile.RemoveDynamic(this, &UPlayerPathControl::UnitMovedToTile); // stop listening to unit travel progress since it's done
}

void UPlayerPathControl::CancelTravelingOnCurrentPath()
{
	if (IsUnitMoving)
	{
		IsUnitMoving = false;
		SelectedUnit->OnTraveledToTile.RemoveDynamic(this, &UPlayerPathControl::UnitMovedToTile); // stop listening to unit travel progress since it's done
	}

	
}

