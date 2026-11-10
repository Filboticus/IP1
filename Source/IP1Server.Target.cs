// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class IP1ServerTarget : TargetRules
{
    public IP1ServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;

        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;

        ExtraModuleNames.Add("IP1");
    }
}
