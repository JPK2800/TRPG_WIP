// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IUnitCombatAnimations.generated.h"

// Interface to be implemented on in-combat unit animation blueprints
UINTERFACE(MinimalAPI)
class UIUnitCombatAnimations : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TRPG_API IIUnitCombatAnimations
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	void SetIdle();			// Sets the unit to the Idle animation

	void SetRun();			// Sets the unit to the Run animation

	void SetAttack1();		// Sets the unit to the Attack1 animation

	void SetAttack2();		// Sets the unit to the Attack2 animation

	void SetBackstep();		// Sets the unit to the Backstep animation

	void SetHitStagger();	// Sets the unit to the Hit animation


};
