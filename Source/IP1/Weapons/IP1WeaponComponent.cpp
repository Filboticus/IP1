// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1WeaponComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "IP1/Character/IP1Character.h"
#include "IP1/Player/IP1PlayerController.h"
#include "IP1/Character/IP1HealthComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UIP1WeaponComponent::UIP1WeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Needed for component networking and Server RPC support.
	SetIsReplicatedByDefault(true);

	CurrentAmmo = MaxAmmo;

	TargetFOV = DefaultFOV;
}

bool UIP1WeaponComponent::EquipWeapon(AIP1Character* TargetCharacter)
{
	Character = TargetCharacter;

	// Check that the character is valid, and has no weapon component yet
	if (Character == nullptr || Character->GetInstanceComponents().FindItemByClass<UIP1WeaponComponent>())
	{
		return false;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint"))); //GetMesh1P Function may not exist?
	
	// Add the weapon as an instance component to the character
	Character->AddInstanceComponent(this);

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(WeaponMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire Input Action Binding
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &UIP1WeaponComponent::Fire);

			// Reload Input Action Binding
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &UIP1WeaponComponent::Reload);

			// Aim Down Sights Input Action Binding
			// Bind to ETriggerEvent::Triggered to catch the initial press and hold
			EnhancedInputComponent->BindAction(AimDownSightsAction, ETriggerEvent::Triggered, this, &UIP1WeaponComponent::AimDownSights);

			// Aim Down Sights Input Action Binding
			// Bind to ETriggerEvent::Completed to catch the release
			EnhancedInputComponent->BindAction(AimDownSightsAction, ETriggerEvent::Completed, this, &UIP1WeaponComponent::AimDownSights);

			// Steady Aim Input Action Binding
			// Bind to ETriggerEvent::Triggered to catch the initial press and hold
			EnhancedInputComponent->BindAction(SteadyAimAction, ETriggerEvent::Triggered, this, &UIP1WeaponComponent::SteadyAim);

			// Steady Aim Input Action Binding
			// Bind to ETriggerEvent::Completed to catch the release
			EnhancedInputComponent->BindAction(SteadyAimAction, ETriggerEvent::Completed, this, &UIP1WeaponComponent::SteadyAim);
		}
	}

	return true;
}

void UIP1WeaponComponent::Fire()
{
	Character = Cast<AIP1Character>(GetOwner());

	if (!Character)
	{
		UE_LOG(LogTemp, Warning,
		   TEXT("Fire failed: no character owner"));

		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("Fire called on %s | Local: %d | Authority: %d"),
		*Character->GetName(),
		Character->IsLocallyControlled(),
		Character->HasAuthority());

	if (!Character->IsLocallyControlled())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning,
		   TEXT("Fire failed: no PlayerController"));
		return;
	}
	
	// Ensure we can Fire
	if(!bCanFire) return;
	
	// Check if we have ammo.
	if(CurrentAmmo > 0)
	{
		// Reduce ammo count.
		CurrentAmmo--;
		
		// Set CanFire -> FALSE
		bCanFire = false;
		
		// Try and play the sound if specified
		if (FireSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
		}
		// ToDo: Insert line trace logic here (Will Need to move into a functions)
		// Call function calculate shot fired and if it hit anything on the server

		// CLIENT: Calculate the intended shot from the actual player view.
		FVector ViewLocation;
		FRotator ViewRotation;

		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

		const FVector ShotDirection = ViewRotation.Vector();

		UE_LOG(LogTemp, Warning,
		TEXT("Sending ServerFire RPC from %s"),
		*Character->GetName());

		// Optional client-side cosmetic prediction can happen here.
		// Do not apply damage or change target health on the client.

		// Send the intended shot to the server for validation.
		ServerFire(ViewLocation, ShotDirection);

		UE_LOG(LogTemp, Log, TEXT("Shot fired! Ammo remaining: %d"), CurrentAmmo);
		
		// Set a timer to call ResetFireDelay after FireDuration seconds.
		GetWorld()->GetTimerManager().SetTimer(
			ReloadTimerHandle,
			this,
			&UIP1WeaponComponent::ResetFireDelay,
			FireDuration,
			false);
		
		// Set Can Reload
		bCanReload = true;

		
	}
	else
	{
		// Set Can Reload
		bCanReload = true;

		if (DryFireSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DryFireSound, Character->GetActorLocation());
		}

		// Handle dry fire
		UE_LOG(LogTemp, Warning, TEXT("Out of ammo! Need to reload!"));
	}
	
	// Multiplayer Setup End
	// Move code below to ServerFire()
	
}

