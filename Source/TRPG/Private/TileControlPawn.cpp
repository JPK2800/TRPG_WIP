// Fill out your copyright notice in the Description page of Project Settings.

#include "TileControlPawn.h"
#include "PlayerPathControl.h"

// Sets default values
ATileControlPawn::ATileControlPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup root scene component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	// Set the root component of the actor
	RootComponent = RootSceneComponent;

	// Create and initialize the Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);  // Attach to the root of the pawn
	SpringArm->TargetArmLength = 500.0f;        // Distance from the pawn (adjustable)
	SpringArm->bEnableCameraLag = true;         // Enable smooth movement for camera
	SpringArm->CameraLagSpeed = 1.0f;			// How quickly it smooths out movement
	SpringArm->bInheritPitch = false;           // Prevent pitch rotation
	SpringArm->bInheritYaw = false;             // Prevent yaw rotation
	SpringArm->bInheritRoll = false;            // Prevent roll rotation
	SetCameraZoomSetting(ZoomMedSettings);

	// Create and initialize the Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);  // Attach the camera to the spring arm

	CurrentViewRotation = ECardinalDirections::UP_DIR;

	// Pathing component
	PathControlcomponent = CreateDefaultSubobject<UPlayerPathControl>(TEXT("PathControl"));
}

// Called when the game starts or when spawned
void ATileControlPawn::BeginPlay()
{
	Super::BeginPlay();
	
	BindToCombatGameMode();	// link to the game mode to listen to phase-changes

	BindInputsToOwnerController();	// link to player controller to listen to inputs

	if (BindToTileData())	// link to the tile data object on each level and find the starting view tile
	{
		AGameTile* startingViewTile = GetStartingViewTile();
		SetViewTile(startingViewTile, true);
	}
}

// Called every frame
void ATileControlPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ATileControlPawn::BindToCombatGameMode()
{
	AGameModeBase* gameMode = GetWorld()->GetAuthGameMode();
	if (!gameMode)
	{
		return false;
	}

	ACombatGameMode* combatGameMode = Cast<ACombatGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!combatGameMode)
	{
		return false;
	}

	combatGameMode->OnTriggerPhase.AddDynamic(this, &ATileControlPawn::CombatPhaseChanged);
	combatGameMode->OnPausePhase.AddDynamic(this, &ATileControlPawn::PauseForEvent);
	return true;
}

void ATileControlPawn::CombatPhaseChanged(ECombatPhase NewPhase, ECombatPhase PreviousPhase, uint8 TurnNumber)
{
	if (NewPhase == ECombatPhase::PLAYER_PHASE)
	{
		TriggerPlayerPhase(TurnNumber);
	}
	else if (PreviousPhase == ECombatPhase::PLAYER_PHASE)
	{
		EndPlayerPhase();
	}
}

void ATileControlPawn::PauseForEvent(bool Toggle)
{
	IsPausedForEvent = Toggle;
}

bool ATileControlPawn::BindInputsToOwnerController()
{
	AGamePlayerController* gameController;
	AActor* owner = GetOwner();
	if (owner)
	{
		gameController = Cast<AGamePlayerController>(owner);
		if (gameController)
		{
			// Tile navigation
			gameController->OnUpPress.AddDynamic(this, &ATileControlPawn::CursorMoveUp);
			gameController->OnDownPress.AddDynamic(this, &ATileControlPawn::CursorMoveDown);
			gameController->OnLeftPress.AddDynamic(this, &ATileControlPawn::CursorMoveLeft);
			gameController->OnRightPress.AddDynamic(this, &ATileControlPawn::CursorMoveRight);

			// Camera
			gameController->OnCamUpPress.AddDynamic(this, &ATileControlPawn::ZoomInCamera);
			gameController->OnCamDownPress.AddDynamic(this, &ATileControlPawn::ZoomOutCamera);
			gameController->OnCamLeftPress.AddDynamic(this, &ATileControlPawn::RotateViewLeft);
			gameController->OnCamRightPress.AddDynamic(this, &ATileControlPawn::RotateViewRight);

			// Action input
			gameController->OnConfirmPress.AddDynamic(this, &ATileControlPawn::InputSelectTile);
			gameController->OnCancelPress.AddDynamic(this, &ATileControlPawn::InputCancelTile);
		}
	}

	return false;
}

