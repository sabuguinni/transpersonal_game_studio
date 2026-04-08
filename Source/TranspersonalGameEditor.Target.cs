using UnrealBuildTool;
using System.Collections.Generic;

public class TranspersonalGameEditorTarget : TargetRules
{
    public TranspersonalGameEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
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

        // Editor-specific optimizations
        bUseUnityBuild = true;
        bUsePCHFiles = true;
        bBuildDeveloperTools = true;
        
        // Enable all debugging features in editor
        bUseLoggingInShipping = true;
        bCompileWithStatsWithoutEngine = true;
        bBuildWithEditorOnlyData = true;
    }
}