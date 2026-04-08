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

        ExtraModuleNames.AddRange(new string[] { "TranspersonalGame" });

        // Performance optimizations
        bUseUnityBuild = true;
        bUsePCHFiles = true;
        bUseSharedPCHs = true;
        
        // Enable modern C++ features
        CppStandard = CppStandardVersion.Cpp20;
        
        // Platform specific settings
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 2000;
        }

        // Build configuration optimizations
        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseLoggingInShipping = false;
            bUseChecksInShipping = false;
            bCompileICU = false;
        }
        else
        {
            bUseLoggingInShipping = true;
            bWithLiveCoding = true;
        }

        // Enable required plugins
        EnablePlugins.AddRange(new string[]
        {
            "MassEntity",
            "MassGameplay", 
            "Niagara",
            "MetaSounds",
            "WorldPartition",
            "Lumen",
            "Nanite",
            "ChaosPhysics",
            "EnhancedInput"
        });
    }
}