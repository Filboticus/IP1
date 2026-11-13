// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IP1DeploymentWidget.generated.h"

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
};
