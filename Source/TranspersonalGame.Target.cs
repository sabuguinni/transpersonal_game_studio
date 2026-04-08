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
        
        // Enable hot reload for development
        if (Configuration == UnrealTargetConfiguration.Development ||
            Configuration == UnrealTargetConfiguration.DebugGame)
        {
            bUseIncrementalLinking = true;
            bUseFastMonoCalls = true;
        }
        
        // Shipping optimizations
        if (Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseLoggingInShipping = false;
            bUseChecksInShipping = false;
            bCompileICU = false;
            bBuildDeveloperTools = false;
            bBuildWithEditorOnlyData = false;
        }
        
        // Platform-specific settings
        if (Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 2000;
        }
    }
}