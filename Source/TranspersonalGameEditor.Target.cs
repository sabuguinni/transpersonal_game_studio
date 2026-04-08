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
        
        // Editor-specific optimizations
        bUseUnityBuild = true;
        bUsePCHFiles = true;
        bUseSharedPCHs = true;
        bUseIncrementalLinking = true;
        bUseFastMonoCalls = true;
        
        // Enable all editor features
        bBuildDeveloperTools = true;
        bBuildWithEditorOnlyData = true;
        bCompileAgainstEngine = true;
        bCompileAgainstCoreUObject = true;
        
        // Development aids
        bUseLoggingInShipping = true;
        bUseChecksInShipping = true;
        
        // Platform-specific editor settings
        if (Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 3000;
            WindowsPlatform.bStrictConformanceMode = true;
        }
    }
}