void ATileControlPawn::TriggerPlayerPhase(uint8 TurnNumber)
{
	if (TurnNumber == 1)
	{
		AGameTile* startingTile = GetStartingViewTile();
		SetViewTile(startingTile, false);
		SetHoverTile(startingTile);
	}
	else
	{
		SetHoverTile(ViewTile);
	}

	IsPlayerPhase = true;
}

void ATileControlPawn::EndPlayerPhase()
{
	SetHoverTile(nullptr);
	SetSelectedTile(nullptr, nullptr);

	IsPlayerPhase = false;
}

bool ATileControlPawn::BindToTileData()
{
	AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ATileDataActor::StaticClass());
	if (auto* tileData = Cast<ATileDataActor>(foundActor))
	{
		TileData = tileData;
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("MISSING TileDataActor on this level!"));
	return false;
}

AGameTile* ATileControlPawn::GetStartingViewTile()
{
	if (TileData)
	{
		return TileData->StartingViewTile;
	}

	return nullptr;
}

void ATileControlPawn::CancelUnitMovementAndAction()
{
	if (!SelectedUnit)
	{
		return;
	}
	IsUnitMoving = false;
	IsUnitChoosingAction = false;
	SelectedUnit->StopMovementToTiles();								// Stop all automatic movements
	SelectedUnit->SetUnitLocAndRot(SelectedTile, SelectedUnitDir);		// Reset unit position and rotation	

	OnCancelUnitMovementAndAction.Broadcast();

}

void ATileControlPawn::GetAvailableTilesForSelectedUnit(AGameTile* SelectedTile, AGameUnit* CurrentSelectedUnit, TArray<AGameTile*>& FoundNavigableTiles, TArray<AGameTile*>& FoundAttackableTiles, TArray<AGameTile*>& FoundInteractableTiles)
{
	if (!CurrentSelectedUnit || !SelectedTile)
	{
		return;
	}

	uint8 remainingMoves = AGameUnit::GetUnitRemainingSpaces(CurrentSelectedUnit) + 1;
	uint8 remainingActions = AGameUnit::GetUnitRemainingActions(CurrentSelectedUnit);
	uint8 minAtkRange = 0, maxAtkRange = 0;
	bool weaponTargetsAllies = false, weaponTargetsEnemies = false;
	bool hasWeapon;
	if (remainingActions > 0)
		hasWeapon = CurrentSelectedUnit->GetUnitEquippedWeaponRange(minAtkRange, maxAtkRange, weaponTargetsEnemies, weaponTargetsAllies);	// get weapon data only if an action is available
	else
		hasWeapon = false;	// skip getting weapon data - no actions left

	TArray<AGameTile*> navigableTiles = TArray<AGameTile*>();
	TArray<AGameTile*> attackableTiles = TArray<AGameTile*>();
	TArray<AGameTile*> interactableTiles = TArray<AGameTile*>();
	TArray<AGameTile*> checkedTiles = TArray<AGameTile*>();


	uint8 maxDistanceToConsider = FMath::Max(remainingMoves + maxAtkRange, remainingMoves + 1);	// Only consider tiles within the movement/attack range or movement/interact range

	// Calculate navigable tiles
	GetAvailableTilesLoop(CurrentSelectedUnit, SelectedTile, maxDistanceToConsider, 0, remainingMoves, 1, minAtkRange, maxAtkRange, weaponTargetsEnemies, weaponTargetsAllies, navigableTiles, attackableTiles, interactableTiles, checkedTiles);

	for (AGameTile* navigableTile : navigableTiles)
	{
		navigableTile->TriggerTileNavigable(true);
	}
	for (AGameTile* attackableTile : attackableTiles)
	{
		attackableTile->TriggerTileAttackable(true);
	}
	for (AGameTile* interactableTile : interactableTiles)
	{
		interactableTile->TriggerTileInteractable(true);
	}

	FoundNavigableTiles = navigableTiles;
	FoundAttackableTiles = attackableTiles;
	FoundInteractableTiles = interactableTiles;
}

