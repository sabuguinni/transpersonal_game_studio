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
            "PhysicsCore",
            "Chaos",
            "ChaosVehicles",
            "GeometryCollectionEngine",
            "FieldSystemEngine",
            "Niagara",
            "AudioMixer",
            "MetasoundEngine",
            "PCG",
            "WorldPartitionRuntime"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        { 
            "Slate", 
            "SlateCore",
            "ToolMenus",
            "EditorStyle",
            "EditorWidgets",
            "UnrealEd",
            "LevelEditor",
            "PropertyEditor",
            "RenderCore",
            "RHI",
            "Landscape",
            "Foliage",
            "ProceduralMeshComponent",
            "MassEntity",
            "MassCommon",
            "MassMovement",
            "MassSpawner",
            "MassActors",
            "StateTree",
            "StructUtils"
        });
        
        // Optimization settings for prehistoric world simulation
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
        
        // Enable RTTI for consciousness system reflection
        bUseRTTI = true;
        
        // Enable exceptions for advanced AI error handling
        bEnableExceptions = true;
        
        PublicDefinitions.Add("TRANSPERSONAL_GAME=1");
        PublicDefinitions.Add("CONSCIOUSNESS_SYSTEM_ENABLED=1");
        PublicDefinitions.Add("PREHISTORIC_WORLD_SIMULATION=1");
    }
}