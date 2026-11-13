// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "IP1/UI/IP1DeploymentWidget.h"
#include "IP1/GameModes/IP1GameMode.h"
#include "IP1PlayerState.h"

AIP1PlayerController::AIP1PlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget>
	HUDWidgetBPClass(TEXT("/Game/UI/WBP_IP1HUD"));
	
	if (HUDWidgetBPClass.Succeeded())
	{
		HUDWidgetClass = HUDWidgetBPClass.Class;
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Failed to find WBP_IP1HUD!")
		);
	}
	
	static ConstructorHelpers::FClassFinder<UUserWidget>
	TeamSelectionWidgetBPClass(TEXT("/Game/UI/WBP_IP1TeamSelection"));
	
	if (TeamSelectionWidgetBPClass.Succeeded())
	{
		TeamSelectionWidgetClass = TeamSelectionWidgetBPClass.Class;
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Failed to find WBP_IP1TeamSelection!")
		);
	}
}

void AIP1PlayerController::Client_ShowTeamSelection_Implementation()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("ShowTeamSelection. Showing UI.")
	);
	
	ShowTeamSelection();
}


void AIP1PlayerController::Server_RequestTeamSelection_Implementation(EIP1Team TeamID)
{
	AIP1GameMode* GM = GetWorld()->GetAuthGameMode<AIP1GameMode>();
	
	if (!GM)
	{
		return;
	}
	
	GM->HandleTeamSelection(this, TeamID);
}

void AIP1PlayerController::Client_TeamSelectionComplete_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Team selection completed. Closing UI."));

	// Close the existing team selection UI.
	HideTeamSelection();
	
	// Open the deployment UI.
	ShowDeploymentScreen();
}

void AIP1PlayerController::Client_ShowDeploymentScreen_Implementation()
{
	ShowDeploymentScreen();
}

void AIP1PlayerController::SelectAllies()
{
	Server_RequestTeamSelection(EIP1Team::Allies);
}

void AIP1PlayerController::SelectAxis()
{
	Server_RequestTeamSelection(EIP1Team::Axis);
}

void AIP1PlayerController::RequestRoleSelection(EIP1PlayerRole RequestedRole)
{
	if (!IsLocalController())
	{
		return;
	}

	ServerRequestRoleSelection(RequestedRole);
}

void AIP1PlayerController::ServerRequestRoleSelection_Implementation(EIP1PlayerRole RequestedRole)
{
	AIP1GameMode* GM = GetWorld()->GetAuthGameMode<AIP1GameMode>();

	if (!GM)
	{
		return;
	}

	GM->HandleRoleSelection(this, RequestedRole);
}

void AIP1PlayerController::ClientRoleSelectionRejected_Implementation()
{
	// Show an error in the selection UI.
}

void AIP1PlayerController::BeginPlay()
{
	Super::BeginPlay();

	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}

	if (!IsLocalController())
	{
		return;
	}
	
	if (!HUDWidgetClass)
	{
		UE_LOG(LogTemp,
			Warning,
			TEXT("HUDWidgetClass is not assigned."));
		return;
	}

	HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);

	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
		
		HUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AIP1PlayerController::ShowTeamSelection()
{
	if (!IsLocalController())
	{
		return;
	}
	
	// Hide the HUD while team selection is active.
	if (HUDWidget)
	{
		HUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (!TeamSelectionWidgetClass)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TeamSelectionWidgetClass is not assigned."));
		
		return;
	}
	
	if (!TeamSelectionWidget)
	{
		TeamSelectionWidget = CreateWidget<UUserWidget>(this, TeamSelectionWidgetClass);
	}
	
	if (TeamSelectionWidget)
	{
		TeamSelectionWidget->AddToViewport();
		
		bShowMouseCursor = true;
		
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TeamSelectionWidget->TakeWidget());
		
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		
		SetInputMode(InputMode);
	}
}

void AIP1PlayerController::HideTeamSelection()
{
	if (!IsLocalController())
	{
		return;
	}
	
	// Remove team selection widget. 
	if (IsValid(TeamSelectionWidget))
	{
		TeamSelectionWidget->RemoveFromParent();
		TeamSelectionWidget = nullptr;
	}
	
	// Show the HUD again after team selection.
	if (HUDWidget)
	{
		HUDWidget->SetVisibility(ESlateVisibility::Visible);
	}
	
	// Restore gameplay input.
	bShowMouseCursor = false;
	
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Team selection closed. Game input restored.")
	);
}

void AIP1PlayerController::ShowDeploymentScreen()
{
	// UI should only be created on the owning client.
	if (!IsLocalController())
	{
		return;
	}
	
	// Hide the gameplay HUD
	if (IsValid(HUDWidget))
	{
		HUDWidget->SetVisibility(
			ESlateVisibility::Collapsed
		);
	}

	if (!DeploymentWidgetClass)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("DeploymentWidgetClass is not assigned!"));
		return;
	}

	// Create the widget only once.
	if (!IsValid(DeploymentWidget))
	{
		DeploymentWidget =
			CreateWidget<UIP1DeploymentWidget>(
				this,
				DeploymentWidgetClass
			);
	}

	if (!IsValid(DeploymentWidget))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Failed to create deployment widget."));
		return;
	}

	// Prevent adding duplicate widgets.
	if (!DeploymentWidget->IsInViewport())
	{
		DeploymentWidget->AddToViewport(100);
	}

	DeploymentWidget->SetVisibility(
		ESlateVisibility::Visible
	);

	DeploymentWidget->RefreshDeploymentUI();

	// Allow mouse interaction with the UI.
	FInputModeUIOnly InputMode;

	InputMode.SetWidgetToFocus(
		DeploymentWidget->TakeWidget()
	);

	InputMode.SetLockMouseToViewportBehavior(
		EMouseLockMode::DoNotLock
	);

	SetInputMode(InputMode);

	bShowMouseCursor = true;

	UE_LOG(LogTemp, Log,
		TEXT("Deployment screen opened."));
	
	UE_LOG(LogTemp, Warning,
	TEXT("[DEPLOY] Widget=%s | InViewport=%d | Visibility=%d | Local=%d"),
	*GetNameSafe(DeploymentWidget),
	IsValid(DeploymentWidget) ? DeploymentWidget->IsInViewport() : 0,
	IsValid(DeploymentWidget) ? static_cast<int32>(DeploymentWidget->GetVisibility()) : -1,
	IsLocalController());
	
	UE_LOG(LogTemp, Warning,
	TEXT("[DEPLOY] WidgetClass=%s"),
	*GetNameSafe(DeploymentWidgetClass.Get()));
}

void AIP1PlayerController::HideDeploymentScreen()
{
	if (!IsLocalController())
	{
		return;
	}
	
	if (IsValid(DeploymentWidget))
	{
		DeploymentWidget->RemoveFromParent();
	}
	
	// Restore gameplay input.
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	
	bShowMouseCursor = false;
	
	UE_LOG(LogTemp, Log, TEXT("Deployment screen closed."))
	
	// Show gameplay HUD
	if (IsValid(HUDWidget))
	{
		HUDWidget->SetVisibility(
			ESlateVisibility::Visible
		);
	}
	
}

bool AIP1PlayerController::IsDeploymentScreenOpen() const
{
	return IsValid(DeploymentWidget) && DeploymentWidget->IsInViewport();
}
