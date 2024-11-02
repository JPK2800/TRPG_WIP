// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryItem.generated.h"

UENUM(BlueprintType)
enum EItemType : uint8
{
	MISC = 0					UMETA(DisplayName = "MISC"),							// Item that cannot be consumed or equipped
	USABLE	 = 1				UMETA(DisplayName = "USABLE"),							// Item used by a unit
	EQUIPPABLE = 2				UMETA(DisplayName = "EQUIPPABLE"),						// Item can be equipped by a unit (uses the EquipmentSlotName variable)
};

// Inventory item
UCLASS(Blueprintable)
class TRPG_API UInventoryItem : public UObject
{
	GENERATED_BODY()

public:

	UInventoryItem();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Item Data")
	FName DisplayName;	// Object inventory name

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item Data")
	TEnumAsByte<EItemType> ItemType;	// Item type

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item Data")
	bool IsKeyItem;		// Item is a key item and cannot be sold / discarded if true

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EQUIPPABLE Item Data")
	FString EquipmentSlotName;	// Slot name for equippable items (if applicable)

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "USABLE Item Data")
	bool IsUsableInCombat;		// True if this item can be "used" in combat (Healing potion for example)

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "USABLE Item Data")
	bool IsUsableOutOfCombat;	// True if this item can be "used" outside of combat

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item Data", meta = (EditCondition = "IsDurabilityItem==false"))
	int MaxStacks = 1;	// Max number of stacks for an item. Default is 1. Logic conflicts with durability items.

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item Data", meta = (EditCondition = "MaxStacks==1"))
	bool IsDurabilityItem = false;	// Item uses durability if true. Logic conflicts with MaxStacks being anything but 1.

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item Data", meta = (EditCondition = "IsDurabilityItem==true"))
	uint8 DurabilityMaximum;		// If IsDurabilityItem, this is the max number of uses for the item. 

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item Data", meta=(EditCondition="IsDurabilityItem==true"))
	uint8 DurabilityRemaining;		// If IsDurabilityItem, track the number of "uses" remaining. 0 is unusable. 

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item Data", meta = (EditCondition = "IsDurabilityItem==true"))
	bool IsDurabilityRestoredAfterCombat = false;	// If true, durability will be restored to the maximum after combat

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "USABLE Item Data")
	bool IsDestroyedOnUse = false;	// Item destroys when it is used.

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item Data")
	bool IsDestroyedOnDurabilityBreak = false;	// Item destroys when durability reaches 0.000f.

};
