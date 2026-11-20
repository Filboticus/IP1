// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1DeploymentWidget.h"
#include "IP1/GameModes/IP1GameState.h"

void UIP1DeploymentWidget::RefreshDeploymentUI()
{
	/***  Refresh Squads ***/
	
	// Refresh Squads List
	BP_RefreshSquadList();
	
	// Future:
	// Refresh roles
	// Refresh spawn locations
}

void UIP1DeploymentWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CachedGameState = GetWorld()
		? GetWorld()->GetGameState<AIP1GameState>()
		: nullptr;

	if (IsValid(CachedGameState))
	{
		CachedGameState->OnSquadSummariesChanged
			.AddUniqueDynamic(
				this, &UIP1DeploymentWidget::HandleSquadSummariesChanged);
	}

	RefreshDeploymentUI();
}

void UIP1DeploymentWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (IsValid(CachedGameState))
	{
		CachedGameState->OnSquadSummariesChanged
			.RemoveDynamic(this, &UIP1DeploymentWidget::HandleSquadSummariesChanged);
	}

	CachedGameState = nullptr;

	Super::NativeDestruct();
}

void UIP1DeploymentWidget::HandleSquadSummariesChanged()
{
	RefreshDeploymentUI();
}