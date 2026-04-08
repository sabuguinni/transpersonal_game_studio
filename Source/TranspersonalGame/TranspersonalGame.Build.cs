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
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "AIModule",
            "NavigationSystem",
            "Niagara",
            "AudioMixer",
            "MetasoundEngine"
        });

        // World generation and streaming
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Landscape",
            "Foliage",
            "ProceduralMeshComponent",
            "GeometryCollectionEngine",
            "ChaosSolverEngine",
            "Chaos"
        });

        // Performance and optimization
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "RenderCore",
            "RHI",
            "ApplicationCore"
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
        else if (Target.Platform == UnrealTargetPlatform.PS5)
        {
            PublicDefinitions.Add("PLATFORM_PS5=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.XSX)
        {
            PublicDefinitions.Add("PLATFORM_XBOX=1");
        }

        // Performance settings
        bUseUnity = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bFasterWithoutUnity = false;
    }
}