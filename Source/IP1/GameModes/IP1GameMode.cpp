// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1GameMode.h"
#include "IP1GameState.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "IP1/Player/IP1PlayerController.h"
#include "IP1/Character/IP1Character.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/GameStateBase.h"
#include "IP1/Player/IP1PlayerStart.h"
#include "IP1/Player/IP1PlayerState.h"
#include "IP1/Squad/IP1SquadManager.h"
#include "Kismet/GameplayStatics.h"


AIP1GameMode::AIP1GameMode()
{
	GameStateClass = AIP1GameState::StaticClass();

	// Multiplayer defaults.
	MaxPlayersPerTeam = 5;
	MaxPlayers = 10;

	// Framework classes.
	GameStateClass = AIP1GameState::StaticClass(); // *REQUIRES IP1GameState Class
	PlayerStateClass = AIP1PlayerState::StaticClass(); // *REQUIRES IP1PlayerState Class
	
	bStartPlayersAsSpectators = true;

	// PlayerController Class. **Calling BP Class for Prototyping
	
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Blueprints/Player/BP_IP1PlayerController"));

	if (PlayerControllerBPClass.Class != nullptr)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class; 
	}
	
	// Default player character. **Calling BP Class for Prototyping
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Character/BP_IP1Character"));

	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	} 
}

void AIP1GameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("GAMEMODE BEGIN PLAY: BEGIN WARMUP"));
	BeginWarmup();
}

void AIP1GameMode::StartPlay()
{
	Super::StartPlay();

	UE_LOG(LogTemp, Log, TEXT("IP1 GameMode Started"));
	UE_LOG(LogTemp, Log, TEXT("Maximum Players: %d"), MaxPlayers);
	UE_LOG(LogTemp, Log, TEXT("Maxium Players Per Team: %d"), MaxPlayersPerTeam);

	UE_LOG(LogTemp, Warning, TEXT("WW2GameMode StartPlay - DefaultPawnClass: %s"), *GetNameSafe(DefaultPawnClass));
}

void AIP1GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (!HasAuthority() || !IsValid(NewPlayer))
	{
		return;
	}
	
	AIP1PlayerController* PC =
		Cast<AIP1PlayerController>(NewPlayer);

	AIP1PlayerState* PS =
		NewPlayer->GetPlayerState<AIP1PlayerState>();

	if (!PS)
	{
		UE_LOG(LogTemp, Error,
			TEXT("PostLogin: IP1PlayerState missing."));

		return;
	}

	// Initialize team.
	PS->SetTeam(EIP1Team::None);

	UE_LOG(LogTemp, Warning,
		TEXT("POST LOGIN: %s | Team: None"),
		*PS->GetPlayerName());
	
	// Display team selection UI.
	PC->Client_ShowTeamSelection();
}

void AIP1GameMode::Logout(AController* Exiting)
{
	if(HasAuthority() && Exiting)
	{
		AIP1PlayerState* IP1State = Exiting->GetPlayerState<AIP1PlayerState>();

		if(IP1State)
		{
			RemovePlayerFromTeam(IP1State);
		}
	}
	
	Super::Logout(Exiting); 

	UpdateTeamCounts();

	UE_LOG(LogTemp,Log, TEXT("PLayer left. Allies: %d | Axis: %d"), AlliedPlayerCount, AxisPlayerCount);
}

void AIP1GameMode::StartMatch()
{
	if (!HasAuthority())
	{
		return;
	}

	AIP1GameState* GS = GetGameState<AIP1GameState>();

	if (!GS)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MatchPhaseTimerHandle);

	// Update Unreal's native match state.
	Super::StartMatch();

	// Update our custom replicated match state.
	GS->SetMatchPhase(EIP1MatchPhase::InProgress);

	GS->SetMatchEndServerTime(GS->GetServerWorldTimeSeconds() + MatchDuration);

	UE_LOG(LogTemp,
		Log,
		TEXT("MATCH: Started. Duration: %.1f"), MatchDuration);

	if (MatchDuration <= 0.0f)
	{
		EndMatch();
		return;
	}

	GetWorldTimerManager().SetTimer(
		MatchPhaseTimerHandle,
		this,
		&AIP1GameMode::EndMatch,
		MatchDuration,
		false);
}

void AIP1GameMode::EndMatch()
{
	if (!HasAuthority())
	{
		return;
	}

	AIP1GameState* GS = GetGameState<AIP1GameState>();

	if(!GS)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MatchPhaseTimerHandle);

	// Update Unreal's native match state.
	Super::EndMatch();

	// Update our custom replicated match state.
	GS->SetMatchPhase(EIP1MatchPhase::PostMatch);

	BeginPostMatch();

	UE_LOG(LogTemp,
		Log,
		TEXT("MATCH: Ended."))
}

