// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1PlayerState.h"
#include "Net/UnrealNetwork.h"

AIP1PlayerState::AIP1PlayerState()
{
	Team = EIP1Team::None;
	Kills = 0;
	Deaths = 0;

	// Enable replication of this PlayerState
	bReplicates = true;
}

void AIP1PlayerState::BeginPlay()
{
	Super::BeginPlay();

	// This is just for testing and should be removed once the squad manager is setup.
	if (HasAuthority())
	{
		SetSquadId(1);
		SetIsSquadLeader(true);
	}

	UE_LOG(
	LogTemp,
	Log,
	TEXT("Player SquadID: %d | Is Leader: %s"),
	GetSquadId(),
	IsSquadLeader() ? TEXT("true") : TEXT("false"));
}

void AIP1PlayerState::SetTeam(EIP1Team NewTeam)
{
	// Only the server should assign teams.
	if (!HasAuthority())
	{
		return;
	}

	if(Team == NewTeam)
	{
		return;
	}

	Team = NewTeam;

	UE_LOG(LogTemp, Log,
		TEXT("SetTeam: %s"),
		*UEnum::GetValueAsString(Team));
	
	// The server doesn't automatically execute OnRep_Team
	// when it changes the value itself.
	OnRep_Team();
}

void AIP1PlayerState::SetSelectedRole(EIP1PlayerRole NewSelectedRole)
{
	if (!HasActiveCameraComponent())
	{
		return;
	}
	
	if (SelectedRole == NewSelectedRole)
	{
		return;
	}
	
	SelectedRole = NewSelectedRole;
	
	// Server-side notification, if needed. 
		
}

void AIP1PlayerState::OnRep_SelectedRole()
{
	// Notify HUD / role selection UI.
	
	// One detail worth noting: OnRep_SelectedRole() is a client-side replication notification. 
	// If you want identical notification behavior on the server, 
	// call a shared handler explicitly from SetSelectedRole() as well.
}

int32 AIP1PlayerState::GetSquadId() const
{
	return SquadId;
}

void AIP1PlayerState::SetSquadId(int32 NewSquadId)
{
	if (!HasAuthority())
	{
		return;
	}

	SquadId = NewSquadId;
	
	// RepNotify does not automatically run on the server.
	OnRep_SquadId();
	
	ForceNetUpdate();
}

bool AIP1PlayerState::IsSquadLeader() const
{
	return bIsSquadLeader;
}

void AIP1PlayerState::SetIsSquadLeader(bool bNewIsSquadLeader)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsSquadLeader = bNewIsSquadLeader;
}

void AIP1PlayerState::AddKill()
{
	if (!HasAuthority())
	{
		return;
	}

	++Kills;

	// APlayerState::Score is a float
	SetScore(GetScore() + 100.f);
}

void AIP1PlayerState::AddDeath()
{
	if (!HasAuthority())
	{
		return;
	}

	++Deaths;
}

void AIP1PlayerState::OnRep_Team()
{
	UE_LOG(
	  LogTemp,
	  Warning,
	  TEXT("TEAM REPLICATION | PlayerState: %s | Team: %d | Pawn: %s"),
	  *GetName(),
	  static_cast<int32>(Team),
	  *GetNameSafe(GetPawn())
  );
	
	OnTeamChanged.Broadcast(Team);
	
	// Later, you can notify UI or other systems when
	// this player's team changes.

	// The replicated Team value has changed on this client.
	// Notify the overhead marker through the owning character.
	if (APawn* MyPawn = GetPawn())
	{
		
	}
}

void AIP1PlayerState::OnRep_SquadId()
{
	// ToDo: Notify squad UI when membership changes.
}

void AIP1PlayerState::OnRep_IsSquadLeader()
{
	// ToDo: Notifty squad UI when leadership changes. 
}

void AIP1PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIP1PlayerState, Team);

	DOREPLIFETIME(AIP1PlayerState, SquadId);

	DOREPLIFETIME(AIP1PlayerState, bIsSquadLeader);
	
	DOREPLIFETIME(AIP1PlayerState, SelectedRole);
}

