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

        // Gameplay systems
        PrivateDependencyModuleNames.AddRange(new string[] 
        { 
            "GameplayTasks",
            "GameplayTags",
            "AIModule",
            "NavigationSystem",
            "Niagara",
            "PhysicsCore",
            "Chaos"
        });

        // UE5 specific systems
        if (Target.Version.MajorVersion >= 5)
        {
            PublicDependencyModuleNames.AddRange(new string[]
            {
                "MassEntity",
                "MassCommon",
                "MassMovement",
                "MassActors",
                "MassSpawner",
                "MassGameplay",
                "StateTreeModule",
                "StructUtils"
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
                "ToolMenus"
            });
        }

        // Platform specific optimizations
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("PLATFORM_WINDOWS=1");
        }

        // Performance settings
        bUseUnity = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bFasterWithoutUnity = false;

        // Enable IWYU for cleaner includes
        bEnforceIWYU = true;
        
        // Optimization settings
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
    }
}