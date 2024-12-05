// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatGameMode.h"
#include "GameFramework/PlayerController.h"
#include "GamePlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpPress);										// Up pressed				(left-stick)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDownPress);										// Down pressed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLeftPress);										// Left pressed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRightPress);										// Right pressed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FConfirmPress);									// Confirmation pressed		(A default)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCancelPress);									// Cancel pressed			(B default)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNextUnitPress);									// NextUnit pressed			(X default)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMoreInfoPress);									// MoreInfo pressed			(Y default)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMapPress);										// Map pressed				(LT default)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FToggleDisplaysPress);							// Toggle tile displays		(RT default)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCamLeftPress);									// Move Cam Left pressed	(right-stick)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCamRightPress);									// Move Cam Right pressed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCamUpPress);										// Move Cam Up pressed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCamDownPress);									// Move Cam Down pressed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMenuPress);										// Menu pressed				(menu default)

// Player controller
// Inputs are defined in the Player Controller blueprint 
UCLASS(Blueprintable)
class TRPG_API AGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	virtual void BeginPlay() override;

public:

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FUpPress OnUpPress;								// Fires when the player presses Up

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FDownPress OnDownPress;							// Fires when the player presses Down

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FLeftPress OnLeftPress;							// Fires when the player presses Left

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FRightPress OnRightPress;						// Fires when the player presses Right

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FConfirmPress OnConfirmPress;					// Fires when the player presses Confirm

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FCancelPress OnCancelPress;						// Fires when the player presses Cancel

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FNextUnitPress OnNextUnitPress;					// Fires when the player presses NextUnit

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FMoreInfoPress OnMoreInfoPress;					// Fires when the player presses MoreInfo

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FMapPress OnMapPress;							// Fires when the player presses Map

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FToggleDisplaysPress OnToggleDisplayPress;		// Fires when the player presses ToggleDisplay

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FCamLeftPress OnCamLeftPress;					// Fires when the player presses CamLeft

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FCamRightPress OnCamRightPress;					// Fires when the player presses CamRight

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FCamUpPress OnCamUpPress;						// Fires when the player presses CamUp

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FCamDownPress OnCamDownPress;					// Fires when the player presses CamDown

	UPROPERTY(BlueprintAssignable, Category = "PlayerInput")
	FMenuPress OnMenuPress;							// Fires when the player presses Menu

public:

	UFUNCTION(BlueprintCallable)
	virtual void TriggerUp();		// Fires the up event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerDown();		// Fires the down event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerLeft();		// Fires the left event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerRight();	// Fires the right event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerConfirm();	// Fires the confirm event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerCancel();	// Fires the cancel event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerNextUnit();	// Fires the next-unit event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerMoreInfo();	// Fires the more-info event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerMapPress();	// Fires the map event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerToggleDisplay();	// Fires the toggle-display event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerCamLeft();	// Fires the cam left event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerCamRight();	// Fires the cam right event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerCamUp();	// Fires the cam up event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerCamDown();	// Fires the cam down event

	UFUNCTION(BlueprintCallable)
	virtual void TriggerMenu();		// Fires the menu event

protected:

	virtual bool LinkToCombatGameMode();	// Makes a binding to the game mode to display phase shifts on the ui

	UFUNCTION()
	virtual void CombatPhaseChanged(ECombatPhase NewPhase, ECombatPhase PreviousPhase, uint8 TurnNumber);	// binding from game mode on phase change

	UFUNCTION(BlueprintImplementableEvent)
	void DisplayPhaseChange(ECombatPhase NewPhase);	// Displays a phase change screen
};
