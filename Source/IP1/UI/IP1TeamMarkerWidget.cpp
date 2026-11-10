// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1TeamMarkerWidget.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"


void UIP1TeamMarkerWidget::SetTeam(EIP1Team NewTeam)
{
	if (!TeamIcon)
	{
		return;
	}

	switch (NewTeam)
	{
	case EIP1Team::Allies:
		TeamIcon->SetBrushFromTexture(AlliedTexture);
		TeamIcon->SetVisibility(ESlateVisibility::Visible);
		break;

	case EIP1Team::Axis:
		TeamIcon->SetBrushFromTexture(AxisTexture);
		TeamIcon->SetVisibility(ESlateVisibility::Visible);
		break;
		
	case EIP1Team::None:
	default:
		// No team assigned yet: don't display an incorrect emblem.
		TeamIcon->SetVisibility(ESlateVisibility::Collapsed);
		break;
	}
}
