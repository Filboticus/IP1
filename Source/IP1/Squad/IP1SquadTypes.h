#pragma once

#include "CoreMinimal.h"
#include "IP1SquadTypes.generated.h"

UENUM(BlueprintType)
enum class EIP1SquadType : uint8
{
	None		UMETA(DisplayName = "None"),
	Command		UMETA(DisplayName = "Commander"),
	Infantry	UMETA(DisplayName = "Infantry"),
	Armor		UMETA(DisplayName = "Armor"),
	Recon		UMETA(DisplayName = "Recon"),
};