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
        ExtraModuleNames.Add("TranspersonalGame");
        
        // Performance optimizations for Jurassic survival game
        bUseUnityBuild = true;
        bUsePCHFiles = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bForceEnableExceptions = false;
        
        // Platform-specific settings
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 2000;
        }
        
        // Build configuration optimizations
        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseLoggingInShipping = false;
            bUseChecksInShipping = false;
            bCompileWithStatsWithoutEngine = false;
        }
    }
}