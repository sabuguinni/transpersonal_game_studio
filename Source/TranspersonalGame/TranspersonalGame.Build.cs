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
            "RenderCore",
            "RHI",
            "Niagara",
            "AudioMixer",
            "AudioExtensions",
            "SignificanceManager",
            "NavigationSystem",
            "AIModule",
            "GameplayTasks",
            "GameplayTags",
            "GameplayAbilities",
            "PhysicsCore",
            "Chaos",
            "ChaosVehicles",
            "GeometryCollectionEngine"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "ApplicationCore",
            "Json",
            "JsonUtilities",
            "HTTP",
            "ImageWrapper",
            "MediaAssets",
            "MediaUtils",
            "MediaPlayerEditor",
            "ToolMenus",
            "EditorStyle",
            "EditorWidgets",
            "UnrealEd",
            "LevelEditor",
            "PropertyEditor",
            "DetailCustomizations",
            "ComponentVisualizers",
            "EngineSettings",
            "DeveloperSettings",
            "CinematicCamera",
            "LevelSequence",
            "MovieScene",
            "MovieSceneTracks",
            "TimeManagement",
            "Sequencer",
            "ProceduralMeshComponent",
            "RuntimeMeshComponent"
        });

        // Enable RTTI for consciousness system reflection
        bUseRTTI = true;
        
        // Enable exceptions for advanced consciousness calculations
        bEnableExceptions = true;

        // Optimization flags for consciousness processing
        PublicDefinitions.Add("CONSCIOUSNESS_SYSTEM_ENABLED=1");
        PublicDefinitions.Add("REALITY_SHIFT_SYSTEM_ENABLED=1");
        PublicDefinitions.Add("TRANSPERSONAL_DEBUG=1");

        // Platform-specific optimizations
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("PLATFORM_SUPPORTS_ADVANCED_CONSCIOUSNESS=1");
        }

        // Development and shipping build configurations
        if (Target.Configuration == UnrealTargetConfiguration.Development || 
            Target.Configuration == UnrealTargetConfiguration.DebugGame)
        {
            PublicDefinitions.Add("CONSCIOUSNESS_DEBUG_ENABLED=1");
            PublicDefinitions.Add("REALITY_SHIFT_DEBUG_ENABLED=1");
        }

        // Enable modern C++ features for consciousness algorithms
        CppStandard = CppStandardVersion.Cpp17;
        
        // Memory optimization for consciousness data structures
        bUseUnityBuild = false;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        
        // Enable parallel compilation for faster builds
        bFasterWithoutUnity = true;
    }
}