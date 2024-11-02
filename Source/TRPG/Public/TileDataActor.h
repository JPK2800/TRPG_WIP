// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameTile.h"
#include "GameFramework/Actor.h"
#include "TileDataActor.generated.h"

UCLASS()
class TRPG_API ATileDataActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATileDataActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	AGameTile* StartingViewTile;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	AGameTile* StartingHoverTile;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<AGameTile*> PlayerStartingTiles;

};