void AIP1GameMode::BeginWarmup()
{
	AIP1GameState* GS = GetGameState<AIP1GameState>();

	if (!GS)
	{
		UE_LOG(LogTemp,
			Error,
			TEXT("MATCH ERROR: IP1GameState not found."));

		return;
	}

	GS->SetMatchPhase(EIP1MatchPhase::WaitingToStart);

	GS->SetMatchEndServerTime(GS->GetServerWorldTimeSeconds() + WarmupDuration);

	UE_LOG(LogTemp,
		Error,
		TEXT("MATCH: Warmup started. Duration: %.1f"), WarmupDuration);

	if (WarmupDuration <= 0.0f)
	{
		StartMatch();
		return;
	}

	GetWorldTimerManager().SetTimer(
		MatchPhaseTimerHandle,
		this,
		&AIP1GameMode::StartMatch,
		WarmupDuration,
		false);
}

void AIP1GameMode::BeginPostMatch()
{
	AIP1GameState* GS = GetGameState<AIP1GameState>();

	if(!GS)
	{
		return;
	}

	GS->SetMatchEndServerTime(
		GS->GetServerWorldTimeSeconds() + PostMatchDuration);

	UE_LOG(LogTemp,
		Log,
		TEXT("MATCH: Post-match phase started."));

	if (PostMatchDuration <= 0.0f)
	{
		HandlePostMatchFinished();
		return;
	}

	GetWorldTimerManager().SetTimer(
		MatchPhaseTimerHandle,
		this,
		&AIP1GameMode::HandlePostMatchFinished,
		PostMatchDuration,
		false);
}

void AIP1GameMode::HandlePostMatchFinished()
{
	GetWorldTimerManager().ClearTimer(
		MatchPhaseTimerHandle);

	UE_LOG(LogTemp,
		Log,
		TEXT("MATCH: Post-match peroid finished."));

	// Prototype 03: remain in PostMatch
	// Later: restart the round or return to a lobby
}

AActor* AIP1GameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!IsValid(Player))
	{
		UE_LOG(LogTemp, Error, TEXT("ChoosePlayerStart: Player controller is invalid."));
		return nullptr;
	}

	AIP1PlayerState* PS = Player->GetPlayerState<AIP1PlayerState>();

	if (!IsValid(PS))
	{
		UE_LOG(LogTemp, Error, TEXT("ChoosePlayerStart: PlayerState is invalid."));
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	const EIP1Team Team = PS->GetTeam();

	UE_LOG(LogTemp, Warning,
		TEXT("SPAWN DEBUG: Controller=%s PlayerState=%s Team=%d"),
		*GetNameSafe(Player),
		*GetNameSafe(PS),
		static_cast<uint8>(Team));

	if (Team == EIP1Team::None)
	{
		UE_LOG(LogTemp, Error,
			TEXT("SPAWN DEBUG: Team is None; this player should not be spawning yet."));
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	int32 CustomStartCount = 0;

	for (TActorIterator<AIP1PlayerStart> It(GetWorld()); It; ++It)
	{
		AIP1PlayerStart* Start = *It;

		if (!IsValid(Start))
		{
			continue;
		}

		++CustomStartCount;

		UE_LOG(LogTemp, Warning,
			TEXT("SPAWN DEBUG: Start=%s Team=%d Location=%s"),
			*GetNameSafe(Start),
			static_cast<uint8>(Start->Team),
			*Start->GetActorLocation().ToString());

		if (Start->Team == Team)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("SPAWN DEBUG: MATCH FOUND: %s"),
				*GetNameSafe(Start));

			return Start;
		}
	}

	UE_LOG(LogTemp, Error,
		TEXT("SPAWN DEBUG: No matching team start. Custom AIP1PlayerStart count=%d"),
		CustomStartCount);
	
	return nullptr;
}

bool AIP1GameMode::CanJoinTeam(EIP1Team Team) const
{
	switch(Team)
	{
	case EIP1Team::Allies:
		return AlliedPlayerCount < MaxPlayersPerTeam;

	case EIP1Team::Axis:
		return AxisPlayerCount < MaxPlayersPerTeam;

	default:
		return false;
	}
}

