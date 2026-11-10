#pragma once

#include "CoreMinimal.h"
#include "IP1TeamTypes.generated.h"

UENUM(BlueprintType)
enum class EIP1Team : uint8
{
	None	UMETA(DisplayName = "None"),
	Allies	UMETA(DisplayName = "Allies"),
	Axis	UMETA(DisplayName = "Axis")
};