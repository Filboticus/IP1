// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IP1DeploymentWidget.generated.h"

class AIP1GameState;
/**
 * 
 */
UCLASS()
class IP1_API UIP1DeploymentWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "Deployment")
	void RefreshDeploymentUI();
	
protected:
	
	virtual void NativeConstruct() override;
	
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void HandleSquadSummariesChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Deployment|Squad")
	void BP_RefreshSquadList();
	
	UPROPERTY()
	TObjectPtr<AIP1GameState> CachedGameState;
};
