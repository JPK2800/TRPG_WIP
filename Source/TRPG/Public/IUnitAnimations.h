// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IUnitAnimations.generated.h"

// Interface to be implemented on overworld-unit animation blueprints
UINTERFACE(MinimalAPI)
class UIUnitAnimations : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TRPG_API IIUnitAnimations
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetIdle();			// Sets the unit to the Idle animation

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetRun();			// Sets the unit to the Run animation

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetNewEquipWeaponType(uint8 WeaponType, USkeletalMesh* SkeletalMesh);	// Sets the unit to the correct weapon type animations

};
