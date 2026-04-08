// Copyright Transpersonal Game Studio 2026

using UnrealBuildTool;

public class TranspersonalGame : ModuleRules
{
    public TranspersonalGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Core dependencies for survival game
        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore",
            "HeadMountedDisplay",
            "EnhancedInput"
        });

        // AI and behavior systems for dinosaur ecosystems
        PrivateDependencyModuleNames.AddRange(new string[] 
        { 
            "AIModule",
            "GameplayTasks",
            "NavigationSystem",
            "MassEntity",
            "MassMovement",
            "MassSpawner",
            "MassCommon",
            "MassActors",
            "MassRepresentation",
            "MassLOD",
            "MassSimulation",
            "StructUtils"
        });

        // UE5 specific features
        if (Target.Version.MajorVersion >= 5)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "Niagara",
                "MetasoundEngine",
                "ProceduralMeshComponent",
                "Landscape",
                "Foliage",
                "PCG" // Procedural Content Generation
            });
        }

        // Physics and destruction for dynamic world
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "PhysicsCore",
            "Chaos",
            "ChaosVehicles",
            "GeometryCollectionEngine"
        });

        // Networking for potential multiplayer
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "NetCore",
            "ReplicationGraph"
        });

        // Development and debugging tools
        if (Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "ToolMenus",
                "EditorStyle",
                "EditorWidgets",
                "GraphEditor",
                "Kismet",
                "PropertyEditor",
                "SlateCore",
                "Slate",
                "ToolWidgets",
                "WorkspaceMenuStructure"
            });
        }
    }
}