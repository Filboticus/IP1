#pragma once

#include "CoreMinimal.h"
#include "IP1/GameModes/IP1TeamTypes.h"
#include "IP1SquadNames.h"
#include "IP1SquadTypes.h"

#include "IP1SquadSummary.generated.h"

/**
 * Lightweight squad information replicated through GameState.
 *
 * Used by client deployment UI to display available squads.
 *
 * The authoritative squad data remains in UIP1SquadManager.
 */

USTRUCT(BlueprintType)
struct FIP1SquadSummary
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	int32 SquadId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	EIP1SquadName SquadName = EIP1SquadName::None;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	EIP1SquadType SquadType = EIP1SquadType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	EIP1Team TeamId = EIP1Team::None;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	int32 MemberCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	int32 MaxMembers = 6;

	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	bool bIsLocked = false;
};