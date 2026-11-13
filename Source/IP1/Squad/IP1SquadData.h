#pragma once

#include "CoreMinimal.h"
#include "IP1/GameModes/IP1TeamTypes.h"
#include "IP1SquadNames.h"
#include "IP1SquadTypes.h"
#include "IP1SquadData.generated.h"

class AIP1PlayerState;

USTRUCT(BlueprintType)
struct FIP1SquadData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	int32 SquadId = INDEX_NONE;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	EIP1SquadName SquadName = EIP1SquadName::None;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	EIP1SquadType SquadType = EIP1SquadType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	EIP1Team TeamId = EIP1Team::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	TObjectPtr<AIP1PlayerState> SquadLeader = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	TArray<TObjectPtr<AIP1PlayerState>> Members;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	int32 MemberCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	int32 MaxMembers = 6;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	bool bIsLocked = false;
};