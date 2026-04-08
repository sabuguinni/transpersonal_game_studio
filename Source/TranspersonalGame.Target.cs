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

        // Performance optimizations for shipping builds
        if (Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseLoggingInShipping = false;
            bUseChecksInShipping = false;
            bCompileWithStatsWithoutEngine = false;
            bCompileWithPluginSupport = true;
        }

        // Development optimizations
        if (Configuration == UnrealTargetConfiguration.Development)
        {
            bUseLoggingInShipping = true;
            bUseChecksInShipping = true;
            bCompileWithStatsWithoutEngine = true;
        }

        // Platform-specific settings
        if (Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 2000;
        }

        // Enable modern C++ features
        CppStandard = CppStandardVersion.Cpp17;
        
        // Build optimization
        bBuildInSolutionByDefault = true;
    }
}