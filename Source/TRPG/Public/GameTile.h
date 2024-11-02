// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Actor.h"
#include "GameTile.generated.h"

class AGameUnit;
enum EUnitFaction : uint8;

// FTerrainInfo is returned from blueprint data before calculating available paths for a unit.
// Terrain data retrieved in this class is unit-specific. For example, a flying unit may have a different MoveCost than a ground unit for a tile.
// New terrain types and behaviors can be defined in blueprints.
USTRUCT(BlueprintType)
struct FTerrainInfo	
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile")
	FName	TerrainName		= "Unknown";	// Name of this tile's terrain.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	uint8	DefStatBoost	= 0;			// Flat boost to Physical Defense while on this tile.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	uint8	ResStatBoost	= 0;			// Flat boost to Magic Defense while on this tile.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	uint8	AvoStatBoost	= 0;			// Percent boost to Avoidance while on this tile. 20 = 20% avoidance.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	uint8	HealFlat		= 0;			// Flat HP to heal at the start of the unit's turn. 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	uint8	HealPercent		= 0;			// Percent HP to heal at the start of the unit's turn. 100 = 100% HP. 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	uint8	MoveCost		= 1;			// Move cost for a unit to travel onto this tile. If this value is 255, the tile is treated as blocked.

};


// FTerrainHudInfo is hud-friendly information for this tile to display when hovering a tile.
USTRUCT(BlueprintType)
struct FTerrainHudInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FName	TerrainDisplayName = "Unknown";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FString Description = "";			// Description of this tile

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	uint8	DefStatBoost = 0;			// Flat boost to Physical Defense while on this tile.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	uint8	ResStatBoost = 0;			// Flat boost to Magic Defense while on this tile.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	uint8	AvoStatBoost = 0;			// Percent boost to Avoidance while on this tile. 20 = 20% avoidance.

};

UENUM(BlueprintType)
enum ECardinalDirections : uint8
{
	NONE		= 0		UMETA(DisplayName = "NONE"),
	UP_DIR		= 1		UMETA(DisplayName = "UP"),
	LEFT_DIR	= 2		UMETA(DisplayName = "LEFT"),
	RIGHT_DIR	= 3		UMETA(DisplayName = "RIGHT"),
	DOWN_DIR	= 4		UMETA(DisplayName = "DOWN")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileHovered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileUnhovered);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileSelected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileUnselected);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileSetNavigable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileUnsetNavigable);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileSetAttackable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileUnsetAttackable);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileSetInteractable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileUnsetInteractable);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTileSetRoutable, ECardinalDirections, OriginDirection, ECardinalDirections, TargetDirection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileUnsetRoutable);

// A tile for units to travel upon during combat.
UCLASS(Blueprintable)
class TRPG_API AGameTile : public AActor
{
	GENERATED_BODY()
	
public:	

	AGameTile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	// Root component (SceneComponent)
	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootSceneComponent;

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileHovered OnTileHovered;					// Fires when the tile is hovered by the TileControlPawn

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileUnhovered OnTileUnhovered;				// Fires when the tile is no longer hovered by the TileControlPawn

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileHovered OnTileSelected;				// Fires when the tile is selected by the TileControlPawn

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileUnhovered OnTileUnselected;			// Fires when the tile is no longer selected by the TileControlPawn

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileSetNavigable OnTileSetNavigable;		// Fires when a unit is selected and this tile displays as a movement option

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileUnsetNavigable OnTileUnsetNavigable;	// Fires when a unit is unselected and this tile should no longer display as a movement option

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileSetAttackable OnTileSetAttackable;		// Fires when a unit is selected and this tile displays as an attack option (outside of movement range)

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileUnsetAttackable OnTileUnsetAttackable;	// Fires when a unit is unselected and this tile should no longer display as an attack option

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileSetInteractable OnTileSetInteractable;		// Fires when a unit is selected and this tile displays as an interaction option 

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileUnsetInteractable OnTileUnsetInteractable;	// Fires when a unit is unselected and this tile should no longer display as an interaction option

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileSetRoutable OnTileSetRoutable;			// Fires when a unit is selected and a path for the unit is being generated (using arrow icons)

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileUnsetRoutable OnTileUnsetRoutable;		// Fires when a unit is unselected and the path for the unit should be cleared

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tile Initialization")
	float AdjacentTileDistance = 100.0f;		// Distance between the center of each tile. A value of 100.0 means this tile will trace for adjacent tiles around the four points: (Loc.X + 100.0, Loc.Y), (Loc.X - 100.0, Loc.Y), (Loc.X, Loc.Y - 100.0), and (Loc.X, Loc.Y + 100.0).

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tile Initialization")
	float AdjacentTileVerticalRange = 100.0f;	// Vertical range for tracing for adjacent tiles. A value of 100.0f means this tile will trace the Z axis from (loc.Z - 50.0) to (loc.Z + 50.0).

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* UnitPositionComponent;		// position where units on this tile will be placed

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECardinalDirections> StartingUnitDirection = ECardinalDirections::DOWN_DIR;

protected:

