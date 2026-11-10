#pragma once

#include "CoreMinimal.h"
#include "IP1MatchTypes.generated.h"

UENUM(Blueprintable)
enum class EIP1MatchPhase : uint8
{
	WaitingToStart	UMETA(DisplayName = "Waiting To Start"),
	InProgress		UMETA(DisplayName = "In Progress"),
	PostMatch		UMETA(DisplayName = "Post Match")
};