void ATileControlPawn::GetAvailableTilesLoop(AGameUnit* CurrentSelectedUnit, AGameTile* CurrentTile, uint8 MaxTileDistance, uint8 CurrentTileDistance, uint8 NavigationDistance, uint8 InteractionDistance, uint8 WeaponActDistanceMin, uint8 WeaponActInstanceMax, bool weaponTargetsEnemies, bool weaponTargetsAllies,
	TArray<AGameTile*>& FoundNavigableTiles, TArray<AGameTile*>& FoundAttackableTiles, TArray<AGameTile*>& FoundInteractableTiles, TArray<AGameTile*> checkedTiles)
{
	if (!CurrentTile)
		return;

	if (CurrentTileDistance > MaxTileDistance)
	{
		return;
	}

	bool tileHasAlly = false, tileHasEnemy = false;	// track if the tile has an enemy/ally for deciding navigation paths later.

	AGameUnit* tileUnit;
	if (CurrentTile->GetUnitOnTile(tileUnit))	// Checked tile has a unit on it
	{
		// Check the current tile for an enemy/ally that the current-equipped weapon can act with
		
		if (tileUnit->UnitFaction == EUnitFaction::ENEMY)	// enemy unit
		{
			tileHasEnemy = true;

			// target is an enemy and is attackable if the weapon equipped can target enemies.
			if (CurrentTileDistance >= WeaponActDistanceMin && CurrentTileDistance <= WeaponActInstanceMax && weaponTargetsEnemies) // in weapon range
			{
				FoundAttackableTiles.AddUnique(CurrentTile);
			}
		}
		if ((tileUnit->UnitFaction == EUnitFaction::PLAYER || tileUnit->UnitFaction == EUnitFaction::PARTNER))	// ally unit
		{
			tileHasAlly = true;

			// target is an ally and is actionable if the weapon equipped can target allies
			if (CurrentTileDistance >= WeaponActDistanceMin && CurrentTileDistance <= WeaponActInstanceMax && weaponTargetsAllies) // in weapon range
			{
				FoundInteractableTiles.AddUnique(CurrentTile);
			}
		}
	}

	checkedTiles.AddUnique(CurrentTile); // consider tile checked - so inner loops do not return to this tile

	uint8 unitMoveCostOnTile = CurrentSelectedUnit->GetUnitMovementForTile(AGameTile::GetTerrainTypeAsByte(CurrentTile));

	// Tile is navigable if there is no enemy
	if (!tileHasEnemy && unitMoveCostOnTile != 255 && (CurrentTileDistance < NavigationDistance))
	{
		FoundNavigableTiles.AddUnique(CurrentTile);
	}
	if (tileHasEnemy || unitMoveCostOnTile == 255 || (CurrentTileDistance + unitMoveCostOnTile >= NavigationDistance)) // continue checking tiles without movement
	{
		// continue iterations if the tile has enemies, but do not consider walking past the unit
		if (!checkedTiles.Contains(CurrentTile->GetNorthTile()))
		{
			GetAvailableTilesLoop(CurrentSelectedUnit, CurrentTile->GetNorthTile(), MaxTileDistance, CurrentTileDistance + unitMoveCostOnTile, 0, InteractionDistance, WeaponActDistanceMin, WeaponActInstanceMax,
				weaponTargetsEnemies, weaponTargetsAllies, FoundNavigableTiles, FoundAttackableTiles, FoundInteractableTiles, checkedTiles);
		}
		if (!checkedTiles.Contains(CurrentTile->GetSouthTile()))
		{
			GetAvailableTilesLoop(CurrentSelectedUnit, CurrentTile->GetNorthTile(), MaxTileDistance, CurrentTileDistance + unitMoveCostOnTile, 0, InteractionDistance, WeaponActDistanceMin, WeaponActInstanceMax,
				weaponTargetsEnemies, weaponTargetsAllies, FoundNavigableTiles, FoundAttackableTiles, FoundInteractableTiles, checkedTiles);
		}
		if (!checkedTiles.Contains(CurrentTile->GetEastTile()))
		{
			GetAvailableTilesLoop(CurrentSelectedUnit, CurrentTile->GetNorthTile(), MaxTileDistance, CurrentTileDistance + unitMoveCostOnTile, 0, InteractionDistance, WeaponActDistanceMin, WeaponActInstanceMax,
				weaponTargetsEnemies, weaponTargetsAllies, FoundNavigableTiles, FoundAttackableTiles, FoundInteractableTiles, checkedTiles);
		}
		if (!checkedTiles.Contains(CurrentTile->GetWestTile()))
		{
			GetAvailableTilesLoop(CurrentSelectedUnit, CurrentTile->GetNorthTile(), MaxTileDistance, CurrentTileDistance + unitMoveCostOnTile, 0, InteractionDistance, WeaponActDistanceMin, WeaponActInstanceMax,
				weaponTargetsEnemies, weaponTargetsAllies, FoundNavigableTiles, FoundAttackableTiles, FoundInteractableTiles, checkedTiles);
		}
	}
	else
	{
		// continue iterations. 
		if (!checkedTiles.Contains(CurrentTile->GetNorthTile()))
		{
			GetAvailableTilesLoop(CurrentSelectedUnit, CurrentTile->GetNorthTile(), MaxTileDistance, CurrentTileDistance + unitMoveCostOnTile, NavigationDistance, InteractionDistance, WeaponActDistanceMin, WeaponActInstanceMax,
				weaponTargetsEnemies, weaponTargetsAllies, FoundNavigableTiles, FoundAttackableTiles, FoundInteractableTiles, checkedTiles);
		}
		if (!checkedTiles.Contains(CurrentTile->GetSouthTile()))
		{
			GetAvailableTilesLoop(CurrentSelectedUnit, CurrentTile->GetSouthTile(), MaxTileDistance, CurrentTileDistance + unitMoveCostOnTile, NavigationDistance, InteractionDistance, WeaponActDistanceMin, WeaponActInstanceMax,
				weaponTargetsEnemies, weaponTargetsAllies, FoundNavigableTiles, FoundAttackableTiles, FoundInteractableTiles, checkedTiles);
		}
		if (!checkedTiles.Contains(CurrentTile->GetEastTile()))
		{
			GetAvailableTilesLoop(CurrentSelectedUnit, CurrentTile->GetEastTile(), MaxTileDistance, CurrentTileDistance + unitMoveCostOnTile, NavigationDistance, InteractionDistance, WeaponActDistanceMin, WeaponActInstanceMax,
				weaponTargetsEnemies, weaponTargetsAllies, FoundNavigableTiles, FoundAttackableTiles, FoundInteractableTiles, checkedTiles);
		}
		if (!checkedTiles.Contains(CurrentTile->GetWestTile()))
		{
			GetAvailableTilesLoop(CurrentSelectedUnit, CurrentTile->GetWestTile(), MaxTileDistance, CurrentTileDistance + unitMoveCostOnTile, NavigationDistance, InteractionDistance, WeaponActDistanceMin, WeaponActInstanceMax,
				weaponTargetsEnemies, weaponTargetsAllies, FoundNavigableTiles, FoundAttackableTiles, FoundInteractableTiles, checkedTiles);
		}
	}

}

