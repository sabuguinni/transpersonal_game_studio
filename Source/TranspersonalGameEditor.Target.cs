using UnrealBuildTool;
using System.Collections.Generic;

public class TranspersonalGameEditorTarget : TargetRules
{
    public TranspersonalGameEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
        
        ExtraModuleNames.AddRange(new string[] { "TranspersonalGame" });

        bUseUnityBuild = true;
        bUsePCHFiles = true;

        bBuildDeveloperTools = true;
        bBuildWithEditorOnlyData = true;
    }
}
