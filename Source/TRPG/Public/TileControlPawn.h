// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GamePlayerController.h"
#include "TileDataActor.h"
#include "GameTile.h"
#include "GameFramework/Pawn.h"
#include "TileControlPawn.generated.h"


class UPlayerPathControl;

// FZoomLevelData stores zoom-level data
USTRUCT(BlueprintType)
struct FZoomLevelData
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float	CameraDistance	= 500.0f;	//  Length of the spring arm at this zoom setting

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float	CameraAngle		= 45.0f;	// Angle/pitch of the camera spring arm. 0 is completely horizontal and 90 is completely vertical.

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTileHover, AGameTile*, Tile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTileSelect, AGameTile*, Tile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTileView, AGameTile*, Tile);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUnitTileSelected, AGameTile*, SelectedTile, AGameUnit*, SelectedUnit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileMovementConfirm);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTileMovementComplete);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetUnit, AGameUnit*, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCancelTargetingUnits);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCancelUnitSelection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCancelUnitMovementAndAction);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FZoomChange, uint8, NewZoomLevel);






// Pawn actor that is possessed by the player controller and interacts with game tiles.
UCLASS(Blueprintable)
class TRPG_API ATileControlPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATileControlPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileHover OnTileHover;						// Fires when the tile is hovered - only activates when it is the player turn

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileSelect OnTileSelect;					// Fires when the tile is selected - only activates when it is the player turn

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FTileView OnTileView;						// Fires when the tile is viewed

	UPROPERTY(BlueprintAssignable, Category = "Tile")
	FUnitTileSelected OnUnitTileSelected;		// Fires when a unit tile is selected to control

	UPROPERTY(BlueprintAssignable, Category = "Unit Actions")
	FCancelUnitSelection OnCancelUnitSelection;	// Fires when a unit is unselected

	UPROPERTY(BlueprintAssignable, Category = "Unit Actions")
	FCancelUnitMovementAndAction OnCancelUnitMovementAndAction;	// Fires when a unit's movement is canceled

	UPROPERTY(BlueprintAssignable, Category = "Unit Actions")
	FTileMovementConfirm OnTileMovementConfirm;	// Fires when a unit is selected and confirm is pressed - only checks if the hovered tile is navigable

	UPROPERTY(BlueprintAssignable, Category = "Unit Actions")
	FTileMovementComplete OnTileMovementComplete;	// Fires when a unit is selected and confirm is pressed and the unit finished moving to the destination tile

	UPROPERTY(BlueprintAssignable, Category = "Unit Actions")
	FTargetUnit OnTargetTile;					// Fires when a new unit is being targeted for an action

	UPROPERTY(BlueprintAssignable, Category = "Unit Actions")
	FCancelTargetingUnits OnCancelTargetingUnits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FZoomLevelData ZoomMaxSettings;				// Settings when zoomed in

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FZoomLevelData ZoomMedSettings;				// Settings when zoomed halfway (default)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FZoomLevelData ZoomMinSettings;				// Settings when zoomed out

	UPROPERTY(BlueprintAssignable, Category = "Camera")
	FZoomChange OnZoomChange;					// Fires when zoom level changes

public:

	UFUNCTION(BlueprintCallable)
	virtual const AGameTile* GetViewTile();			// returns the current view tile

	UFUNCTION(BlueprintCallable)
	virtual const AGameTile* GetHoverTile();		// returns the current hover tile

	UFUNCTION(BlueprintCallable)
	virtual const AGameTile* GetSelectedTile();		// returns the current hover tile

	UFUNCTION(BlueprintCallable)
	virtual const AGameTile* GetUnitDestinationTile(TEnumAsByte<ECardinalDirections>& TargetDir);	// returns the tile where a unit is set to move

	UFUNCTION(BlueprintCallable)
	virtual void SetViewTile(AGameTile* Tile, bool SnapImmediately);		// sets the view tile. SnapImmediately will remove camera lag.

	UFUNCTION(BlueprintCallable)
	virtual void SetHoverTile(AGameTile* Tile);		// sets the hover tile

	UFUNCTION(BlueprintCallable)
	virtual void SetSelectedTile(AGameTile* Tile, AGameUnit* Unit);	// sets the selected tile. 

	// Tile movement functions

	UFUNCTION()
	virtual void SetUnitMovingToTile(AGameUnit* Unit, AGameTile* Tile, ECardinalDirections Direction);	// sets a unit to be moving - called by PlayerPathControl

	UFUNCTION()
	virtual void SetUnitMovedToTile(AGameUnit* Unit, AGameTile* Tile);	// Sets unit to be successfully moved - called by PlayerPathControl

	// Unit action functions

	UFUNCTION(BlueprintCallable)
	virtual void SetUnitActionComplete(AGameUnit* Unit, bool AllowMovement, uint8 RemainingMovement);	// Decrements unit remaining actions


	ECardinalDirections GetCurrentCameraRotation();	// Returns the current cam rotation

