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
        
        // Enable modern C++ features
        CppStandard = CppStandardVersion.Cpp20;
        
        // Editor development features
        bWithLiveCoding = true;
        bBuildDeveloperTools = true;
        bCompileWithStatsWithoutEngine = true;
        bCompileWithPluginSupport = true;
        
        // Enable all logging and checks in editor
        bUseLoggingInShipping = true;
        bUseChecksInShipping = true;
        
        // Disable legacy features
        bLegacyPublicIncludePaths = false;
        
        // Platform-specific settings
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.bStrictConformanceMode = true;
        }

        // Enable chaos physics
        bUseChaos = true;
        bCompileChaos = true;
        bUseChaosChecked = true;
        
        // Editor-specific features
        bBuildWithEditorOnlyData = true;
        bCompileAgainstEngine = true;
        
        // Enable faster compilation for development
        bOverrideBuildEnvironment = true;
        
        // Enable all editor tools and utilities
        bBuildTargetDeveloperTools = true;
        
        // Memory settings for large worlds
        bOverrideBuildEnvironment = true;
    }
}