void ATileControlPawn::ClearSelectedTileData()
{
	for (AGameTile* navigableTile : NavigableTiles)
	{
		navigableTile->TriggerTileNavigable(false);
	}
	for (AGameTile* attackableTile : AttackableTiles)
	{
		attackableTile->TriggerTileAttackable(false);
	}
	for (AGameTile* interactableTile : InteractableTiles)
	{
		interactableTile->TriggerTileInteractable(false);
	}
	NavigableTiles.Empty();
	AttackableTiles.Empty();
	InteractableTiles.Empty();
}

void ATileControlPawn::SetCameraZoomSetting(FZoomLevelData CameraSetting)
{
	if (SpringArm)
	{
		SpringArm->TargetArmLength = CameraSetting.CameraDistance;
		FRotator currentRot = SpringArm->GetRelativeRotation();
		FRotator targetRot = currentRot;
		targetRot.Pitch = CameraSetting.CameraAngle;
		SpringArm->SetRelativeRotation(targetRot);
	}
}

void ATileControlPawn::SetCameraYaw(ECardinalDirections NewCardinalDir)
{
	FRotator springArmRot = SpringArm->GetTargetRotation();

	switch (NewCardinalDir)
	{
	case (ECardinalDirections::UP_DIR):
		springArmRot.Yaw = 0.0f;
		break;
	case (ECardinalDirections::RIGHT_DIR):
		springArmRot.Yaw = 270.0f;
		break;
	case (ECardinalDirections::DOWN_DIR):
		springArmRot.Yaw = 180.0f;
		break;
	case (ECardinalDirections::LEFT_DIR):
		springArmRot.Yaw = 90.0f;
		break;
	}

	SpringArm->SetWorldRotation(springArmRot);
	CurrentViewRotation = NewCardinalDir;
}

