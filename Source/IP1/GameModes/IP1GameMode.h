// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "IP1TeamTypes.h"
#include "IP1/Player/IP1PlayerRoleTypes.h"
#include "IP1GameMode.generated.h"

/**
 * AIP1GameMode
 *
 * The base game mode class used by this project.
 */

class AIP1PlayerState;
class AIP1GameState;
class AIP1PlayerController;

UCLASS()
class IP1_API AIP1GameMode : public AGameMode
{
	GENERATED_BODY()

public:
	 AIP1GameMode();
	
	virtual void StartPlay() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;
	
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
protected:
	virtual void BeginPlay() override;

public:
	/**
	 * Match State
	 */

	// Configurable in the GameMode Blueprint defaults.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	float WarmupDuration = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	float MatchDuration = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	float PostMatchDuration = 10.0f;

	virtual void StartMatch() override;

	virtual void EndMatch() override;

protected:
	void BeginWarmup();

	void BeginPostMatch();

	void HandlePostMatchFinished();

	FTimerHandle MatchPhaseTimerHandle;
	
	/**
	 * Team Management
	 */
	
	// Determine whether a team has room for another player.
	bool CanJoinTeam(EIP1Team Team) const;

	// Adds a PlayerState to a team.
	void AssignPlayerTeam(AIP1PlayerState* PlayerState, EIP1Team Team);

	// Removes a PlayerState from its current team.
	void RemovePlayerFromTeam(AIP1PlayerState* PlayerState);

	// Recalculates the number of players on each team.
	void UpdateTeamCounts();
	
	

public:

	/**
	 * Team Information
	 */

	// Returns the number of players currently assigned to a team.
	UFUNCTION(BlueprintPure, Category = "Teams")
	int32 GetTeamPlayerCount(EIP1Team Team) const;

	/**
	 * Team Configuration
	 */
	
	// Maximum players allow on each team
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams")
	int32 MaxPlayersPerTeam = 5;

	// Total number of players allow in the match
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams")
	int32 MaxPlayers = 10;

	/**
	* Current Team Counts
	*/
	
	// Current team population.
	UPROPERTY(BlueprintReadOnly, Category = "Teams")
	int32 AlliedPlayerCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Teams")
	int32 AxisPlayerCount = 0;

	FTimerHandle RespawnTimerHandle;
 
	// Respawning
	float RespawnDelay = 5.0f;

	void ScheduleRespawn(AController* Controller);

	void RespawnPlayer(AController* Controller);
	
	void HandleTeamSelection(AIP1PlayerController* PlayerController, EIP1Team RequestedTeamID);
	
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	
	virtual void RestartPlayer(AController* NewPlayer) override;
	
	/** 
	 * Combat role
	 */
	 
	void HandleRoleSelection(AIP1PlayerController* PlayerController, EIP1PlayerRole RequestedRole);
	 
};
