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
                "MassEntity",
                "MassMovement", 
                "MassSpawner",
                "MassAI",
                "PCG",
                "WorldPartition",
                "Chaos"
            });
        }

        // Editor-only dependencies
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "EditorStyle",
                "EditorWidgets",
                "UnrealEd",
                "ToolMenus"
            });
        }

        // Platform-specific optimizations
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("PLATFORM_WINDOWS=1");
        }

        // Performance settings
        bUseUnity = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bFasterWithoutUnity = false;

        // Optimization flags
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
        
        // Enable IWYU for better compile times
        bEnforceIWYU = true;
        
        // Disable legacy features for better performance
        bLegacyPublicIncludePaths = false;
    }
}