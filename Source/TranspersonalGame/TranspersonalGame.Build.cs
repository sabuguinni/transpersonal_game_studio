// Copyright Transpersonal Game Studio. All Rights Reserved.

using UnrealBuildTool;

public class TranspersonalGame : ModuleRules
{
    public TranspersonalGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Slate",
            "SlateCore",
            "NavigationSystem",
            "AIModule",
            "GameplayTasks",
            "MassEntity",
            "MassCommon",
            "MassMovement",
            "MassActors",
            "MassSpawner",
            "MassSimulation",
            "MassGameplay",
            "MassLOD",
            "MassReplication",
            "StateTree",
            "GameplayStateTree",
            "StructUtils",
            "Niagara",
            "MetasoundEngine",
            "AudioMixer",
            "Landscape",
            "Foliage",
            "ProceduralMeshComponent",
            "GeometryCollectionEngine",
            "ChaosSolverEngine",
            "ChaosVehicles",
            "PhysicsCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "RenderCore",
            "RHI",
            "ApplicationCore",
            "Json",
            "JsonObjectConverter",
            "HTTP",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "DeveloperSettings",
            "ToolMenus",
            "EditorStyle",
            "EditorWidgets",
            "UnrealEd",
            "BlueprintGraph",
            "KismetCompiler",
            "PropertyEditor",
            "Sequencer",
            "MovieScene",
            "LevelSequence",
            "CinematicCamera"
        });

        // Optimization settings
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
        
        // Enable IWYU
        bEnforceIWYU = true;
        
        // Performance settings for large worlds
        bUseUnity = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        
        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            PublicDefinitions.Add("UE_BUILD_SHIPPING_WITH_EDITOR=0");
            bUseLoggingInShipping = false;
        }
        
        // Platform-specific optimizations
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("PLATFORM_WINDOWS=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.PS5)
        {
            PublicDefinitions.Add("PLATFORM_PS5=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.XSX)
        {
            PublicDefinitions.Add("PLATFORM_XBOX_SERIES_X=1");
        }
    }
}