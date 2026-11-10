// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1Character.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "IP1/Weapons/IP1WeaponComponent.h"
#include "IP1/Character/IP1HealthComponent.h"
#include "IP1/GameModes/IP1GameMode.h"
#include "IP1/Player/IP1PlayerState.h"
#include "IP1/UI/IP1TeamMarkerWidget.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

// Sets default values
AIP1Character::AIP1Character()
{
	// Tell the engine this actor should replicate data
	bReplicates = true;

	// Tell the engine to replicate the movement (Transform, Velocity)
	SetReplicateMovement(true);

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent-> bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// Create Health Component
	HealthComponent = CreateDefaultSubobject<UIP1HealthComponent>(TEXT("HealthComponent"));
	
	// Create Weapon Component
	WeaponComponent = CreateDefaultSubobject<UIP1WeaponComponent>(TEXT("WeaponComponent"));
	//WeaponComponent->SetupAttachment(FirstPersonCameraComponent);

	// UI Widget - TeamMaker
	TeamMarkerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TeamMarkerWidget"));

	TeamMarkerWidget->SetupAttachment(GetRootComponent());

	TeamMarkerWidget->SetWidgetSpace(EWidgetSpace::Screen);
	TeamMarkerWidget->SetDrawSize(FVector2D(32.0f, 32.0f));
	TeamMarkerWidget->SetDrawAtDesiredSize(false);
	TeamMarkerWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));

	TeamMarkerWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AIP1Character::OnRep_IsDead()
{
	if (bIsDead)
	{
		HandleDeath();
	}
	else
	{
		HandleRespawn();
	}
}

bool AIP1Character::IsDead() const
{
	return bIsDead;
}

void AIP1Character::Die()
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	AIP1PlayerState* PS = GetPlayerState<AIP1PlayerState>();

	if (PS)
	{
		PS->AddDeath();
	}

	SetDeathState(true);

	AIP1GameMode* GM = GetWorld()->GetAuthGameMode<AIP1GameMode>();

	if (GM)
	{
		GM->ScheduleRespawn(GetController());
	}
}

void AIP1Character::HandleDeath()
{
	// Stop the character from moving.
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// Stop the capsule from blocking movement and weapon traces.
	GetCapsuleComponent()->SetCollisionEnabled(
		ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	// Stop actor-based gameplay input on the owning player.
	DisableInput(nullptr);

	// ToDo: Play death animation or ragdoll here.
	// 3. Configure Mesh collision and turn on Ragdoll physics
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		// Use the built-in Ragdoll preset 
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
        
		// Turn on physics simulation for the mesh bones
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetAllBodiesSimulatePhysics(true);
        
		// Ensure character skeletal bones wake up to fall naturally
		MeshComp->WakeAllRigidBodies();
	}
}

void AIP1Character::HandleRespawn()
{
	if (!HasAuthority())
	{
		return;
	}

	// Restore movement.
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	//Restore capsule collision.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	// ToDo: Reset health to maximum.
	// ToDo: Restore weapon state and ammunition if required.
}

void AIP1Character::SetDeathState(bool bNewIsDead)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsDead = bNewIsDead;

	// Apply behavior immediately on the server.
	OnRep_IsDead();

	// Ensure client receives the updated state promptly.
	ForceNetUpdate();
}

// Called when the game starts or when spawned
void AIP1Character::BeginPlay()
{
	UE_LOG(LogTemp,
	Warning,
	TEXT("CHARACTER BEGIN PLAY | %s | Authority: %s | Dead: %s"),
	*GetName(),
	HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"),
	bIsDead ? TEXT("TRUE") : TEXT("FALSE"));

	Super::BeginPlay();

	RefreshTeamMarkerBinding();

	UIP1HealthComponent* HC = FindComponentByClass<UIP1HealthComponent>();
	
	if (HC && HasAuthority())
	{
		HC->ResetHealth();
		
		HC->OnDeath.AddDynamic(this, &AIP1Character::Die);
	}
}

void AIP1Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AIP1Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// Called to bind functionality to input
void AIP1Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Setup action bindings
	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIP1Character::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIP1Character::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// Equip Weapon Component * Backup initialization for clients
	if (WeaponComponent && GetController())
	{
		WeaponComponent->EquipWeapon(this);
	}
	
}

UIP1HealthComponent* AIP1Character::GetHealthComponent() const
{
	return HealthComponent;
}

void AIP1Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIP1Character, bIsDead);
}

void AIP1Character::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	UE_LOG(LogTemp, Warning,
		TEXT("ON REP PLAYER STATE | Character: %s | PlayerState: %s"),
		*GetName(),
		*GetNameSafe(GetPlayerState()));

	RefreshTeamMarkerBinding();
}

void AIP1Character::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundPlayerState && TeamChangedHandle.IsValid())
	{
		BoundPlayerState->OnTeamChanged.Remove(TeamChangedHandle);
	}

	BoundPlayerState = nullptr;
	TeamChangedHandle.Reset();
	
	Super::EndPlay(EndPlayReason);
}

void AIP1Character::RefreshTeamMarkerBinding()
{
	AIP1PlayerState* CurrentPlayerState = GetPlayerState<AIP1PlayerState>();

	UE_LOG(LogTemp, Warning,
		TEXT("REFRESH TEAM MARKER | Character: %s | PlayerState: %s"),
		*GetName(),
		*GetNameSafe(CurrentPlayerState));

	if (!CurrentPlayerState)
	{
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("TEAM MARKER TEAM | Character: %s | Team: %d"),
		*GetName(),
		static_cast<int32>(CurrentPlayerState->GetTeam()));
	
	// Avoid binding to the same PlayerState more than once.
	if (BoundPlayerState != CurrentPlayerState)
	{
		if (BoundPlayerState && TeamChangedHandle.IsValid())
		{
			BoundPlayerState->OnTeamChanged.Remove(TeamChangedHandle);
		}
	

		BoundPlayerState = CurrentPlayerState;

		TeamChangedHandle = BoundPlayerState->OnTeamChanged.AddUObject(
			this,
			&AIP1Character::UpdateTeamMarker);
	}

	// Always apply the current value, even if the event
	// fired before the character finished initializing.
	UpdateTeamMarker(BoundPlayerState->GetTeam());
}

void AIP1Character::UpdateTeamMarker(EIP1Team NewTeam)
{
	UE_LOG(LogTemp, Warning,
	TEXT("TEAM MARKER UPDATE | Character: %s | Team: %d | Widget: %s"),
	*GetName(),
	static_cast<int32>(NewTeam),
	*GetNameSafe(TeamMarkerWidget->GetUserWidgetObject()));

	if (!TeamMarkerWidget)
	{
		return;
	}

	UIP1TeamMarkerWidget* Marker = Cast<UIP1TeamMarkerWidget>(TeamMarkerWidget->GetUserWidgetObject());

	if (Marker)
	{
		Marker->SetTeam(NewTeam);
	}
}

