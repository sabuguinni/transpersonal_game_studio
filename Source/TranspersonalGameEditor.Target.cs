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
        
        // Enable live coding for faster iteration
        bWithLiveCoding = true;
        bUseIncrementalLinking = true;
        
        // Editor features
        bBuildDeveloperTools = true;
        bBuildWithEditorOnlyData = true;
        bCompileWithStatsWithoutEngine = true;
        bCompileWithPluginSupport = true;
        
        // Debug features for development
        bUseLoggingInShipping = true;
        bUseChecksInShipping = true;
        
        // Platform specific editor settings
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 2000;
            WindowsPlatform.bStrictConformanceMode = true;
        }

        // Enable Chaos Physics in editor
        bUseChaos = true;
        bCompileChaos = true;
        bUseChaosChecked = true;
        bCustomSceneQueryStructure = true;

        // Editor-specific performance settings
        bForceEnableExceptions = true; // Needed for some editor tools
        bForceEnableRTTI = true; // Needed for reflection in editor
        
        // Enable all editor subsystems
        bCompileAgainstEngine = true;
        bCompileAgainstCoreUObject = true;
        bCompileAgainstApplicationCore = true;
        
        // Development tools
        bIncludePluginsForTargetPlatforms = true;
        bCompileWithAccessibilitySupport = true;
        
        // Faster editor compilation
        bUsePrecompiled = false; // Use source builds for better debugging
    }
}