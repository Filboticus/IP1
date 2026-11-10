// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "IP1/GameModes/IP1TeamTypes.h"
#include "IP1PlayerRoleTypes.h"
#include "IP1PlayerState.generated.h"

/**
 * AIP1PlayerState
 *
 *	Base player state class used by this project.
 */

DECLARE_MULTICAST_DELEGATE_OneParam(FOnIP1TeamChanged, EIP1Team);

UCLASS()
class IP1_API AIP1PlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	AIP1PlayerState();
	
	// Team Identity
	UPROPERTY(ReplicatedUsing = OnRep_Team, BlueprintReadOnly, Category = "Player|Team")
	EIP1Team Team = EIP1Team::None;

	UFUNCTION(BlueprintCallable, Category = "Player|Team")
	void SetTeam(EIP1Team NewTeam);

	UFUNCTION(BlueprintCallable, Category = "Player|Team")
	EIP1Team GetTeam() const {return Team;}

	// Combat role
public:
	UFUNCTION(BlueprintPure, Category = "Player|Role")
	EIP1PlayerRole GetSelectedRole() const { return SelectedRole; }
	
	// Called only by authoritative server logic.
	void SetSelectedRole(EIP1PlayerRole NewSelectedRole);
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_SelectedRole, BlueprintReadOnly, Category = "Player|Role")
	EIP1PlayerRole SelectedRole = EIP1PlayerRole::None;
	
	UFUNCTION()
	void OnRep_SelectedRole();

	// Squad membership
public:
	UPROPERTY(ReplicatedUsing = OnRep_SquadID, BlueprintReadOnly, Category = "Squad")
	int32 SquadId = INDEX_NONE;

	UFUNCTION(BlueprintPure, Category = "Squad")
	int32 GetSquadId() const;

	// Called by the authoritativbe squad manager. 
	UFUNCTION(BlueprintCallable, Category = "Squad")
	void SetSquadId(int32 NewSquadId);
	
	// Squad leadership
	UPROPERTY(ReplicatedUsing = OnRep_IsSquadLeader)
	bool bIsSquadLeader = false;
	
	UFUNCTION(BlueprintPure, Category = "Squad")
	bool IsSquadLeader() const;

	UFUNCTION(BlueprintCallable, Category = "Squad")
	void SetIsSquadLeader(bool bNewIsSquadLeader);
	
	// Combat statistics
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player|Stats")
	int32 Kills = 0;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player|Stats")
	int32 Deaths = 0;

	UFUNCTION(BlueprintCallable, Category = "Player|Stats")
	void AddKill();

	UFUNCTION(BlueprintCallable, Category = "Player|Stats")
	void AddDeath();

	FOnIP1TeamChanged OnTeamChanged;

protected:
	
	virtual void BeginPlay() override;
	
	// Called on clients when Team is replicated.
	UFUNCTION()
	void OnRep_Team();

	// Called on clients when SquadID is replicated
	UFUNCTION()
	void OnRep_SquadId();

	// Called on clients when IsSquadLeader is replicated
	UFUNCTION()
	void OnRep_IsSquadLeader();

	// Use inherited APlayerState functions for:
	// Player name: SetPlayerName() / GetPlayerName()
	// Score:       SetScore() / GetScore()

public:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