protected:

	bool IsOutOfCombat = true;	// True when the game is not in the combat state

	bool IsPlayerPhase = false;	// True when the game is in player-phase and inputs can be made from the player controller.

	bool IsPausedForEvent = false;	// True when a combat event is active 

	bool IsUnitMoving = false;		// True if a unit is being moved

	bool IsUnitChoosingAction = false;	// True if a unit is moved and an action is being selected

	bool IsUnitChoosingActionTarget = false;	// True if a unit is moved, chose an action, and is selecting a target

	// Root component (SceneComponent)
	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootSceneComponent;

	// Component that handles unit-pathing for the player
	UPROPERTY(VisibleAnywhere)
	UPlayerPathControl* PathControlcomponent;

	// The spring arm component that will hold the camera
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	// The camera component that follows the pawn
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(BlueprintReadOnly)
	AGameTile* ViewTile;		// The current tile to view. Same as the HoverTile during the player phase.

	UPROPERTY(BlueprintReadOnly)
	AGameTile* HoverTile;		// The current tile hovered. 

	UPROPERTY(BlueprintReadOnly)
	AGameTile* SelectedTile;	// The selected tile. Only relevant during the player phase.

	ECardinalDirections SelectedUnitDir;	// The selected unit's facing direction. 

	UPROPERTY(BlueprintReadOnly)
	AGameUnit* SelectedUnit;	// The selected unit. Only relevant during the player phase.

	ATileDataActor* TileData;	// An actor spawned on every combat level to store certain data - such as the first tile to hover

	ECardinalDirections CurrentViewRotation = ECardinalDirections::UP_DIR;	// The current camera rotation. Up = facing north, Right = facing east, Down = facing south, Left = facing west.

	UPROPERTY(BlueprintReadOnly)
	uint8 CurrentZoomLevel = 2;	// Current zoom level from 1 to 3. 1 is zoomed in max and 3 is zoomed out max.

	bool IsZoomingCamera = false;	// True when the camera needs to be moved to the current zoom level settings

	FZoomLevelData CurrentZoomSetting = ZoomMedSettings;	// Current zoom

	TArray<AGameTile*> NavigableTiles = TArray<AGameTile*>();		// Tiles that can be travel to when a unit is selected
	TArray<AGameTile*> AttackableTiles = TArray<AGameTile*>();		// Tiles that can be attacked when a unit is selected
	TArray<AGameTile*> InteractableTiles = TArray<AGameTile*>();	// Tiles that can be interacted with when a unit is selected

	TArray<AGameUnit*> TargetableActionUnits = TArray<AGameUnit*>();		// Units that can be targeted for a current action
	AGameUnit* CurrentTargetableActionUnit = nullptr;						// The current targetet unit for a current action
	uint8 CurrentTargetUnitIndex = 0;