void ATileControlPawn::CursorMoveUp()
{
	if (HoverTile && IsPlayerPhase && !IsPausedForEvent && !IsUnitMoving && !IsUnitChoosingAction)
	{
		AGameTile* nextTile;
		switch (CurrentViewRotation)
		{
		case(ECardinalDirections::UP_DIR):
				nextTile = HoverTile->GetNorthTile();
				break;
		case(ECardinalDirections::RIGHT_DIR):
			nextTile = HoverTile->GetWestTile();
			break;
		case(ECardinalDirections::DOWN_DIR):
			nextTile = HoverTile->GetSouthTile();
			break;
		case(ECardinalDirections::LEFT_DIR):
			nextTile = HoverTile->GetEastTile();
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("COULD NOT DETERMINE CURRENT TileControlPawn ORIENTATION!"));
			return;
		}

		if (nextTile)
		{
			SetHoverTile(nextTile);
			SetViewTile(nextTile, false);
		}
	}
}

void ATileControlPawn::CursorMoveDown()
{
	if (HoverTile && IsPlayerPhase && !IsPausedForEvent && !IsUnitMoving && !IsUnitChoosingAction)
	{
		AGameTile* nextTile;
		switch (CurrentViewRotation)
		{
		case(ECardinalDirections::UP_DIR):
			nextTile = HoverTile->GetSouthTile();
			break;
		case(ECardinalDirections::RIGHT_DIR):
			nextTile = HoverTile->GetEastTile();
			break;
		case(ECardinalDirections::DOWN_DIR):
			nextTile = HoverTile->GetNorthTile();
			break;
		case(ECardinalDirections::LEFT_DIR):
			nextTile = HoverTile->GetWestTile();
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("COULD NOT DETERMINE CURRENT TileControlPawn ORIENTATION!"));
			return;
		}

		if (nextTile)
		{
			SetHoverTile(nextTile);
			SetViewTile(nextTile, false);
		}
	}
}

void ATileControlPawn::CursorMoveLeft()
{
	if (HoverTile && IsPlayerPhase && !IsPausedForEvent && !IsUnitMoving && !IsUnitChoosingAction)
	{
		AGameTile* nextTile;
		switch (CurrentViewRotation)
		{
		case(ECardinalDirections::UP_DIR):
			nextTile = HoverTile->GetWestTile();
			break;
		case(ECardinalDirections::RIGHT_DIR):
			nextTile = HoverTile->GetSouthTile();
			break;
		case(ECardinalDirections::DOWN_DIR):
			nextTile = HoverTile->GetEastTile();
			break;
		case(ECardinalDirections::LEFT_DIR):
			nextTile = HoverTile->GetNorthTile();
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("COULD NOT DETERMINE CURRENT TileControlPawn ORIENTATION!"));
			return;
		}

		if (nextTile)
		{
			SetHoverTile(nextTile);
			SetViewTile(nextTile, false);
		}
	}
}

