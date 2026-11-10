#pragma once

#include "CoreMinimal.h"
#include "IP1PlayerRoleTypes.generated.h"

UENUM(BlueprintType)
enum class EIP1PlayerRole : uint8
{
	None				UMETA(DisplayName = "None"),
	SquadLeader			UMETA(DisplayName = "Squad Leader"),
	Rifleman			UMETA(DisplayName = "Rifleman"),
	Assault				UMETA(DisplayName = "Assault"),
	AutomaticRifleman	UMETA(DisplayName = "Automatic Rifleman"),
	Medic				UMETA(DisplayName = "Medic"),
	Support				UMETA(DisplayName = "Support"),
	MachineGunner		UMETA(DisplayName = "Machine Gunner"),
	AntiTank			UMETA(DisplayName = "Anti-Tank"),
	Engineer			UMETA(DisplayName = "Engineer")
};
