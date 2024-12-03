// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TileControlPawn.h"
#include "Components/ActorComponent.h"
#include "PlayerPathControl.generated.h"

// Component for TileControlPawn. Manages the tiles planned for navigation when moving units and triggers arrows to display.
UCLASS()
class TRPG_API UPlayerPathControl : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerPathControl();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual AGameTile* GetPathLastTile(ECardinalDirections& Direction);

protected:

	ATileControlPawn* TileControlPawn;	// Owner actor 

	bool IsPathing = false;				// True when a unit is selected and the player is making a path of it

	AGameTile* StartingTile = nullptr;		// The starting tile for the path.

	uint8 MaxMovementDist = 0;				// Max movement distance for the currently selected unit

	TArray<AGameTile*> CurrentPath = TArray<AGameTile*>();	// The current path of tiles being displayed.

	ECardinalDirections LastDirection;	// The final direction for the unit when moving.

	uint8 TravelPathTilesTraveled = 0;

	// Moving-unit variables

	bool IsUnitMoving = false;	// True if the unit is moving to the CurrentPath destination

	bool IsUnitMoved = false;	// True if the unit moved to the CurrentPath destination but the turn is not yet over

	AGameUnit* SelectedUnit = nullptr;

	// Binding 

	virtual void BindToTileControlPawn();	// Binds to TileControlPawn events (owner)

	// Selecting and planning unit movements

	UFUNCTION()
	virtual void TileUnitSelected(AGameTile* Tile, AGameUnit* Unit);		// Fires when a unit tile is selected or deselected

	UFUNCTION()
	virtual void TileUnitUnselected();

	UFUNCTION()
	virtual void TileHovered(AGameTile* Tile);	// Fires every time a new tile is hovered - update the path if a unit is selected

	virtual void SetIsPathingUnit(const bool IsPathingUnit, AGameTile* StartingUnitTile, const ECardinalDirections StartingUnitPathDirection, const uint8 MaxMovement);	// Sets whether this component should be tracking a path.

	virtual void UpdateTileDisplaysForNewPath(const TArray<AGameTile*> NewPath, const TArray<AGameTile*> PrevPath); // Updates the arrow-displays for the new path

	virtual bool CalculateNewPathToReachTile(const TArray<AGameTile*> Path, const AGameTile* TargetTile, TArray<AGameTile*>& NewPath); // Creates the cheapest path from the existing path to the target tile. Returns false if not possible with the current max movement.
	
	bool FindPathToReachTileLoop(AGameTile* CurrentTile, const AGameTile* TargetTile, const uint8 PathLength, const uint8 MaxPathLength, const TArray<AGameTile*> SearchPath, TArray<AGameTile*>& FoundPath, uint8& FoundPathLength);

	// Selecting a destination tile and moving the the position - while still retaining the old position in case the action is canceled

	UFUNCTION()
	virtual void UnitMovedToTile(AGameTile* Tile);	// triggered when bound to a moving game unit to trigger other unit-traveling functions

	UFUNCTION()
	virtual void BeginTravelingOnCurrentPath();		// First call to begin moving a unit across the CurrentPath

	virtual void ContinueTravelingOnCurrentPath();	// Continued calls to keep units traveling the CurrentPath after listening to events from the unit

	virtual void EndTravelingOnCurrentPath();		// Called when the last tile has been reached by the moving unit in CurrentPath

	UFUNCTION()
	virtual void CancelTravelingOnCurrentPath();	// Called when the cancel input is pressed during or after a unit has been moved (before the unit's turn is locked in)
};
