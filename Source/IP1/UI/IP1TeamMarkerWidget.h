// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IP1/GameModes/IP1TeamTypes.h"
#include "IP1TeamMarkerWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS()
class IP1_API UIP1TeamMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(Blueprintable, Category = "Team Marker")
	void SetTeam(EIP1Team NewTeam);

protected:
	// Must match the name of the image in the Widget Blueprint
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> TeamIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team Marker")
	TObjectPtr<UTexture2D> AlliedTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team Marker")
	TObjectPtr<UTexture2D> AxisTexture;
};
