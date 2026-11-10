// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1HealthComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
UIP1HealthComponent::UIP1HealthComponent()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = false;

	// Allow this component to replicate with its owner.
	SetIsReplicatedByDefault(true);
}

// Called when the game starts or when spawned
void UIP1HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only the server initializes authoritative health.
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentHealth = MaxHealth;
	}
}

void UIP1HealthComponent::ApplyDamage(float DamageAmount)
{
	// Never allow a client to modify authoritative health.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (DamageAmount <= 0.0f || IsDead())
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	UE_LOG(LogTemp, Log,
	   TEXT("%s took %.1f damage. Health: %.1f / %.1f"),
	   *GetOwner()->GetName(),
	   DamageAmount,
	   CurrentHealth,
	   MaxHealth);

	// Death handling will be added in a later milestone.

	if (CurrentHealth <= 0.0f)
	{
		OnDeath.Broadcast();
	}
}

void UIP1HealthComponent::Heal(float HealAmount)
{
	// Only the server can restore health.
	if(!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (HealAmount <= 0.0f || IsDead())
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
}

bool UIP1HealthComponent::IsDead() const
{
	return CurrentHealth <= 0.0f;
}

void UIP1HealthComponent::ResetHealth()
{
	// Only the server can reset authoritative health.
	if (!GetOuter() || !GetOwner()->HasAuthority())
	{
		return;
	}

	CurrentHealth = MaxHealth;

	// Add any existing death-state reset here if your
	// health component tracks whether it has already died.
}

float UIP1HealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float UIP1HealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

void UIP1HealthComponent::OnRep_CurrentHealth()
{
	// Client-side response to a replicated health chnage.
	UE_LOG(LogTemp, Log, TEXT("%s health updated: %.1f / %.1f"), GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), CurrentHealth, MaxHealth);

	// Later, notify the HUD or player a damage indicator here.
}

void UIP1HealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UIP1HealthComponent, CurrentHealth);
}