void AIP1GameMode::AssignPlayerTeam(AIP1PlayerState* PlayerState, EIP1Team Team)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!PlayerState)
	{
		return;
	}

	// Make sure the request team is valid.
	if (!CanJoinTeam(Team))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot add player %s to %d: team is full or invalid"), *PlayerState->GetPlayerName(), static_cast<uint8>(Team));

		return;
	}

	
	// If the player is already assigned to this team, there is nothing to do.
	if (PlayerState->Team == Team)
	{
		return;
	}

	// If the player already belongs to another team, remove them from that team first.
	if (PlayerState->Team != EIP1Team::None)
	{
		RemovePlayerFromTeam(PlayerState);
	}

	PlayerState->SetTeam(Team);

	UE_LOG(LogTemp, Warning, TEXT("AddPlayerToTeam(): UpdateTeamCounts()."))
	UpdateTeamCounts();

	UE_LOG(LogTemp, Log, TEXT("AddPlayerToTeam: Player %s assigned to team %d"), *PlayerState->GetPlayerName(), static_cast<uint8>(Team));
}


void AIP1GameMode::RemovePlayerFromTeam(AIP1PlayerState* PlayerState)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!PlayerState)
	{
		return;
	}

	if(PlayerState->Team == EIP1Team::None)
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Removing player %s from team %d."), *PlayerState->GetPlayerName(), static_cast<uint8>(PlayerState->Team));

	PlayerState->SetTeam(EIP1Team::None);

	UpdateTeamCounts();
}

void AIP1GameMode::UpdateTeamCounts()
{
	if(!GameState)
	{
		return;
	}

	AlliedPlayerCount = 0;
	AxisPlayerCount = 0;

	for(APlayerState* PlayerState : GameState->PlayerArray)
	{
		const AIP1PlayerState* IP1PlayerState = Cast<AIP1PlayerState>(PlayerState);

		if(!IP1PlayerState)
		{
			continue;
		}

		switch (IP1PlayerState->Team)
		{
		case EIP1Team::Allies:
			AlliedPlayerCount++;
			break;

		case EIP1Team::Axis:
			AxisPlayerCount++;
			break;

		case EIP1Team::None:
		default:
			break;
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("UpdateTeamCounts(): Team Counts Update - Allies: %d | Axis: %d"), AlliedPlayerCount, AxisPlayerCount);
}

void AIP1GameMode::HandleTeamSelection(AIP1PlayerController* PlayerController, EIP1Team RequestedTeamID)
{
	if (!HasAuthority() || !PlayerController)
	{
		return;
	}

	AIP1PlayerState* PS =
		PlayerController->GetPlayerState<AIP1PlayerState>();

	if (!PS)
	{
		return;
	}

	// Prevent changing teams through repeated selection.
	if (PS->GetTeam() != EIP1Team::None)
	{
		return;
	}

	if (RequestedTeamID != EIP1Team::Allies &&
		RequestedTeamID != EIP1Team::Axis)
	{
		return;
	}

	// Refresh team counts before checking capacity.
	UpdateTeamCounts();

	if (!CanJoinTeam(RequestedTeamID))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Team selection rejected: Team is full."));

		return;
	}

	// Assign the requested team.
	PS->SetTeam(RequestedTeamID);

	UpdateTeamCounts();

	UE_LOG(LogTemp, Warning,
		TEXT("TEAM SELECTION ACCEPTED: %s | Team: %s"),
		*PS->GetPlayerName(),
		*UEnum::GetValueAsString(RequestedTeamID));
	
	// Spawn the player now that they have a valid team.
	RestartPlayer(PlayerController);

	// Notify the owning client that team selection is complete.
	// Close selection UI. 
	PlayerController->Client_TeamSelectionComplete();
}

void AIP1GameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	UE_LOG(LogTemp, Warning,
		TEXT("HandleStartingNewPlayer called."));
	
	// Intentionally do not call Super.
	// Players must select a team and role before a separate spawn request.
}

void AIP1GameMode::RestartPlayer(AController* NewPlayer)
{
	if (!IsValid(NewPlayer))
	{
		UE_LOG(LogTemp, Error,
			TEXT("RestartPlayer: Controller is invalid."));
		return;
	}

	AIP1PlayerState* PS =
		NewPlayer->GetPlayerState<AIP1PlayerState>();

	if (!IsValid(PS))
	{
		UE_LOG(LogTemp, Error,
			TEXT("RestartPlayer BLOCKED: PlayerState is invalid."));
		return;
	}
	const EIP1Team Team = PS->GetTeam();

	if (Team == EIP1Team::None)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("RestartPlayer BLOCKED: %s has not selected a team."),
			*PS->GetPlayerName());
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("RestartPlayer: Calling Super | Controller=%s | Team=%d | ExistingPawn=%s"),
		*GetNameSafe(NewPlayer),
		static_cast<uint8>(Team),
		*GetNameSafe(NewPlayer->GetPawn()));

	Super::RestartPlayer(NewPlayer);

	UE_LOG(LogTemp, Warning,
		TEXT("RestartPlayer: After Super | Controller=%s | Pawn=%s"),
		*GetNameSafe(NewPlayer),
		*GetNameSafe(NewPlayer->GetPawn()));
}

