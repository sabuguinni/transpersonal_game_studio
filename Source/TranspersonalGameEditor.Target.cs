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
        bBuildInSolutionByDefault = true;
        bUseLoggingInShipping = true;
        
        // Enable hot reload for faster iteration
        bAllowHotReload = true;
        
        // Performance settings for editor
        bUseUnityBuild = true;
        bUsePCHFiles = true;
        
        // Enable all editor features
        bBuildDeveloperTools = true;
        bBuildWithEditorOnlyData = true;
        
        // Source control integration
        bEnableTrace = true;
    }
}