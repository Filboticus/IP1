// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1SquadManager.h"

#include "IP1SquadData.h"
#include "AssetRegistry/AssetDataTagMapSerializationDetails.h"
#include "IP1/GameModes/IP1GameState.h"
#include "IP1/Player/IP1PlayerState.h"


void UIP1SquadManager::Initialize(AIP1GameState* InGameState)
{
	OwningGameState = InGameState;
}

int32 UIP1SquadManager::CreateSquad(EIP1Team TeamId, int32 MaxMembers)
{
	// The important detail is that the manager checks the authority of its owning GameState.
	// A UObject doesn't have the same actor-level authority API as an AActor,
	// so it should not try to use its own HasAuthority().

	if (!IsValid(OwningGameState) || !OwningGameState->HasAuthority())
	{
		return INDEX_NONE;
	}

	if (MaxMembers <= 0)
	{
		return INDEX_NONE;	
	}

	FIP1SquadData NewSquad;
	NewSquad.SquadId = NextSquadId++;
	NewSquad.TeamId = TeamId;
	NewSquad.MaxMembers = MaxMembers;

	const int32 NewSquadId = NewSquad.SquadId;

	Squads.Add(MoveTemp(NewSquad));

	PublishSquads();

	return NewSquadId;
}

bool UIP1SquadManager::AssignPlayerToSquad(AIP1PlayerState* PlayerState, int32 SquadId)
{
	if (!IsValid(OwningGameState) ||
	!OwningGameState->HasAuthority() ||
	!IsValid(PlayerState))
	{
		return false;
	}

	const int32 SquadIndex = FindSquadIndexById(SquadId);

	if (!Squads.IsValidIndex(SquadIndex))
	{
		return false;
	}

	// Assignment to the same squad is already satisfied.
	if (PlayerState->GetSquadId() == SquadId)
	{
		return true;
	}

	FIP1SquadData& TargetSquad = Squads[SquadIndex];

	// Do not remove a player from their current squad
	// unless the new squad can accept them.
	if (TargetSquad.Members.Num() >= TargetSquad.MaxMembers)
	{
		return false;
	}

	// Remove the previous assignment, if any.
	RemovePlayerFromSquadInternal(PlayerState);

	// Add the new assignment.
	TargetSquad.Members.Add(PlayerState);
	PlayerState->SetSquadId(SquadId);

	// First member becomes leader.
	if (!IsValid(TargetSquad.SquadLeader))
	{
		TargetSquad.SquadLeader = PlayerState;
	}

	PublishSquads();

	return true;
}

bool UIP1SquadManager::RemovePlayerFromSquad(AIP1PlayerState* PlayerState)
{
	if (!IsValid(OwningGameState) ||
		!OwningGameState->HasAuthority() ||
		!IsValid(PlayerState))
	{
		return false;
	}

	RemovePlayerFromSquadInternal(PlayerState);

	PublishSquads();

	return true;
}

bool UIP1SquadManager::GetSquadById(int32 SquadId, FIP1SquadData& OutSquad) const
{
	const int32 Index = FindSquadIndexById(SquadId);

	if (!Squads.IsValidIndex(Index))
	{
		return false;
	}

	OutSquad = Squads[Index];
	return true;
}

bool UIP1SquadManager::CanSelectRole(int32 SquadID, const EIP1PlayerRole RequestedRole, AIP1PlayerState* RequestingPlayer) const
{
	if (!IsValid(RequestingPlayer))
	{
		return false;
	}
	
	const FIP1SquadData* Squad = Squads.FindByPredicate(
		[SquadID](const FIP1SquadData& SquadData)
			{
				return SquadData.SquadId == SquadID;
			}
		);
	
	if (!Squad)
	{
		return false;
	}
	
	// Verify the player actually belongs to this squad.
	if (!Squad->Members.Contains(RequestingPlayer))
	{
		return false;
	}
	
	const int32 RoleLimit = GetRoleLimit(RequestedRole);
	
	if (RoleLimit <= 0)
	{
		return false;
	}
	
	const int32 CurrentCount = CountPlayerWithRole(SquadID, RequestingPlayer, RequestedRole);
	
	return CurrentCount < RoleLimit;
	
	// Note that Contains(RequestingPlayer) assumes a compatible member array type. 
	// If your array uses TObjectPtr<AIP1PlayerState>, 
	// and your compiler doesn't accept the raw pointer for Contains, 
	// use ContainsByPredicate to compare Member.Get() with RequestingPlayer.
}

int32 UIP1SquadManager::GetRoleLimit(EIP1PlayerRole Role) const
{
	switch (Role)
	{
		case EIP1PlayerRole::Rifleman:
			return 6;
		
		case EIP1PlayerRole::AutomaticRifleman:
			return 1;

		case EIP1PlayerRole::Medic:
			return 1;

		case EIP1PlayerRole::Support:
			return 1;

		default:
			return 0;
	}
}

int32 UIP1SquadManager::CountPlayerWithRole(int32 SquadID, AIP1PlayerState* PlayerState, EIP1PlayerRole RequestedRole,
	const AIP1PlayerState* ExcludedPlayer) const
{
		// Find the squad matching the requested ID.
	const FIP1SquadData* Squad = Squads.FindByPredicate(
			[SquadID](const FIP1SquadData& SquadData)
		{
			return SquadData.SquadId == SquadID;
		}
	);
		
	// Squad does not exist.
	if (!Squad)
	{
		return 0;
	}
	
	int32 Count = 0;
	
	// Iterate only through this squad's members.
	for (const TObjectPtr<AIP1PlayerState>& Member : Squad->Members)
	{
		// Skip invalid members.
		if (!IsValid(Member.Get()))
		{
			continue;
		}
		
		// Don't count the requesting player.
		if (Member.Get() == ExcludedPlayer)
		{
			continue;
		}
		
		// Check the member's selected role.
		if (Member->GetSelectedRole() == RequestedRole)
		{
			++Count;
		}
	}
	
	UE_LOG(
		LogTemp,
		Log,
		TEXT("SquadManager | IPSquad %d | Role %d | Count %d"),
		SquadID,
		static_cast<int32>(RequestedRole),
		Count
		);
	
	return Count;
}

int32 UIP1SquadManager::FindSquadIndexById(int32 SquadID) const
{
	return Squads.IndexOfByPredicate(
		[SquadID](const FIP1SquadData& Squad)
		{
			return Squad.SquadId == SquadID;
		});
}

void UIP1SquadManager::RemovePlayerFromSquadInternal(AIP1PlayerState* PlayerState)
{
	if (!IsValid(PlayerState))
	{
		return;
	}

	const int32 SquadIndex = FindSquadIndexById(PlayerState->GetSquadId());

	if(!Squads.IsValidIndex(SquadIndex))
	{
		PlayerState->SetSquadId(INDEX_NONE);
		return;
	}

	FIP1SquadData& Squad = Squads[SquadIndex];

	Squad.Members.Remove(PlayerState);

	if(Squad.SquadLeader == PlayerState)
	{
		Squad.SquadLeader = Squad.Members.Num() > 0
			? Squad.Members[0]
			: nullptr;
	}
}

void UIP1SquadManager::PublishSquads()
{
	if (!IsValid(OwningGameState) ||
		!OwningGameState->HasAuthority())
	{
		return;
	}

	// OwningGameState->UpdateReplicatedSquads(Squads);

	// ToDo:
	// We'll implement UpdateReplicatedSquads() in GameState next.
	// The manager's Squads array is the authoritative working data.
	// GameState receives a copy to replicate. All squad membership
	// changes should go through the manager's API rather than directly
	// editing either array.
}

