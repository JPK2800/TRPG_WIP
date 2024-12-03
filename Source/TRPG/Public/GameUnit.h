// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameUnit.generated.h"

class AGameTile;
class UUnitMovementData;
enum ECardinalDirections : uint8;


UENUM(BlueprintType)
enum EUnitFaction : uint8
{
	NO_FACTION = 0		UMETA(DisplayName = "NONE"),	// Ignore this unit
	PLAYER = 1			UMETA(DisplayName = "PLAYER"),		// Player controlled unit
	PARTNER = 2			UMETA(DisplayName = "PARTNER"),		// Partner AI controlled unit
	ENEMY = 3			UMETA(DisplayName = "ENEMY"),		// Enemy AI controlled unit
	NPC = 4				UMETA(DisplayName = "NPC"),			// Neutral NPC controlled unit

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUnitActivation, bool, Toggle);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTraveledToTile, AGameTile*, Tile);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUnitConfirmOnTile, AGameTile*, Tile);


// A combat actor that performs actions when controlled.
UCLASS(Blueprintable)
class TRPG_API AGameUnit : public APawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Fires when the unit is activated/deactivated and can/cannot take action
	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FUnitActivation OnUnitActivation;						

	// Fires when the unit reaches each tile when moving
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Tile")
	FTraveledToTile OnTraveledToTile;						

	// Fires when the unit performs an action and is locked onto a new tile
	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FUnitConfirmOnTile OnUnitConfirmOnTile;					

	// 0 = no faction | 1 = player | 2 = ally | 3 = enemy | 4 = npc
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 UnitFaction = EUnitFaction::NO_FACTION;

	float InitialTileVerticalRange = 300.0f;	// Range to trace for the initial tile this unit is standign on. Range of 100.0f means it will trace from Loc.Z - 50.0f to Loc.Z + 50.0f.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsMandatoryUnit;						// True if this unit is mandatory for deployment for combat

protected:

	AGameTile* CurrentUnitTile;				// The current unit's tile

	TEnumAsByte<ECardinalDirections> CurrentDirection;	// The current unit's direction

	uint8 RemainingMovementSpaces = 0;		// Remaining unit spaces to move this turn.

	uint8 RemainingActions = 0;				// Remaining actions to make this turn.

	UUnitMovementData* MovementDataComponent;	// Movement data component that initializes in blueprints

public:

	// Unit main events
	
	virtual void ActivateUnitOnPhaseStart();				// Called by the combat game mode. Enables control over this unit.

	virtual void DeactivateUnitOnPhaseEnd();				// Called by the combat game mode. Disables control over this unit.

	UFUNCTION(BlueprintCallable)
	virtual void SetUnitLocAndRot(AGameTile* TargetTile, ECardinalDirections TargetDirection);	// Sets the unit on a specific tile

	// Unit updating movement and actions during a turn.

	virtual AGameTile* GetCurrentUnitTile();

	virtual ECardinalDirections GetCurrentUnitDirection();

	virtual void SetCurrentUnitDirection(ECardinalDirections NewDir);

	UFUNCTION(BlueprintImplementableEvent)
	void MoveUnitToTile(AGameTile* Tile);

	UFUNCTION(BlueprintImplementableEvent)
	void StopMovementToTiles();

	UFUNCTION(BlueprintCallable)
	virtual void SetUnitRemainingSpaces(uint8 NewRemainingSpaces);		// Sets the number of remaining spaces after moving. Normally 0 after performing an action. 

	UFUNCTION(BlueprintCallable)
	static uint8 GetUnitRemainingSpaces(AGameUnit*& Unit);								// Gets the number of remaining spaces to move.

	UFUNCTION(BlueprintCallable)
	virtual void SetUnitRemainingActions(uint8 NewRemainingActions);	// Sets the number of remaining actions. Normally 1 at the start and 0 after an action is taken.

	UFUNCTION(BlueprintCallable)
	static uint8 GetUnitRemainingActions(AGameUnit*& Unit);							// Gets the number of remaining actions.

	UFUNCTION(BlueprintImplementableEvent)
	void ResetUnitMovementAndActions();							// Resets unit movement and actions at the start of the player turn. Unit data is in blueprints.

	// Unit weapon data / skill data / movement data - from blueprints

	UFUNCTION(BlueprintPure, BlueprintImplementableEvent)
	bool GetUnitEquippedWeaponRange(uint8& MinRange, uint8& MaxRange, bool& TargetsEnemies, bool& TargetsAllies);	// returns true if the unit has an equipped weapon. 

	uint8 GetUnitMovementForTile(uint8 TerrainType);			// Gets the number of tiles a unit can pass on the target tile

	// Unit surrounding data
	UFUNCTION(BlueprintCallable)
	void GetUnitsInRange(const uint8 MinRange, const uint8 MaxRange, const TArray<TEnumAsByte<EUnitFaction>> TargetFactions, AGameTile* CurrentTile, TArray<AGameTile*> SearchedTiles, TArray<AGameUnit*>& FoundUnits, const uint8 SearchDepth); // Find nearby units in range

	// Setting unit to gray when no other action can be taken

	bool ReadyToSetUnitGray();

	UFUNCTION(BlueprintImplementableEvent)
	void SetUnitGray(bool Toggle);					// Sets the unit to grayscale when true and back to normal colors when false

protected:

	virtual void InitializeSetUnitOnInitialTile();	// Traces for a tile below this unit and links with it if one is found

	virtual void InitializeUnitMovementData();		// Links to the unit movement data component
};