void UIP1WeaponComponent::ServerFire_Implementation(FVector_NetQuantize ClientViewLocation, FVector_NetQuantizeNormal ClientShotDirection)
{
	// SERVER: Verify the owning character.

	// Resolve the character on THIS instance of the component.
	Character = Cast<AIP1Character>(GetOwner());

	AActor* ComponentOwner = GetOwner();

	UE_LOG(LogTemp, Error,
		TEXT("Weapon Component: %s"),
		*GetName());

	UE_LOG(LogTemp, Error,
		TEXT("Component Owner: %s"),
		ComponentOwner
			? *ComponentOwner->GetName()
			: TEXT("NULL"));

	UE_LOG(LogTemp, Error,
		TEXT("Component Owner Class: %s"),
		ComponentOwner
			? *ComponentOwner->GetClass()->GetName()
			: TEXT("NULL"));

	
	UE_LOG(LogTemp, Warning,
		TEXT("ServerFire RPC received. Owner: %s | Authority: %d"),
		Character ? *Character->GetName() : TEXT("None"),
		Character ? Character->HasAuthority() : false);

	UE_LOG(LogTemp, Warning,
	   TEXT("ServerFire_Implementation | Component: %s | Owner: %s | OwnerClass: %s"),
	   *GetName(),
	   GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
	   GetOwner() ? *GetOwner()->GetClass()->GetName() : TEXT("None"));

	UE_LOG(LogTemp, Warning,
		TEXT("Character pointer valid: %s"),
		Character ? TEXT("YES") : TEXT("NO"));

	if (Character)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Character: %s | HasAuthority: %d | Role: %d"),
			*Character->GetName(),
			Character->HasAuthority(),
			static_cast<int32>(Character->GetLocalRole()));
	}

	
	if (!Character || !Character->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("ServerFire rejected: invalid character or no authority"));
		return;
	}

	// This can probably be included in the above if statement?
	if(Character->IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("Character->IsDead()"));
		return;
	}

	AController* Controller = Character->GetController();

	if (!Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerFire(): No Valid Controller!"));
		return;
	}

	// Ensure we have a valid world context
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerFire(): No Valid World!"));
		return;
	}
	
	// SERVER: Enforce the fire-rate limit.
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	if(CurrentTime - LastServerFireTime < FireInterval)
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerFire(): CurrentTime - LastServerFireTime < FireInterval"));
		return;
	}

	// SERVER: Validate that the client shot starts near
	// the server's own character viewpoint.
	FVector ServerEyeLocation;
	FRotator ServerEyeRotation;

	Character->GetActorEyesViewPoint(ServerEyeLocation, ServerEyeRotation);

	const float OriginError = FVector::Dist(FVector(ClientViewLocation), ServerEyeLocation);

	if (OriginError > MaxViewOriginError)
	{
		UE_LOG(LogTemp, Warning, TEXT("OriginError > MaxViewOriginError"));
		return;
	}

	// SERVER: Validate that the direction is plausible.
	const FVector ShotDirection = FVector(ClientShotDirection).GetSafeNormal();

	if (ShotDirection.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("ShotDirection.IsNearlyZero()"));
		return;
	}

	const FVector ServerAimDirection = Controller->GetControlRotation().Vector();

	const float AimDot = FVector::DotProduct(ShotDirection, ServerAimDirection);

	if(AimDot < MinAimDirectionDot)
	{
		UE_LOG(LogTemp, Warning, TEXT("AimDot < MinAimDirectionDot"));
		return;
	}

	// The server uses its own origin, never the client's
	// supplied origin, to perform the authoritative trace.
	const FVector TraceStart = ServerEyeLocation;

	const FVector TraceEnd = TraceStart + ShotDirection * WeaponRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	FHitResult Hit;

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams);

	if (!bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("!bHit"));
		return;
	}

	AActor* HitActor = Hit.GetActor();

	if (!HitActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("!bHitActor"));
		return;
	}

	// Find the health component on the hit actor.

	UIP1HealthComponent* TargetHealthComponent = HitActor->FindComponentByClass<UIP1HealthComponent>();

	if (!TargetHealthComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("!TargetHealthComponent"));
		return;
	}
	
	// Only the server changes health
	TargetHealthComponent->ApplyDamage(WeaponDamage);

	UE_LOG(
	   LogTemp,
	   Log,
	   TEXT("%s hit %s for %.1f damage"),
	   *Character->GetName(),
	   *HitActor->GetName(),
	   WeaponDamage
   );

	
	
}

