using UnrealBuildTool;

public class TranspersonalGame : ModuleRules
{
    public TranspersonalGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    
        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore",
            "UMG",
            "Slate",
            "SlateCore",
            "EnhancedInput",
            "GameplayTags",
            "GameplayTasks",
            "AIModule",
            "NavigationSystem",
            "Niagara",
            "AudioMixer",
            "MetasoundEngine"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { 
            "GameplayAbilities",
            "GameplayMessageRuntime",
            "StructUtils",
            "TypedElementFramework",
            "CommonUI",
            "CommonInput",
            "CommonGame"
        });

        // Consciousness and Transpersonal Systems
        PublicIncludePaths.AddRange(new string[] {
            "TranspersonalGame/Core",
            "TranspersonalGame/Consciousness",
            "TranspersonalGame/Narrative",
            "TranspersonalGame/Audio",
            "TranspersonalGame/UI"
        });
    }
}