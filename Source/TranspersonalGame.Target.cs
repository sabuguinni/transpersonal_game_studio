using UnrealBuildTool;
using System.Collections.Generic;

public class TranspersonalGameTarget : TargetRules
{
    public TranspersonalGameTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        
        ExtraModuleNames.AddRange(new string[] 
        { 
            "TranspersonalGame",
            "TranspersonalCore",
            "TranspersonalAI",
            "TranspersonalWorld",
            "TranspersonalPlayer"
        });

        // Performance optimizations
        bUseUnityBuild = true;
        bUsePCHFiles = true;
        
        // Enable for shipping builds
        if (Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseLoggingInShipping = false;
            bUseChecksInShipping = false;
            bCompileWithStatsWithoutEngine = false;
        }

        // Development settings
        if (Configuration == UnrealTargetConfiguration.Development || 
            Configuration == UnrealTargetConfiguration.DebugGame)
        {
            bUseLoggingInShipping = true;
            bCompileWithStatsWithoutEngine = true;
        }

        // Platform-specific settings
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 2000;
        }
    }
}