// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IP1HealthComponent.generated.h"

/** ToDo:
 * Name                    Purpose
 * ---                     ---
 * MaxHealth               Maximum health, initially 100
 * CurrentHealth           Current health, replicated from the server
 * ApplyDamage()           Reduces health on the server
 * Heal()                  Restores health on the server
 * IsDead()                Checks whether healt has reached zero
 * OnRep_CurrentHealth()   Responds to health replication on clients
*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup = (IP1), meta = (BlueprintSpawnableComponent))
class IP1_API UIP1HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UIP1HealthComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Call on clients when CurrentHealth is replicated.
	UFUNCTION()
	void OnRep_CurrentHealth();

public:
	// Maximum health for this actor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.0f;

	// The server owns this value and replicates it to clients.
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.0f;

	// These functions only change health on the authoritative server.
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float HealAmount);

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const;

	// Enables CurrentHealth replication.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
