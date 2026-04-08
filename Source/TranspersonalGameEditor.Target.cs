// Copyright Transpersonal Game Studio. All Rights Reserved.

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

        // Editor specific optimizations
        bUseUnityBuild = true;
        bUsePCHFiles = true;
        bUseSharedPCHs = true;
        bWithLiveCoding = true;
        
        // Enable modern C++ features
        CppStandard = CppStandardVersion.Cpp20;
        
        // Editor performance settings
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            WindowsPlatform.PCHMemoryAllocationFactor = 2000;
        }

        // Development features
        bUseLoggingInShipping = true;
        bCompileWithStatsWithoutEngine = true;
        bCompileWithPluginSupport = true;

        // Enable editor plugins
        EnablePlugins.AddRange(new string[]
        {
            "MassEntity",
            "MassGameplay",
            "Niagara", 
            "MetaSounds",
            "WorldPartition",
            "Lumen",
            "Nanite",
            "ChaosPhysics",
            "EnhancedInput",
            "EditorScriptingUtilities",
            "PythonScriptPlugin",
            "SequencerScripting"
        });
    }
}