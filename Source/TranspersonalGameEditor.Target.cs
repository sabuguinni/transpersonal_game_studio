// Copyright Transpersonal Game Studio. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TranspersonalGameEditorTarget : TargetRules
{
    public TranspersonalGameEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        
        ExtraModuleNames.AddRange(new string[] { "TranspersonalGame" });
        
        // Editor optimizations
        bUseUnityBuild = true;
        bUsePCHFiles = true;
        bUseSharedPCHs = true;
        bIWYU = true;
        
        // Editor-specific settings
        bBuildDeveloperTools = true;
        bBuildWithEditorOnlyData = true;
        bCompileAgainstEngine = true;
        bCompileAgainstCoreUObject = true;
        bCompileWithPluginSupport = true;
        bIncludePluginsForTargetPlatforms = true;
        
        // Enable all editor features for development
        bWithServerCode = true;
        bCompileWithStatsWithoutEngine = true;
        bCompileWithAvailabilityConditioning = false;
        
        // Fast iteration settings
        bUseIncrementalLinking = true;
        bUseFastMonoCalls = true;
        bUseAdaptiveUnityBuild = true;
        
        // Platform-specific editor settings
        if (Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 3000;
            bBuildAdditionalConsoleApp = false;
        }
        
        // Editor-specific definitions for development
        GlobalDefinitions.Add("TRANSPERSONAL_EDITOR=1");
        GlobalDefinitions.Add("MASS_ENTITY_DEBUG=1");
        GlobalDefinitions.Add("DINOSAUR_AI_DEBUG=1");
        GlobalDefinitions.Add("WORLD_PARTITION_DEBUG=1");
        
        // Enable detailed logging in editor
        GlobalDefinitions.Add("DETAILED_LOGGING=1");
        GlobalDefinitions.Add("PERFORMANCE_PROFILING=1");
    }
}