void AIP1GameMode::HandleRoleSelection(AIP1PlayerController* PlayerController, EIP1PlayerRole RequestedRole)
{
	if (!HasAuthority() || !PlayerController)
	{
		return;
	}

	AIP1PlayerState* PS =
		PlayerController->GetPlayerState<AIP1PlayerState>();

	if (!PS)
	{
		return;
	}

	AIP1GameState* GS =
		GetGameState<AIP1GameState>();

	if (!GS)
	{
		return;
	}

	UIP1SquadManager* SquadManager =
		GS->GetSquadManager();

	if (!SquadManager)
	{
		return;
	}

	// Get the player's current Squad ID.
	const int32 SquadID = PS->GetSquadId();

	// Validate the requested role within their squad.
	if (!SquadManager->CanSelectRole(
		SquadID,
		RequestedRole,
		PS))
	{
		PlayerController->ClientRoleSelectionRejected();
		return;
	}

	// Role selection was successful.
	PS->SetSelectedRole(RequestedRole);
}

int32 AIP1GameMode::GetTeamPlayerCount(EIP1Team Team) const
{
	switch(Team)
	{
	case EIP1Team::Allies:
		return AlliedPlayerCount;

	case EIP1Team::Axis:
		return AxisPlayerCount;

	default:
		return 0;
	}
}

void AIP1GameMode::ScheduleRespawn(AController* Controller)
{
	if (!HasAuthority() || !Controller)
	{
		return;
	}

	FTimerHandle LocalRespawnTimer;

	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this,
		&AIP1GameMode::RespawnPlayer,
		Controller);

	GetWorldTimerManager().SetTimer(LocalRespawnTimer, RespawnDelegate,
		RespawnDelay, false);
}

void AIP1GameMode::RespawnPlayer(AController* Controller)
{
	if (!HasAuthority() || !IsValid(Controller))
	{
		return;
	}

	// 1. Remove the dead pawn.
	APawn* OldPawn = Controller->GetPawn();

	UE_LOG(LogTemp,
		Warning,
		TEXT("RESPAWN BEFORE | Controller: %s | Pawn: %s | Address: %p"),
		*GetNameSafe(Controller),
		*GetNameSafe(OldPawn),
		OldPawn);

	if (OldPawn)
	{
		UE_LOG(LogTemp,
			Warning,
			TEXT("RESPAWN: Destroying old pawn %s"),
			*GetNameSafe(OldPawn));

		Controller->UnPossess();
		OldPawn->Destroy();
	}

	// 2. Spawn and possess a fresh pawn.
	RestartPlayer(Controller);

	// 3. Verify the result.
	APawn* NewPawn = Controller->GetPawn();

	UE_LOG(LogTemp,
		Warning,
		TEXT("RESPAWN AFTER | Controller: %s | Pawn: %s | Address: %p"),
		*GetNameSafe(Controller),
		*GetNameSafe(NewPawn),
		NewPawn);

	if (OldPawn == NewPawn)
	{
		UE_LOG(LogTemp,
			Error,
			TEXT("RESPAWN WARNING: Controller still possesses the same pawn."));
	}

	if (!NewPawn)
	{
		UE_LOG(LogTemp,
			Error,
			TEXT("RESPAWN FAILED: No pawn was spawned for %s"),
			*GetNameSafe(Controller));

		return;
	}
	
	UCapsuleComponent* Capsule =
	   NewPawn->FindComponentByClass<UCapsuleComponent>();

	if (Capsule)
	{
		UE_LOG(LogTemp,
			Warning,
			TEXT("RESPAWN CAPSULE | Collision: %d | Profile: %s"),
			static_cast<int32>(Capsule->GetCollisionEnabled()),
			*Capsule->GetCollisionProfileName().ToString());
	}

	AIP1Character* Character = Cast<AIP1Character>(NewPawn);

	if (Character)
	{
		UE_LOG(LogTemp,
			Warning,
			TEXT("RESPAWN STATE | IsDead: %s | Location: %s"),
			Character->IsDead() ? TEXT("TRUE") : TEXT("FALSE"),
			*Character->GetActorLocation().ToString());
	}
}


