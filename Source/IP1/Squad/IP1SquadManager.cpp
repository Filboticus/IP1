// Fill out your copyright notice in the Description page of Project Settings.


#include "IP1SquadManager.h"

#include "IP1SquadData.h"
#include "UObject/UnrealType.h"
#include "AssetRegistry/AssetDataTagMapSerializationDetails.h"
#include "IP1/GameModes/IP1GameState.h"
#include "IP1/Player/IP1PlayerState.h"


void UIP1SquadManager::Initialize(AIP1GameState* InGameState)
{
	OwningGameState = InGameState;
}

bool UIP1SquadManager::CreateSquad(AIP1PlayerState* CreatingPlayer)
{
	if (!IsValid(CreatingPlayer))
	{
		return false;
	}

	const EIP1Team Team = CreatingPlayer->GetTeam();

	if (Team == EIP1Team::None)
	{
		return false;
	}

	// Find the next available squad name.
	const EIP1SquadName NewSquadName =
		FindNextAvailableSquadName(Team);

	if (NewSquadName == EIP1SquadName::None)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("CreateSquad FAILED: No available squad names."));

		return false;
	}

	// Create squad data.
	FIP1SquadData NewSquad;

	NewSquad.SquadId = NextSquadId++;
	NewSquad.TeamId = Team;
	NewSquad.SquadName = NewSquadName;
	NewSquad.SquadLeader = CreatingPlayer;
	NewSquad.Members.Add(CreatingPlayer);

	// Add squad to the manager.
	Squads.Add(NewSquad);

	// Update player membership.
	CreatingPlayer->SetSquadId(NewSquad.SquadId);

	UE_LOG(LogTemp, Log,
		TEXT("Created Squad | ID=%d | Name=%d | Team=%d"),
		NewSquad.SquadId,
		static_cast<int32>(NewSquad.SquadName),
		static_cast<int32>(Team));

	return true;
}

bool UIP1SquadManager::CreateTestSquad(EIP1Team Team)
{
	if (Team == EIP1Team::None)
	{
		return false;
	}

	const EIP1SquadName NewSquadName =
		FindNextAvailableSquadName(Team);

	if (NewSquadName == EIP1SquadName::None)
	{
		return false;
	}

	FIP1SquadData NewSquad;

	NewSquad.SquadId = NextSquadId++;
	NewSquad.TeamId = Team;
	NewSquad.SquadName = NewSquadName;

	// Test squads start empty.
	NewSquad.SquadLeader = nullptr;
	NewSquad.Members.Empty();

	Squads.Add(NewSquad);

	UE_LOG(LogTemp, Warning,
		TEXT("Created TEST Squad | ID=%d | Name=%s| Team=%s"),
		NewSquad.SquadId,
		*StaticEnum<EIP1SquadName>()->GetNameStringByValue(static_cast<int64>(NewSquad.SquadName)),
		*StaticEnum<EIP1Team>()->GetNameStringByValue(static_cast<int64>(Team)));

	return true;
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

	RefreshSquadSummaries();

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

	RefreshSquadSummaries();

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

TArray<FIP1SquadSummary> UIP1SquadManager::BuildSquadSummaries() const
{
	TArray<FIP1SquadSummary> Result;

	Result.Reserve(Squads.Num());

	for (const FIP1SquadData& Squad : Squads)
	{
		FIP1SquadSummary Summary;

		Summary.SquadId = Squad.SquadId;
		Summary.SquadName = Squad.SquadName;
		Summary.SquadType = Squad.SquadType;
		Summary.TeamId = Squad.TeamId;

		Summary.MemberCount = Squad.Members.Num();
		Summary.MaxMembers = Squad.MaxMembers;
		Summary.bIsLocked = Squad.bIsLocked;

		Result.Add(Summary);
	}

	Result.Sort(
		[](const FIP1SquadSummary& A,
		   const FIP1SquadSummary& B)
		{
			if (A.TeamId != B.TeamId)
			{
				return static_cast<uint8>(A.TeamId) <
					   static_cast<uint8>(B.TeamId);
			}

			return A.SquadId < B.SquadId;
		});

	return Result;
}

void UIP1SquadManager::RefreshSquadSummaries()
{
	// Get the GameState that owns this SquadManager.
	AIP1GameState* GS = OwningGameState;

	if (!IsValid(GS))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("RefreshSquadSummaries: "
				 "GameState not found."));

		return;
	}

	// Only the server can publish squad summaries.
	if (!GS->HasAuthority())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("RefreshSquadSummaries: "
				 "Called without server authority."));

		return;
	}

	// Convert authoritative squad data into
	// lightweight summaries.
	const TArray<FIP1SquadSummary> NewSummaries =
		BuildSquadSummaries();

	// Update the replicated array in GameState.
	GS->PublishSquadSummaries(NewSummaries);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("RefreshSquadSummaries: "
			 "Published %d squad summaries."),
		NewSummaries.Num());
}

FText UIP1SquadManager::GetSquadNameText(EIP1SquadName SquadName)
{
	const UEnum* SquadNameEnum = StaticEnum<EIP1SquadName>();
	
	if (!SquadNameEnum)
	{
		return FText::GetEmpty();
	}
	
	return SquadNameEnum->GetDisplayNameTextByValue(static_cast<int64>(SquadName));
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
	
	// Clear the player's squad membership.
	PlayerState->SetSquadId(INDEX_NONE);
}

EIP1SquadName UIP1SquadManager::FindNextAvailableSquadName(EIP1Team Team) const
{
	if (Team == EIP1Team::None)
	{
		return EIP1SquadName::None;
	}
	
	// Iterate through every valid squad name.
	// Able = 1, Baker = 2, etc.
	
	const uint8 FirstName = static_cast<uint8>(EIP1SquadName::Able);
	
	const uint8 LastName = static_cast<uint8>(EIP1SquadName::Zebra);
	
	for (uint8 NameIndex = FirstName; NameIndex <= LastName; ++NameIndex)
	{
		const EIP1SquadName CandidateName = static_cast<EIP1SquadName>(NameIndex);
		
		bool bNameInUse = false;
		
		// Check existing squads.
		for (const FIP1SquadData& Squad : Squads)
		{
			// Only compare squads on the same team.
			if (Squad.TeamId != Team)
			{
				continue;
			}
			
			if (Squad.SquadName == CandidateName)
			{
				bNameInUse = true;
				break;
			}
		}
		
		// Return the first unsued name.
		if (!bNameInUse)
		{
			return CandidateName;
		}
	}
	
	// All avaialable names are already assigned.
	return EIP1SquadName::None;
}

