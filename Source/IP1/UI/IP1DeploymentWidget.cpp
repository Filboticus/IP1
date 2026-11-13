// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1DeploymentWidget.h"

void UIP1DeploymentWidget::RefreshDeploymentUI()
{
	// ToDo:
	// Refresh squad list
	// Refresh available roles
	// Refresh spawn locations
}

void UIP1DeploymentWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	RefreshDeploymentUI();
}
