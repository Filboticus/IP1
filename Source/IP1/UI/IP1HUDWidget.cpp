// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1HUDWidget.h"
#include "IP1/Character/IP1Character.h"
#include "IP1/Character/IP1HealthComponent.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "IP1/GameModes/IP1GameState.h"

void UIP1HUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateHealth();
	UpdateMatchStatus();
}

void UIP1HUDWidget::UpdateHealth()
{
	APlayerController* PC = GetOwningPlayer();

	if(!PC)
	{
		return;
	}

	AIP1Character* Character = Cast<AIP1Character>(PC->GetPawn());

	if(!Character)
	{
		return;
	}

	// ToDo: 
	// Read CurrentHealth and MaxHealth
	// from your existing IP1 character/health system.
	//
	// Example:
	// const float CurrentHealth = ...
	// const float MaxHealth = ...

	// Update HealthBar and HealthText once the
	// existing health property names are connected.

	UIP1HealthComponent* HealthComponent = Character->GetHealthComponent();

	if (!HealthComponent)
	{
		return;
	}
	
	const float CurrentHealth = HealthComponent->GetCurrentHealth();
	const float MaxHealth = HealthComponent->GetMaxHealth();
	
	const float HealthPercent = MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;

	if (HealthBar)
	{
		HealthBar->SetPercent(HealthPercent);
	}

	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString(FString::Printf(TEXT("Health %.0f / %.0f"), CurrentHealth, MaxHealth))));
	}
}

void UIP1HUDWidget::UpdateMatchStatus()
{
	APlayerController* PC = GetOwningPlayer();

	if (!PC)
	{
		return;
	}

	AIP1GameState* GS = GetWorld()->GetGameState<AIP1GameState>();

	if (!GS)
	{
		return;
	}

	// Update match phase
	if (MatchPhaseText)
	{
		MatchPhaseText->SetText(FText::FromString(GS->GetMatchPhaseString()));

		// MatchPhaseText->SetText(FText::FromString(UEnum::GetValueAsString(GS->GetMatchPhase()));
	}

	// ToDo:
	// Read the time remaining from your existing IP1GameState countdown.
	// Do not create another independent match timer.

	// Update match timer
	if(MatchTimerText)
	{
		//MatchTimerText->SetText(FText::FromString(GS->GetMatchTimeRemaining()))

		const float TimeRemaining = GS->GetMatchTimeRemaining();

		const int32 TotalSeconds = FMath::Max(0, FMath::FloorToInt(TimeRemaining));

		const int32 Minutes = TotalSeconds / 60;
		const int32 Seconds = TotalSeconds % 60;

		MatchTimerText->SetText(
			FText::FromString(
				FString::Printf(
					TEXT("%02d:%02d"),
					Minutes,
					Seconds)));
			
	}
	
}
