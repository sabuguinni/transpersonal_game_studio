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
            "AudioMixer",
            "MetasoundEngine"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        { 
            "RenderCore",
            "RHI",
            "Landscape",
            "Foliage",
            "CinematicCamera"
        });

        // Enable C++20 features for advanced consciousness calculations
        CppStandard = CppStandardVersion.Cpp20;
        
        // Optimization for transpersonal state transitions
        bUseUnity = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bFasterWithoutUnity = false;
    }
}