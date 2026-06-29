// Copyright Transpersonal Game Studio. All Rights Reserved.
// TranspersonalGame.Build.cs — verified by Performance Optimizer (Agent #4)

using UnrealBuildTool;

public class TranspersonalGame : ModuleRules
{
    public TranspersonalGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            // Core engine
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",

            // Enhanced Input (UE5.1+)
            "EnhancedInput",

            // Character movement + physics
            "PhysicsCore",
            "Chaos",

            // AI & Navigation (DinosaurBase, NPC behavior)
            "AIModule",
            "NavigationSystem",
            "GameplayTasks",

            // Gameplay framework
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",

            // Procedural Content Generation
            "PCG",

            // Rendering / Lumen
            "RenderCore",
            "Renderer",

            // Slate UI (HUD)
            "Slate",
            "SlateCore",
            "UMG",

            // Online / session (future multiplayer)
            "OnlineSubsystem",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            // Niagara VFX
            "Niagara",

            // Audio
            "AudioMixer",
            "MetasoundEngine",

            // Landscape
            "Landscape",
            "LandscapeEditor",

            // Foliage
            "Foliage",

            // World Partition / streaming
            "WorldPartitionEditor",

            // Mass AI (crowd simulation)
            "MassAI",
            "MassEntity",
            "MassGameplay",

            // Chaos physics extras
            "GeometryCollectionEngine",
            "ChaosSolverEngine",
        });

        // Needed for UE5 editor tools
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "LevelEditor",
                "PropertyEditor",
            });
        }

        // Optimization: enable unity builds for faster compile
        bUseUnity = true;

        // Enable IWYU for cleaner includes
        bEnforceIWYU = true;
    }
}
