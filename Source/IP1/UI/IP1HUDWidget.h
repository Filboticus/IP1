// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IP1HUDWidget.generated.h"

class UTextBlock;
class UProgressBar;

/**
 * 
 */

UCLASS()
class IP1_API UIP1HUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MatchPhaseText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MatchTimerText;

private:
	void UpdateHealth();

	void UpdateMatchStatus();
};
