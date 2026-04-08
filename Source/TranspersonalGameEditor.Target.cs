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
        ExtraModuleNames.Add("TranspersonalGame");
        
        // Editor-specific optimizations
        bUseUnityBuild = true;
        bUsePCHFiles = true;
        bBuildDeveloperTools = true;
        bBuildWithEditorOnlyData = true;
        
        // Enable all editor features for development
        bCompileWithPluginSupport = true;
        bIncludePluginsForTargetPlatforms = true;
        
        // Development tools
        bCompileWithAccessibilitySupport = true;
        bWithLiveCoding = true;
    }
}