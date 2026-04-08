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
        
        // Enable IWYU for better compile times
        bIWYU = true;
        
        // Shipping optimizations
        if (Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseLoggingInShipping = false;
            bUseChecksInShipping = false;
            bCompileICU = false;
            bWithServerCode = false;
            bCompileWithStatsWithoutEngine = false;
            bCompileWithPluginSupport = true;
            bIncludePluginsForTargetPlatforms = true;
            
            // Aggressive optimization for shipping
            bAllowLTCG = true;
            bPGOOptimize = true;
        }
        
        // Development optimizations
        if (Configuration == UnrealTargetConfiguration.Development)
        {
            bUseLoggingInShipping = true;
            bUseChecksInShipping = true;
            bWithServerCode = false;
            bCompileWithStatsWithoutEngine = true;
        }
        
        // Platform-specific settings
        if (Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 2000;
            bBuildAdditionalConsoleApp = false;
        }
        
        // Mass Entity optimizations for large-scale simulation
        GlobalDefinitions.Add("MASS_ENTITY_OPTIMIZATIONS=1");
        GlobalDefinitions.Add("LARGE_WORLD_COORDINATES=1");
        
        // Dinosaur AI specific optimizations
        GlobalDefinitions.Add("DINOSAUR_AI_OPTIMIZED=1");
        GlobalDefinitions.Add("MAX_DINOSAUR_COUNT=50000");
        
        // Memory optimizations for open world
        GlobalDefinitions.Add("OPEN_WORLD_STREAMING=1");
        GlobalDefinitions.Add("WORLD_PARTITION_ENABLED=1");
    }
}