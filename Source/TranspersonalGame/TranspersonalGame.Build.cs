using UnrealBuildTool;

public class TranspersonalGame : ModuleRules
{
    public TranspersonalGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Core UE5 modules required for all systems
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

        // UE5.5 specific modules for advanced features
        PrivateDependencyModuleNames.AddRange(new string[] 
        { 
            "GameplayTags",
            "GameplayTasks",
            "AIModule",
            "NavigationSystem",
            "Niagara",
            "CinematicCamera",
            "LevelSequence",
            "MovieScene",
            "PhysicsCore",
            "Chaos",
            "ChaosVehicles",
            "GeometryCollectionEngine",
            "FieldSystemEngine",
            "PCG",  // Procedural Content Generation
            "WorldPartitionEditor",
            "MassEntity",
            "MassMovement",
            "MassSpawner",
            "MassActors",
            "MassCommon",
            "MassGameplay",
            "MassLOD",
            "MassRepresentation",
            "MassSimulation",
            "StateTree",
            "MetaSounds",
            "AudioMixer",
            "AudioExtensions"
        });

        // Editor-only modules for development tools
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "EditorStyle",
                "EditorWidgets",
                "ToolMenus",
                "PropertyEditor",
                "DetailCustomizations"
            });
        }

        // Platform specific optimizations
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("TRANSPERSONAL_PLATFORM_WINDOWS=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.PS5)
        {
            PublicDefinitions.Add("TRANSPERSONAL_PLATFORM_PS5=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.XSX)
        {
            PublicDefinitions.Add("TRANSPERSONAL_PLATFORM_XBOX=1");
        }

        // Performance and memory optimizations
        PublicDefinitions.AddRange(new string[]
        {
            "TRANSPERSONAL_ENABLE_PROFILING=1",
            "TRANSPERSONAL_MAX_WORLD_SIZE=8192",  // 8km x 8km world limit
            "TRANSPERSONAL_TARGET_FPS_PC=60",
            "TRANSPERSONAL_TARGET_FPS_CONSOLE=30"
        });

        // Development build settings
        if (Target.Configuration == UnrealTargetConfiguration.Development || 
            Target.Configuration == UnrealTargetConfiguration.Debug)
        {
            PublicDefinitions.Add("TRANSPERSONAL_DEVELOPMENT=1");
        }
    }
}