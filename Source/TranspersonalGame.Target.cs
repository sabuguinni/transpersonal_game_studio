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
        
        // Enable modern C++ features
        CppStandard = CppStandardVersion.Cpp20;
        
        // Disable legacy features
        bLegacyPublicIncludePaths = false;
        
        // Enable faster iteration
        bWithLiveCoding = true;
        
        // Platform-specific settings
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.bStrictConformanceMode = true;
        }

        // Build configuration specific settings
        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseLoggingInShipping = false;
            bUseChecksInShipping = false;
            bCompileWithStatsWithoutEngine = false;
            bCompileWithPluginSupport = true;
        }
        else
        {
            bUseLoggingInShipping = true;
            bUseChecksInShipping = true;
            bCompileWithStatsWithoutEngine = true;
        }

        // Enable Mass Entity for crowd simulation
        bCompileAgainstEngine = true;
        bBuildDeveloperTools = Target.Configuration != UnrealTargetConfiguration.Shipping;
        
        // Memory and performance
        bOverrideBuildEnvironment = true;
        
        // Enable chaos physics
        bUseChaos = true;
        bCompileChaos = true;
        bUseChaosChecked = Target.Configuration != UnrealTargetConfiguration.Shipping;
        
        // Audio settings for MetaSounds
        bCompileWithPluginSupport = true;
    }
}