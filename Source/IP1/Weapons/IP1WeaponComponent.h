// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "IP1WeaponComponent.generated.h"

class UCameraComponent;
class AIP1Character;
class AIP1PlayerController;

/**
 *  Weapon Component Class
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class IP1_API UIP1WeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties */
	UIP1WeaponComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 MaxAmmo = 8;
	
	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	/** Sound to play when there is no ammo & trying fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* DryFireSound;

	/** Sound to play when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* ReloadSound;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* WeaponMappingContext;

	/** AimDownSights Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta=(AllowPrivateAccess = "true"))
	class UInputAction* AimDownSightsAction;

	/** SteadyAim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta=(AllowPrivateAccess = "true"))
	class UInputAction* SteadyAimAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta=(AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta=(AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;
	
	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EquipWeapon(AIP1Character* TargetCharacter);

	/** Fire the Weapon */
	// Called by the locally controlled character.
	// Calculates the intended shot on the client.
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();
	
	/** Reload the Weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();

	/** Aim Down Sights */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AimDownSights(const FInputActionInstance& Instance);

	/** Steady Aim */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SteadyAim(const FInputActionInstance& Instance);

	UFUNCTION(BlueprintCallable, Category = "Weapon | Aiming")
	void StartZoom(float ZoomedFOV);

	UFUNCTION(BlueprintCallable, Category = "Weapon | Aiming")
	void EndZoom();

	

protected:
	/** Begins gameplay for this component. */
	UFUNCTION()
	virtual void BeginPlay() override;

	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Executed by the server, not the client.
	UFUNCTION(Server, Reliable, Category = "Combat")
	void ServerFire(FVector_NetQuantize ClientViewLocation, FVector_NetQuantizeNormal ClientShotDirection);
	
	// Server-side weapon settings.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float WeaponDamage = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float WeaponRange = 10000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float FireInterval = 0.30f;

	// Reject shot origins too far from the server's viewpoint.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Validation")
	float MaxViewOriginError = 200.0f;

	// Reject shot directions too far from server-known aim.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Validation")
	float MinAimDirectionDot = 0.70f;

	// Server-only fire-rate tracking.
	float LastServerFireTime = -1000.0f;

	/**
	 * Core State Variables
	 */
	// The boolean tracking your aiming state
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAiming = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsSteadyAiming = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bCanSteadyAim = false;

	// The boolean tracking if reload is possible
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bCanReload= false;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bCanFire = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float FireDuration = 0.5; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ReloadDuration = 8.0f; // X amount of time

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Aiming")
	float DefaultFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Aiming")
	float SAZoomedFOV = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Aiming")
	float ADSZoomedFOV = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Aiming")
	float ZoomInterSpeed = 12.0f;
	
	FTimerHandle ReloadTimerHandle;
	
	UFUNCTION()
	void ResetFireDelay();

private:
	/** The Character holding this weapon*/
	UPROPERTY()
	AIP1Character* Character;

	UFUNCTION()
	void UpdateZoom();

	UPROPERTY()
	float TargetFOV;

	UPROPERTY()
	FTimerHandle ZoomTimerHandle;

	UPROPERTY()
	ACharacter* OwningCharacter;

	UPROPERTY()
	UCameraComponent* PlayerCamera;

	
};
