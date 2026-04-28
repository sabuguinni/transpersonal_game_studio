using UnrealBuildTool;
using System.Collections.Generic;

public class TranspersonalGameTarget : TargetRules
{
    public TranspersonalGameTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
        
        ExtraModuleNames.AddRange(new string[] { "TranspersonalGame" });

        bUseUnityBuild = true;
        bUsePCHFiles = true;

        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseLoggingInShipping = false;
            bUseChecksInShipping = false;
            bBuildDeveloperTools = false;
            bBuildWithEditorOnlyData = false;
        }

        bUseSharedPCHs = true;
    }
}