void ATileControlPawn::CursorMoveRight()
{
	if (HoverTile && IsPlayerPhase && !IsPausedForEvent && !IsUnitMoving && !IsUnitChoosingAction)
	{
		AGameTile* nextTile;
		switch (CurrentViewRotation)
		{
		case(ECardinalDirections::UP_DIR):
			nextTile = HoverTile->GetEastTile();
			break;
		case(ECardinalDirections::RIGHT_DIR):
			nextTile = HoverTile->GetNorthTile();
			break;
		case(ECardinalDirections::DOWN_DIR):
			nextTile = HoverTile->GetWestTile();
			break;
		case(ECardinalDirections::LEFT_DIR):
			nextTile = HoverTile->GetSouthTile();
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("COULD NOT DETERMINE CURRENT TileControlPawn ORIENTATION!"));
			return;
		}

		if (nextTile != nullptr)
		{
			SetHoverTile(nextTile);
			SetViewTile(nextTile, false);
		}
	}
}

void ATileControlPawn::RotateViewRight()
{
	if (IsPlayerPhase && !IsPausedForEvent)
	{
		switch (CurrentViewRotation)
		{
		case (ECardinalDirections::UP_DIR):
			SetCameraYaw(ECardinalDirections::RIGHT_DIR);
			break;
		case (ECardinalDirections::RIGHT_DIR):
			SetCameraYaw(ECardinalDirections::DOWN_DIR);
			break;
		case (ECardinalDirections::DOWN_DIR):
			SetCameraYaw(ECardinalDirections::LEFT_DIR);
			break;
		case (ECardinalDirections::LEFT_DIR):
			SetCameraYaw(ECardinalDirections::UP_DIR);
			break;
		}
	}
}

void ATileControlPawn::RotateViewLeft()
{
	if (IsPlayerPhase && !IsPausedForEvent)
	{
		switch (CurrentViewRotation)
		{
		case (ECardinalDirections::UP_DIR):
			SetCameraYaw(ECardinalDirections::LEFT_DIR);
			break;
		case (ECardinalDirections::LEFT_DIR):
			SetCameraYaw(ECardinalDirections::DOWN_DIR);
			break;
		case (ECardinalDirections::DOWN_DIR):
			SetCameraYaw(ECardinalDirections::RIGHT_DIR);
			break;
		case (ECardinalDirections::RIGHT_DIR):
			SetCameraYaw(ECardinalDirections::UP_DIR);
			break;
		}

	}
}

void ATileControlPawn::ZoomInCamera()
{
	if (IsPlayerPhase && !IsPausedForEvent)
	{
		if (CurrentZoomLevel == 3)
		{
			SetCameraZoomSetting(ZoomMedSettings);
			CurrentZoomLevel = 2;
			return;
		}
		else if (CurrentZoomLevel == 2)
		{
			SetCameraZoomSetting(ZoomMaxSettings);
			CurrentZoomLevel = 1;
		}
	}
}

void ATileControlPawn::ZoomOutCamera()
{
	if (IsPlayerPhase && !IsPausedForEvent)
	{
		if (CurrentZoomLevel == 1)
		{
			SetCameraZoomSetting(ZoomMedSettings);
			CurrentZoomLevel = 2;
			return;
		}
		else if (CurrentZoomLevel == 2)
		{
			SetCameraZoomSetting(ZoomMinSettings);
			CurrentZoomLevel = 3;
		}
	}
}