	AGameTile* NorthTile;		// -Y axis - set by InitializeLinkToNeighbors()
	AGameTile* WestTile;		// -X axis - set by InitializeLinkToNeighbors()
	AGameTile* EastTile;		//  X axis - set by InitializeLinkToNeighbors()
	AGameTile* SouthTile;		//  Y axis - set by InitializeLinkToNeighbors()

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PlaneTraceComponent;	// plane to be traced by adjacent tiles for initialization

	AGameUnit* CurrentUnit;

	uint8 TerrainTypeByte = 255;	// Terrain type as a byte. Set by blueprint to match the enum type.

	bool IsNavigable	= false;	// true when a unit is selected and this tile is considered reachable by movement
	bool IsAttackable	= false;	// true when a unit is selected and this tile is considered attackable 
	bool IsInteractable = false;	// true when a unit is selected and this tile is considered interactable

public:

	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetIsNavigable();	// Getter for IsNavigable

	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetIsAttackable();	// Getter for IsNavigable

	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetIsInteractable();	// Getter for IsNavigable

	// Called when the player hovers this tile actor.
	virtual void TriggerTileHover();

	// Called when the player hovers a different tile actor or the game state has been changed.
	virtual void TriggerTileUnhover();

	// Called when the player selects this tile actor.
	virtual void TriggerTileSelected();

	// called when the player ends the selection of this tile actor.
	virtual void TriggerTileUnselected();

	virtual void TriggerTileNavigable(bool Toggle);	// Called by tilecontrolpawn when this tile can be moved onto

	virtual void TriggerTileAttackable(bool Toggle);	// Called by tilecontrolpawn when this tile can be attacked

	virtual void TriggerTileInteractable(bool Toggle);	// Called by tilecontrolpawn when this tile can be interacted (ally-targeting weapons)

	virtual void TriggerTilePathing(bool Toggle, ECardinalDirections PastTileDirection, ECardinalDirections NextTileDirection);	// Called by PlayerPathControl when setting path tile visibility

	// Terrain data

	// Called before calculating navigation for an active unit. Uses the unit's movement types to determine how they can cross this tile.
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent)
	FTerrainInfo GetTerrainInfoForUnit(const uint8 UnitMovementTypeInBytes);

	// Gets the terrain info for the hud.
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent)
	FTerrainInfo GetTerrainInfoForHud();

	UFUNCTION(BlueprintImplementableEvent)
	uint8 GetTerrainTypeByte();

	static const uint8 GetTerrainTypeAsByte(AGameTile* Tile);

	// Sets the unit to this tile
	UFUNCTION(BlueprintCallable)
	virtual void SetUnitOnTile(AGameUnit* Unit, ECardinalDirections Direction);

	UFUNCTION(BlueprintCallable)
	virtual AGameTile* GetNorthTile();
	UFUNCTION(BlueprintCallable)
	virtual AGameTile* GetWestTile();
	UFUNCTION(BlueprintCallable)
	virtual AGameTile* GetSouthTile();
	UFUNCTION(BlueprintCallable)
	virtual AGameTile* GetEastTile();

	UFUNCTION(BlueprintCallable)
	bool GetUnitOnTile(AGameUnit*& Unit);

	static bool GetTilesAreAdjacent(AGameTile* A, AGameTile* B, bool& bIsNorthTile, bool& bIsEastTile, bool& bIsSouthTile, bool& bIsWestTile);	// returns true if two tiles are neighbors

protected:

	// Traces for neighboring tiles and assigns them to NorthTile/WestTile/EastTile/SouthTile
	virtual void InitializeLinkToNeighbors();


};
