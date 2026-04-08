// Copyright Transpersonal Game Studio. All Rights Reserved.

using UnrealBuildTool;

public class TranspersonalGame : ModuleRules
{
    public TranspersonalGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        // Core dependencies for Jurassic survival game
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
            "GameplayAbilities",
            "Niagara",
            "AudioMixer",
            "MetasoundEngine",
            "Chaos",
            "ChaosVehicles",
            "PhysicsCore",
            "GeometryCollectionEngine",
            "FieldSystemEngine",
            "Landscape",
            "Foliage",
            "ProceduralMeshComponent",
            "MassEntity",
            "MassMovement",
            "MassSpawner",
            "MassActors",
            "MassCommon",
            "MassSimulation",
            "MassGameplay",
            "StateTree",
            "StructUtils"
        });

        // Editor-only dependencies
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "EditorStyle",
                "EditorWidgets",
                "ToolMenus",
                "PropertyEditor",
                "BlueprintGraph",
                "KismetCompiler",
                "ToolWidgets",
                "EditorSubsystem"
            });
        }

        // Platform-specific optimizations
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("PLATFORM_WINDOWS=1");
            bEnableExceptions = false;
            bUseRTTI = false;
        }

        // Performance defines for Jurassic game
        PublicDefinitions.AddRange(new string[]
        {
            "JURASSIC_GAME=1",
            "ENABLE_MASS_AI=1",
            "ENABLE_PROCEDURAL_WORLD=1",
            "ENABLE_ADVANCED_PHYSICS=1"
        });

        // Optimization settings
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
        
        // Include paths
        PublicIncludePaths.AddRange(new string[]
        {
            "TranspersonalGame/Public",
            "TranspersonalGame/Public/Core",
            "TranspersonalGame/Public/Gameplay",
            "TranspersonalGame/Public/AI",
            "TranspersonalGame/Public/World",
            "TranspersonalGame/Public/Physics",
            "TranspersonalGame/Public/Audio",
            "TranspersonalGame/Public/UI"
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            "TranspersonalGame/Private",
            "TranspersonalGame/Private/Core",
            "TranspersonalGame/Private/Gameplay", 
            "TranspersonalGame/Private/AI",
            "TranspersonalGame/Private/World",
            "TranspersonalGame/Private/Physics",
            "TranspersonalGame/Private/Audio",
            "TranspersonalGame/Private/UI"
        });
    }
}