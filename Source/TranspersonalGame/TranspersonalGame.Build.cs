// TranspersonalGame.Build.cs
// Core Systems Programmer — Agent #03
// Updated: Cycle AUTO_20260701_005
// Adds EnhancedInput, NavigationSystem, AIModule, GameplayTasks for full character + AI support

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
            "NavigationSystem",
            "AIModule",
            "GameplayTasks",
            "UMG",
            "Slate",
            "SlateCore",
            "PhysicsCore",
            "Chaos",
            "GeometryCore",
            "PCG",
            "ProceduralMeshComponent"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "RenderCore",
            "RHI"
        });

        // Enable IWYU
        bEnforceIWYU = true;
    }
}
