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
            "Niagara",
            "PhysicsCore",
            "Chaos",
            "ChaosVehicles",
            "GeometryCollectionEngine",
            "FieldSystemEngine",
            "ProceduralMeshComponent",
            "Landscape",
            "Foliage",
            "MetasoundEngine",
            "AudioMixer",
            "CinematicCamera",
            "LevelSequence",
            "MovieScene"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "Slate",
            "SlateCore",
            "ToolMenus",
            "EditorStyle",
            "EditorWidgets",
            "UnrealEd",
            "PropertyEditor",
            "Kismet",
            "KismetCompiler",
            "BlueprintGraph",
            "AnimGraph",
            "AnimationCore",
            "AnimGraphRuntime",
            "MotionWarping",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "NetCore",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "HTTP",
            "Json",
            "JsonUtilities"
        });

        // Optimization settings
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
        
        // Enable RTTI for reflection
        bUseRTTI = true;
        
        // Enable exceptions for error handling
        bEnableExceptions = true;
        
        // PCG and World Partition support
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "PCG",
                "WorldPartitionEditor",
                "LevelEditor",
                "ContentBrowser",
                "AssetTools"
            });
        }
        
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "PCG",
            "WorldPartition"
        });
    }
}