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
        
        // Enable faster iteration
        bWithLiveCoding = true;
        
        // Shipping optimizations
        if (Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseLoggingInShipping = false;
            bUseChecksInShipping = false;
            bCompileWithStatsWithoutEngine = false;
            bCompileWithPluginSupport = true;
        }

        // Development features
        if (Configuration == UnrealTargetConfiguration.Development || 
            Configuration == UnrealTargetConfiguration.DebugGame)
        {
            bUseLoggingInShipping = true;
            bUseChecksInShipping = true;
            bCompileWithStatsWithoutEngine = true;
        }

        // Platform specific settings
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 2000;
        }

        // Enable Chaos Physics
        bUseChaos = true;
        bCompileChaos = true;
        bUseChaosChecked = false;
        bCustomSceneQueryStructure = true;

        // Audio settings
        bCompileWithPluginSupport = true;
        
        // Disable unnecessary features for performance
        bBuildDeveloperTools = (Configuration != UnrealTargetConfiguration.Shipping);
        bBuildWithEditorOnlyData = Target.bBuildEditor;
        
        // Memory and performance
        bForceEnableExceptions = false;
        bForceEnableObjCExceptions = false;
        bForceEnableRTTI = false;
    }
}