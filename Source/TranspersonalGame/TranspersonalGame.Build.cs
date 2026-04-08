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

        // Private dependencies for game systems
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
            "MetasoundEngine"
        });

        // Editor-only dependencies
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

        // Platform-specific optimizations
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("PLATFORM_WINDOWS=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDefinitions.Add("PLATFORM_MAC=1");
        }

        // Performance settings
        bUseUnity = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bFasterWithoutUnity = false;
        
        // Enable IWYU for cleaner includes
        bEnforceIWYU = true;
        
        // Optimization flags
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
    }
}