protected:

	// Binding/linking to other actors in the world

	virtual bool BindToCombatGameMode();	// Makes a binding to the game mode to track when it becomes the player phase

	virtual bool BindInputsToOwnerController();	// Makes a binding to the player controller using this pawn to manage tiles

	virtual bool BindToTileData();			// Links to the Tile Data actor on the level - or reports a warning log if it doesn't exist

	// Phase events and control

	UFUNCTION()
	virtual void CombatPhaseChanged(ECombatPhase NewPhase, ECombatPhase PreviousPhase, uint8 TurnNumber);	// binding from game mode on phase change

	UFUNCTION()
	virtual void PauseForEvent(bool Toggle);	// Disables inputs during events

	virtual void TriggerPlayerPhase(uint8 TurnNumber);		// Called from binding to phase control event when it becomes the player phase.

	virtual void EndPlayerPhase();			// Called from binding to phase control event when it is no longer the player phase

	// View / Hover / Selected tile functions

	virtual AGameTile* GetStartingViewTile();	// Gets the starting tile from the Tile Data

	// Selected tile/unit movement/pathfinding control

	UFUNCTION(BlueprintImplementableEvent)
	void UnitReadyForActions(AGameUnit* Unit, AGameTile* Tile);		// Called when a unit is moved to a new tile - triggers a new menu 

	// Movement and tile selection
	UFUNCTION(BlueprintCallable)
	virtual void CancelUnitMovementAndAction();						// Called when a unit should be returned to their old position and the move should be undone

	// Action target selection
	UFUNCTION(BlueprintCallable)
	virtual void SetUnitTargetingPhase(TArray<AGameUnit*> TargetableUnits);	// Called when this unit is choosing between the parameter units for their action

	UFUNCTION(BlueprintCallable)
	virtual void SetTargetNextUnit();								// Targets next unit when choosing a target

	UFUNCTION(BlueprintCallable)
	virtual void SetTargetPrevUnit();								// Targets next unit when choosing a target

	UFUNCTION(BlueprintImplementableEvent)
	void UnitActionTargetUnitConfirmed(AGameUnit* TargetUnit);		// Confirmed target for an action - ready to begin animation sequence

	UFUNCTION()
	virtual void CancelUnitTargetingPhase();								// Called when this unit is no longer choosing between units for their action

	// Gets selected-unit surrounding tile displays and signals to the tiles to display this info. Saves these tile pointers.
	static void GetAvailableTilesForSelectedUnit(AGameTile* CurrentSelectedUnit, AGameUnit* SelectedUnit, TArray<AGameTile*>& NavigableTiles, TArray<AGameTile*>& AttackableTiles, TArray<AGameTile*>& InteractableTiles);

	static void GetAvailableTilesLoop(AGameUnit* CurrentSelectedUnit, AGameTile* CurrentTile, uint8 MaxTileDistance, uint8 CurrentTileDistance, uint8 NavigationDistance, uint8 InteractionDistance,
		uint8 WeaponActDistanceMin, uint8 WeaponActInstanceMax, bool weaponTargetsEnemies, bool weaponTargetsAllies, 
		TArray<AGameTile*>& FoundNavigableTiles, TArray<AGameTile*>& FoundAttackableTiles, TArray<AGameTile*>& FoundInteractableTiles, TArray<AGameTile*> checkedTiles);

	virtual void ClearSelectedTileData();

	// Camera control

	virtual void SetCameraZoomSetting(FZoomLevelData CameraSetting);	// Applies camera settings when zooming in/out

	virtual void InterpCameraZoomSettingStep(float DeltaTime);			// Transitions the camera zoom to the target settings

	virtual void SetCameraYaw(ECardinalDirections NewCardinalDir);		// Applies horizontal rotation for camera when meving cam left/right

	// Player inputs

	UFUNCTION()
	virtual void CursorMoveUp();			// Moves to the tile Up from the current tile

	UFUNCTION()
	virtual void CursorMoveDown();			// Moves to the tile Down from the current tile

	UFUNCTION()
	virtual void CursorMoveLeft();			// Moves to the tile Left from the current tile

	UFUNCTION()
	virtual void CursorMoveRight();			// Moves to the tile Right from the current tile

	UFUNCTION()
	virtual void RotateViewRight();			// Rotates the camera right

	UFUNCTION()
	virtual void RotateViewLeft();			// Rotates the camera left

	UFUNCTION()
	virtual void ZoomInCamera();			// Zooms in the camera one of three stages

	UFUNCTION()
	virtual void ZoomOutCamera();			// Zooms out the camera one of three stages

	UFUNCTION()
	virtual void InputSelectTile();			// Selects the currently hovered tile

	UFUNCTION()
	virtual void InputCancelTile();			// Deselects the currently selected tile

};
