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
        
        // Platform-specific settings
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 2000;
        }

        // Build configuration specific settings
        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseLoggingInShipping = false;
            bUseChecksInShipping = false;
            bCompileICU = false;
            bBuildDeveloperTools = false;
            bBuildWithEditorOnlyData = false;
            
            // Maximum optimization for shipping
            bAllowLTCG = true;
            bPGOOptimize = true;
        }
        else if (Target.Configuration == UnrealTargetConfiguration.Test)
        {
            bUseLoggingInShipping = true;
            bUseChecksInShipping = true;
            bBuildDeveloperTools = true;
        }
        
        // Enable stats and profiling in non-shipping builds
        if (Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
            bUseStatsWithoutEngine = true;
            bCompileRecast = true;
        }

        // Memory and performance settings
        bUseMallocProfiler = (Target.Configuration == UnrealTargetConfiguration.Debug);
        bUseSharedPCHs = true;
        
        // Enable crash reporting
        bIncludeCrashReporter = true;
    }
}