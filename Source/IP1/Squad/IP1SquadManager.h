// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "IP1SquadData.h"
#include "IP1/GameModes/IP1TeamTypes.h"
#include "IP1/Player/IP1PlayerRoleTypes.h"
#include "IP1SquadManager.generated.h"

class AIP1GameState;
class AIP1PlayerState;

/**
 * 
 */
UCLASS(BlueprintType)
class IP1_API UIP1SquadManager : public UObject
{
	GENERATED_BODY()

public:
	// Initialize the manager with its owning GameState.
	void Initialize(AIP1GameState* InGameState);

	// Create a squad and returns its ID, or INDEX_NONE on failure.
	int32 CreateSquad(EIP1Team TeamId, int32 MaxMembers = 5);

	// Assigns a player to an existing squad.
	bool AssignPlayerToSquad(AIP1PlayerState* PlayerState, int32 SquadId);

	// Removes a player from their current squad.
	bool RemovePlayerFromSquad(AIP1PlayerState* PlayerState);

	// Retrieves a copy of the squad data.
	bool GetSquadById(int32 SquadId, FIP1SquadData& OutSquad) const;

	//Returns the squad roster for server-side insepction.
	const TArray<FIP1SquadData>& GetSquads() const
	{
		return Squads;
	}
	
	// Combat role
	bool CanSelectRole(int32 SquadID, const EIP1PlayerRole RequestedRole, AIP1PlayerState* RequestingPlayer) const;
	
	int32 GetRoleLimit(EIP1PlayerRole Role) const;
	
	int32 CountPlayerWithRole(int32 SquadID, AIP1PlayerState* PlayerState, EIP1PlayerRole RequestedRole, const AIP1PlayerState* ExcludedPlayer = nullptr) const;

private:
	UPROPERTY()
	TObjectPtr<AIP1GameState> OwningGameState = nullptr;

	// Authoritative squad data, maintained by this manager.
	UPROPERTY()
	TArray<FIP1SquadData> Squads;

	int32 NextSquadId = 1;

	int32 FindSquadIndexById(int32 SquadID) const;

	void RemovePlayerFromSquadInternal(AIP1PlayerState* PlayerState);

	// Copies authoritative data to GameState for replication.
	void PublishSquads();
};
