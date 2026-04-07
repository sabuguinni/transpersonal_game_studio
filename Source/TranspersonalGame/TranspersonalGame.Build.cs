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
            "GameplayTags",
            "GameplayAbilities",
            "Niagara",
            "ProceduralMeshComponent",
            "Landscape",
            "Foliage"
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
            "AnimGraph"
        });

        // Enable C++17 features for advanced consciousness simulation
        CppStandard = CppStandardVersion.Cpp17;
        
        // Optimization for procedural generation
        bUseUnity = true;
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bFasterWithoutUnity = false;
    }
}