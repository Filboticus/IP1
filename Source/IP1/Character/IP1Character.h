// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "IP1Character.generated.h"

class AIP1PlayerState;
class UInputAction;
class UCameraComponent;
class UIP1HealthComponent;
class UIP1WeaponComponent;
class UWidgetComponent;
enum class EIP1Team : uint8;
struct FInputActionValue;


DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class IP1_API AIP1Character : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First Person Camera */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Weapon Component */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	UIP1WeaponComponent* WeaponComponent;

public:
	// Sets default values for this character's properties
	AIP1Character();

	/*
	 ***** Death State *****
	Function		Responsibility
	IsDead()		Lets other classes query whether the character is dead.
	Die()			Server-side entry point for death.
	HandleDeath()	Disables gameplay and applies death behavior.
	OnRep_IsDead()	Updates client-side behavior when the replicated state changes.
	HandleRespawn()	Restores the character after respawn.
	SetDeathState()	Updates the death flag on the server.

	* We will use ReplicatedUsing because changing a replicated variable does not
	* automatically invoke gameplay behavior on clients unless you explicitly
	* arrange for it.
	*/
	
	UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleAnywhere, BlueprintReadOnly, Category = "Death")
	bool bIsDead = false;

	UFUNCTION()
	void OnRep_IsDead();

	UFUNCTION(BlueprintCallable, Category = "Death")
	bool IsDead() const;

	UFUNCTION()
	void Die();

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleRespawn();

	UFUNCTION()
	void SetDeathState(bool bNewIsDead);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	
	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:
	/** Returns Mesh1P suboject */
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	/** Returns FirstPersonCameraComponent subobject*/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Health Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	TObjectPtr<UIP1HealthComponent> HealthComponent;

	UFUNCTION(BlueprintPure, Category = "Health")
	UIP1HealthComponent* GetHealthComponent() const;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void OnRep_PlayerState() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> TeamMarkerWidget;

private:
	void RefreshTeamMarkerBinding();

	void UpdateTeamMarker(EIP1Team NewTeam);

	UPROPERTY()
	TObjectPtr<AIP1PlayerState> BoundPlayerState;

	FDelegateHandle TeamChangedHandle;
};
