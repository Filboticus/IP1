// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "IP1/GameModes/IP1TeamTypes.h"
#include "IP1PlayerStart.generated.h"

UCLASS()
class IP1_API AIP1PlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	// Declare constructor taking FObjectInitializer
	AIP1PlayerStart(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Spawn")
	EIP1Team Team = EIP1Team::None;
};
