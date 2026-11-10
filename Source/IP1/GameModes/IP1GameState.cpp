// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1GameState.h"
#include "IP1/Squad/IP1SquadManager.h"

#include "Net/UnrealNetwork.h"

AIP1GameState::AIP1GameState()
{
	bReplicates = true;
	
	//MatchPhase = EIP1MatchPhase::WaitingToStart;

	SquadManager = CreateDefaultSubobject<UIP1SquadManager>(TEXT("SquadManager"));
}

void AIP1GameState::BeginPlay()
{
	Super::BeginPlay();

	if (SquadManager)
	{
		SquadManager->Initialize(this);
	}
}

void AIP1GameState::SetMatchPhase(EIP1MatchPhase NewPhase)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchPhase = NewPhase;

	// RepNotify runs automatically on clients when the replicated value changes, not on the server.
	OnRep_MatchPhase();

	ForceNetUpdate();
}

void AIP1GameState::SetMatchEndServerTime(float NewEndTime)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchEndServerTime = NewEndTime;

	ForceNetUpdate();
}

float AIP1GameState::GetMatchTimeRemaining() const
{
	if (MatchEndServerTime <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, MatchEndServerTime - GetServerWorldTimeSeconds());
}

EIP1MatchPhase AIP1GameState::GetMatchPhase() const
{
	return MatchPhase;
}

FString AIP1GameState::GetMatchPhaseString() const
{
	switch (MatchPhase)
	{
	case EIP1MatchPhase::WaitingToStart:
		return TEXT("Waiting To Start");

	case EIP1MatchPhase::InProgress:
		return TEXT("In Progress");

	case EIP1MatchPhase::PostMatch:
		return TEXT("Post Match");

	default:
		return TEXT("Unknown");
	}
}

UIP1SquadManager* AIP1GameState::GetSquadManager() const
{
	return SquadManager;
}

void AIP1GameState::UpdateReplicatedSquads(const TArray<FIP1SquadData>& NewSquads)
{
	if (!HasAuthority())
	{
		return;
	}

	ReplicatedSquads = NewSquads;

	// Notify locally; RepNotify is primarily for clients.
	ForceNetUpdate();
}

void AIP1GameState::OnRep_MatchPhase()
{
	FString PhaseName;

	switch (MatchPhase)
	{
	case EIP1MatchPhase::WaitingToStart:
		PhaseName = TEXT("Waiting To Start");
		break;

	case EIP1MatchPhase::InProgress:
		PhaseName = TEXT("In Progress");
		break;
		
	case EIP1MatchPhase::PostMatch:
		PhaseName = TEXT("Post Match");
		break;

	default:
		PhaseName = TEXT("Unknown");
	}
	
	UE_LOG(LogTemp,
		Log,
		TEXT("MATCH PHASE | %s | Time Remaining: %.1f"),
		*PhaseName,
		GetMatchTimeRemaining());

	// Later: notify the HUD or update match widgets here. 
}

void AIP1GameState::OnRep_ReplicatedSquads()
{
	// ToDo:
	// notify squad HUD, deployment screen,
	// scoreboard, or other client-side UI
}

void AIP1GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIP1GameState, MatchPhase);
	DOREPLIFETIME(AIP1GameState, MatchEndServerTime);
	DOREPLIFETIME(AIP1GameState, ReplicatedSquads);
}
