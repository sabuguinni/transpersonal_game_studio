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

        // Gameplay dependencies
        PrivateDependencyModuleNames.AddRange(new string[] 
        { 
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "AIModule",
            "NavigationSystem",
            "Niagara",
            "AudioMixer",
            "MetasoundEngine"
        });

        // UE5 specific systems
        if (Target.Version.MajorVersion >= 5)
        {
            PublicDependencyModuleNames.AddRange(new string[]
            {
                "PCG",              // Procedural Content Generation
                "MassEntity",       // Mass AI system
                "MassMovement",
                "MassSpawner",
                "MassActors",
                "WorldPartition"    // World streaming
            });
        }

        // Editor-only dependencies
        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "EditorStyle",
                "EditorWidgets",
                "ToolMenus"
            });
        }

        // Platform-specific optimizations
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("PLATFORM_WINDOWS=1");
        }

        // Performance settings
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
        
        // Enable IWYU (Include What You Use)
        bEnforceIWYU = true;
        
        // Disable unity builds for better incremental compilation during development
        bUseUnity = false;
    }
}