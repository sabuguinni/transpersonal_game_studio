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
        bBuildDeveloperTools = true;
        bBuildWithEditorOnlyData = true;
        bCompileWithPluginSupport = true;
        bIncludePluginsForTargetPlatforms = true;

        // Enable all debugging features in editor
        bUseLoggingInShipping = true;
        bUseChecksInShipping = true;
        bCompileWithStatsWithoutEngine = true;

        // Editor performance
        bUsePCHFiles = true;
        bUseUnityBuild = true;
        bForceUnityBuild = false;

        // Enable modern C++ features
        CppStandard = CppStandardVersion.Cpp17;
        
        // Build optimization for editor
        bBuildInSolutionByDefault = true;
    }
}