// Copyright Transpersonal Game Studio. All Rights Reserved.

using UnrealBuildTool;

public class TranspersonalGame : ModuleRules
{
    public TranspersonalGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Core dependencies
        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Slate",
            "SlateCore"
        });

        // Private dependencies for advanced systems
        PrivateDependencyModuleNames.AddRange(new string[] 
        { 
            "RenderCore",
            "RHI",
            "NavigationSystem",
            "AIModule",
            "GameplayTasks",
            "GameplayTags",
            "PhysicsCore",
            "Chaos",
            "ChaosVehicles",
            "Niagara",
            "AudioMixer",
            "MetasoundEngine",
            "LevelSequence",
            "MovieScene",
            "CinematicCamera"
        });

        // UE5 specific modules
        if (Target.Version.MajorVersion >= 5)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "MassEntity",
                "MassMovement", 
                "MassSpawner",
                "MassActors",
                "MassCommon",
                "MassSimulation",
                "MassGameplay",
                "WorldPartition",
                "Lumen",
                "Nanite"
            });
        }

        // Development and editor dependencies
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "EditorStyle",
                "EditorWidgets",
                "ToolMenus",
                "PropertyEditor"
            });
        }

        // Platform specific optimizations
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("PLATFORM_WINDOWS=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDefinitions.Add("PLATFORM_MAC=1");
        }

        // Build configuration specific settings
        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            PublicDefinitions.Add("UE_BUILD_SHIPPING=1");
            bUseLoggingInShipping = false;
        }
        else
        {
            PublicDefinitions.Add("UE_BUILD_DEVELOPMENT=1");
        }

        // Performance settings
        bUseUnity = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bFasterWithoutUnity = false;
    }
}