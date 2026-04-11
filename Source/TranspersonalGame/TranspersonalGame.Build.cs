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
            "GameplayTags",
            "PhysicsCore",
            "Chaos",
            "ChaosVehicles",
            "GeometryCollectionEngine",
            "FieldSystemEngine",
            "Niagara",
            "AudioMixer",
            "MetasoundEngine",
            "ProceduralMeshComponent",
            "Landscape",
            "Foliage",
            "MassEntity",
            "MassMovement",
            "MassSpawner",
            "MassActors",
            "MassCommon",
            "MassSimulation",
            "MassGameplay",
            "StructUtils",
            "StateTreeModule",
            "SmartObjectsModule",
            "WorldPartition",
            "PCG"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        { 
            "RenderCore",
            "RHI",
            "ApplicationCore",
            "Json",
            "JsonObjectConverter",
            "HTTP",
            "ToolMenus",
            "EditorStyle",
            "EditorWidgets",
            "UnrealEd",
            "LevelEditor",
            "PropertyEditor",
            "DetailCustomizations",
            "ComponentVisualizers",
            "EngineSettings",
            "DeveloperSettings"
        });

        // Enable RTTI for advanced reflection
        bUseRTTI = true;
        
        // Enable exceptions for error handling
        bEnableExceptions = true;
        
        // Optimization settings
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
        
        // Include paths
        PublicIncludePaths.AddRange(new string[] 
        {
            "TranspersonalGame/Public",
            "TranspersonalGame/Public/Core",
            "TranspersonalGame/Public/Systems",
            "TranspersonalGame/Public/Components",
            "TranspersonalGame/Public/Actors",
            "TranspersonalGame/Public/UI",
            "TranspersonalGame/Public/World",
            "TranspersonalGame/Public/AI",
            "TranspersonalGame/Public/Audio",
            "TranspersonalGame/Public/VFX"
        });
        
        PrivateIncludePaths.AddRange(new string[] 
        {
            "TranspersonalGame/Private",
            "TranspersonalGame/Private/Core",
            "TranspersonalGame/Private/Systems",
            "TranspersonalGame/Private/Components",
            "TranspersonalGame/Private/Actors",
            "TranspersonalGame/Private/UI",
            "TranspersonalGame/Private/World",
            "TranspersonalGame/Private/AI",
            "TranspersonalGame/Private/Audio",
            "TranspersonalGame/Private/VFX"
        });

        // Preprocessor definitions
        PublicDefinitions.AddRange(new string[]
        {
            "TRANSPERSONAL_GAME=1",
            "UE_BUILD_SHIPPING_WITH_EDITOR=1"
        });

        if (Target.Configuration == UnrealTargetConfiguration.Development || 
            Target.Configuration == UnrealTargetConfiguration.Debug)
        {
            PublicDefinitions.Add("TRANSPERSONAL_DEBUG=1");
        }
    }
}