void ATileControlPawn::InputSelectTile()
{
	if (!IsPlayerPhase || IsPausedForEvent)
	{
		return;
	}

	if (HoverTile && !SelectedTile && !IsUnitMoving && !IsUnitChoosingAction)	// Select a unit
	{
		AGameUnit* foundUnit;
		if (HoverTile->GetUnitOnTile(foundUnit)) // Get unit
		{
			if (foundUnit->UnitFaction != EUnitFaction::PLAYER) 
			{
				// skip this unit if they are not a player unit
				return;
			}

			if (AGameUnit::GetUnitRemainingActions(foundUnit) == 0 && AGameUnit::GetUnitRemainingSpaces(foundUnit) == 0)
			{
				// Unit has no need to be selected with no actions and spaces available
				return;
			}

			SetSelectedTile(HoverTile, foundUnit);	// set selected unit and tile
		}
	}
	else if (HoverTile && SelectedTile && !IsUnitMoving && !IsUnitChoosingAction)	// Select a destination for the unit
	{
		if (HoverTile && HoverTile->GetIsNavigable())
		{
			OnTileMovementConfirm.Broadcast();
		}
	}
	else if (IsUnitMoving || IsUnitChoosingAction)
	{
		OnCancelUnitMovementAndAction.Broadcast();
	}

}

void ATileControlPawn::InputCancelTile()
{
	if (IsPlayerPhase && SelectedTile)
	{
		if (IsUnitMoving && !IsUnitChoosingAction)
		{
			// if a unit is moving, undo the move but keep the unit selected
			// if the unit is moved, undo is done through the unit turn menu
			CancelUnitMovementAndAction();
		}
		else if (!IsUnitMoving && !IsUnitChoosingAction)
		{
			// if the unit has not moved yet, undo the unit selection
			SetSelectedTile(nullptr, nullptr);
		}

	}
}

const AGameTile* ATileControlPawn::GetViewTile()
{
	return ViewTile;
}

const AGameTile* ATileControlPawn::GetHoverTile()
{
	return HoverTile;
}

const AGameTile* ATileControlPawn::GetSelectedTile()
{
	return SelectedTile;
}

void ATileControlPawn::SetViewTile(AGameTile* Tile, bool SnapImmediately)
{
	ViewTile = Tile;

	if (ViewTile)
	{
		if (SpringArm->bEnableCameraLag == SnapImmediately)
			SpringArm->bEnableCameraLag = !SnapImmediately;
		SetActorLocation(ViewTile->GetActorLocation());
		OnTileView.Broadcast(ViewTile);
	}
}

void ATileControlPawn::SetHoverTile(AGameTile* Tile)
{
	if (HoverTile && HoverTile != Tile)
	{
		HoverTile->TriggerTileUnhover();
	}

	HoverTile = Tile;
	if (HoverTile)
	{
		HoverTile->TriggerTileHover();
		OnTileHover.Broadcast(HoverTile);
	}
}

void ATileControlPawn::SetSelectedTile(AGameTile* Tile, AGameUnit* Unit)
{
	if (SelectedTile && SelectedTile != Tile)
	{
		SelectedTile->TriggerTileUnselected();
	}
	SelectedTile = Tile;
	SelectedUnit = Unit;
	SelectedUnitDir = Unit ? Unit->GetCurrentUnitDirection() : ECardinalDirections::NONE;
	ClearSelectedTileData();
	if (SelectedTile)
	{
		SelectedTile->TriggerTileSelected();
		GetAvailableTilesForSelectedUnit(SelectedTile, SelectedUnit, NavigableTiles, AttackableTiles, InteractableTiles);
	}

	OnUnitTileSelected.Broadcast(Tile, Unit);

	if (!Tile || !Unit)
	{
		OnCancelUnitSelection.Broadcast();
	}
}

void ATileControlPawn::SetUnitMovingToTile(AGameUnit* Unit, AGameTile* Tile, ECardinalDirections Direction)
{
	Unit->MoveUnitToTile(Tile);
	Unit->SetCurrentUnitDirection(Direction);
	IsUnitMoving = true;
}

void ATileControlPawn::SetUnitMovedToTile(AGameUnit* Unit, AGameTile* Tile)
{
	// Unit moved to the target tile - now display options on the new tile
	IsUnitMoving = false;
	IsUnitChoosingAction = true;
	UnitReadyForActions(Unit, Tile);
}

ECardinalDirections ATileControlPawn::GetCurrentCameraRotation()
{
	return CurrentViewRotation;
}