void UIP1WeaponComponent::Reload()
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}
	
	// Check if we can reload
	if(bCanReload)
	{

		// Set CanFire
		bCanFire = false;
		
		// Reload Ammo
		CurrentAmmo = MaxAmmo;

		if (ReloadSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, Character->GetActorLocation());
		}

		UE_LOG(LogTemp, Warning, TEXT("Reloaded to Max"));

		// ToDo: Play Reload Animation Montage

		// Set a timer to call ResetFireDelay after ReLoadDuration seconds.
		GetWorld()->GetTimerManager().SetTimer(
			ReloadTimerHandle,
			this,
			&UIP1WeaponComponent::ResetFireDelay,
			ReloadDuration,
			false);
		
		// Set Can Reload
		bCanReload = false;		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Ammo Full, Can not reload"));
	}
}

void UIP1WeaponComponent::AimDownSights(const FInputActionInstance& Instance)
{
	// Get the trigger event from the instance
	ETriggerEvent CurrentEvent = Instance.GetTriggerEvent();

	if(CurrentEvent == ETriggerEvent::Triggered)
	{
		bIsAiming = true;
		bCanSteadyAim = true;

		// Set ZoomedFOV for ADS
		StartZoom(ADSZoomedFOV);
		UpdateZoom();
	}
	else if (CurrentEvent == ETriggerEvent::Completed)
	{
		bIsAiming = false;
		bCanSteadyAim = false;

		EndZoom();
		UpdateZoom();
	}
}

void UIP1WeaponComponent::SteadyAim(const FInputActionInstance& Instance)
{
	Character = Cast<AIP1Character>(GetOwner());

	// Get the trigger event from the instance
	ETriggerEvent CurrentEvent = Instance.GetTriggerEvent();

	if(!bCanSteadyAim) return;
	
	if(CurrentEvent == ETriggerEvent::Triggered)
	{
		bIsSteadyAiming = true;

		//PlayerCamera->SetFieldOfView(ZoomedFOV);
		StartZoom(SAZoomedFOV);
		UpdateZoom();
	}
	else if (CurrentEvent == ETriggerEvent::Completed)
	{
		bIsSteadyAiming = false;

		//PlayerCamera->SetFieldOfView(DefaultFOV);
		EndZoom();
		UpdateZoom();
	}
}

void UIP1WeaponComponent::StartZoom(float ZoomedFOV)
{
	if (!PlayerCamera) return;

	TargetFOV = ZoomedFOV;

	// Start looping the timer. DeltaTime is replaced by World's delta time.
	// 0.016s is roughly 60Hz, providing a smooth transition.
	GetWorld()->GetTimerManager().SetTimer(ZoomTimerHandle, this, &UIP1WeaponComponent::UpdateZoom, 0.016f, true);
}

void UIP1WeaponComponent::EndZoom()
{
	if (!PlayerCamera) return;

	TargetFOV = DefaultFOV;

	// Keep looping the timer to zoom back out smoothly
	GetWorld()->GetTimerManager().SetTimer(ZoomTimerHandle, this, &UIP1WeaponComponent::UpdateZoom, 0.016f, true);
}

void UIP1WeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<ACharacter>(GetOwner());
	if(OwningCharacter)
	{
		PlayerCamera = OwningCharacter->FindComponentByClass<UCameraComponent>();

		UE_LOG(LogTemp, Log, TEXT("PlayerCamera: %p"), PlayerCamera);
	}
}

void UIP1WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Character = Cast<AIP1Character>(GetOwner());

	if (Character == nullptr)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(WeaponMappingContext);
		}
	}
}

void UIP1WeaponComponent::ResetFireDelay()
{
	// Set Can Fire back to true when the timer finishes
	bCanFire = true;

	// Clear the handle safely
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
}

void UIP1WeaponComponent::UpdateZoom()
{
	if (!PlayerCamera || !GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ZoomTimerHandle);
		return;
	}

	float CurrentFOV = PlayerCamera->FieldOfView;
	float DeltaTIme = GetWorld()->GetDeltaSeconds();

	// Smooth interpolate to the target
	float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTIme, ZoomInterSpeed);
	PlayerCamera->SetFieldOfView(NewFOV);

	// if we are close enough to the target, snap to it and KILL the timer loop
	if (FMath::IsNearlyEqual(NewFOV, TargetFOV, 0.0f))
	{
		PlayerCamera->SetFieldOfView(TargetFOV);
		GetWorld()->GetTimerManager().ClearTimer(ZoomTimerHandle);
	}
	
	
}
