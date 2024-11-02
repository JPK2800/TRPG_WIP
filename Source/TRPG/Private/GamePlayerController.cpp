// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlayerController.h"

void AGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	LinkToCombatGameMode();
}

void AGamePlayerController::TriggerUp()
{
	OnUpPress.Broadcast();
}

void AGamePlayerController::TriggerDown()
{
	OnDownPress.Broadcast();
}

void AGamePlayerController::TriggerLeft()
{
	OnLeftPress.Broadcast();
}

void AGamePlayerController::TriggerRight()
{
	OnRightPress.Broadcast();
}

void AGamePlayerController::TriggerConfirm()
{
	OnConfirmPress.Broadcast();
}

void AGamePlayerController::TriggerCancel()
{
	OnCancelPress.Broadcast();
}

void AGamePlayerController::TriggerNextUnit()
{
	OnNextUnitPress.Broadcast();
}

void AGamePlayerController::TriggerMoreInfo()
{
	OnMoreInfoPress.Broadcast();
}

void AGamePlayerController::TriggerMapPress()
{
	OnMapPress.Broadcast();
}

void AGamePlayerController::TriggerToggleDisplay()
{
	OnToggleDisplayPress.Broadcast();
}

void AGamePlayerController::TriggerCamLeft()
{
	OnCamLeftPress.Broadcast();
}

void AGamePlayerController::TriggerCamRight()
{
	OnCamRightPress.Broadcast();
}

void AGamePlayerController::TriggerCamUp()
{
	OnCamUpPress.Broadcast();
}

void AGamePlayerController::TriggerCamDown()
{
	OnCamDownPress.Broadcast();
}

void AGamePlayerController::TriggerMenu()
{
	OnMenuPress.Broadcast();
}

bool AGamePlayerController::LinkToCombatGameMode()
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

	combatGameMode->OnTriggerPhase.AddDynamic(this, &AGamePlayerController::CombatPhaseChanged);
	combatGameMode->BeginFirstPhase();
	return true;
}

void AGamePlayerController::CombatPhaseChanged(ECombatPhase NewPhase, ECombatPhase PreviousPhase, uint8 TurnNumber)
{
	DisplayPhaseChange(NewPhase);
}
