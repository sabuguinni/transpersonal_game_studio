// Copyright Transpersonal Game Studio. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TranspersonalGameTarget : TargetRules
{
    public TranspersonalGameTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        
        ExtraModuleNames.AddRange(new string[] { 
            "TranspersonalGame",
            "CoreSystems",
            "WorldGeneration", 
            "DinosaurAI",
            "SurvivalSystems"
        });

        // Performance optimizations for prehistoric world simulation
        bUseUnityBuild = true;
        bUsePCHFiles = true;
        bUseSharedPCHs = true;
        
        // Enable for large world support (Jurassic landscape)
        bWithLiveCoding = false; // Disabled for shipping builds
        
        // Memory optimization for massive dinosaur crowds
        WindowsPlatform.PCHMemoryAllocationFactor = 2000;
    }
}