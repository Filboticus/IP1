// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "IP1MatchTypes.h"
#include "IP1/Squad/IP1SquadSummary.h"
#include "IP1/Squad/IP1SquadData.h"
#include "IP1GameState.generated.h"

class UIP1SquadManager;

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FIP1OnSquadSummariesChanged);

UCLASS()
class IP1_API AIP1GameState : public AGameState
{
	GENERATED_BODY()

public:

	AIP1GameState();

	virtual void BeginPlay() override;

	// Current phase of the match.
	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase, BlueprintReadOnly, Category = "Match")
	EIP1MatchPhase MatchPhase = EIP1MatchPhase::WaitingToStart;

	// Server world time when the current countdown expires.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	float MatchEndServerTime = 0.0f;

	// Server-side functions used by GameMode.
	void SetMatchPhase(EIP1MatchPhase NewPhase);

	void SetMatchEndServerTime(float NewEndTime);

	// Available to C++ and Blueprints
	UFUNCTION(BlueprintPure, Category = "Match")
	float GetMatchTimeRemaining() const;
	
	UFUNCTION(BlueprintPure, Category = "Match")
	EIP1MatchPhase GetMatchPhase() const;

	UFUNCTION(BlueprintPure, Category = "Match")
	FString GetMatchPhaseString() const;

	// The squad manager is owned by this GameState.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squads")
	TObjectPtr<UIP1SquadManager> SquadManager;

	UFUNCTION(BlueprintPure, Category = "Squads")
	UIP1SquadManager* GetSquadManager() const;

	// Replicated snapshot of the authoritative squad roster.
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedSquads, BlueprintReadOnly, Category = "Squads")
	TArray<FIP1SquadData> ReplicatedSquads;

	// Called by the manager to publish authoritative squad data.
	void UpdateReplicatedSquads(const TArray<FIP1SquadData>& NewSquads);
	
	UPROPERTY(BlueprintAssignable, Category = "Squads")
	FIP1OnSquadSummariesChanged OnSquadSummariesChanged;
	
	UFUNCTION(BlueprintPure, Category="Squads")
	const TArray<FIP1SquadSummary>& GetSquadSummaries() const;
	
	// Server only. Called after authoritative
	// squad data changes.
	void PublishSquadSummaries(
	   const TArray<FIP1SquadSummary>& NewSummaries);
	

protected:
	UPROPERTY(ReplicatedUsing = OnRep_SquadSummaries)
	TArray<FIP1SquadSummary> SquadSummaries;
	
	UFUNCTION()
	void OnRep_SquadSummaries();
	
	UFUNCTION()
	void OnRep_MatchPhase();

	UFUNCTION()
	void OnRep_ReplicatedSquads();

public:
	void virtual GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
