// Copyright Transpersonal Game Studio. All Rights Reserved.
using UnrealBuildTool;

public class TranspersonalGame : ModuleRules
{
	public TranspersonalGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicIncludePaths.AddRange(new string[] {
			System.IO.Path.Combine(ModuleDirectory),
			System.IO.Path.Combine(ModuleDirectory, "AI"),
			System.IO.Path.Combine(ModuleDirectory, "AI/BehaviorTrees"),
			System.IO.Path.Combine(ModuleDirectory, "AI/Combat"),
			System.IO.Path.Combine(ModuleDirectory, "AI/CombatAI"),
			System.IO.Path.Combine(ModuleDirectory, "AI/Components"),
			System.IO.Path.Combine(ModuleDirectory, "AI/Core"),
			System.IO.Path.Combine(ModuleDirectory, "AI/CrowdSimulation"),
			System.IO.Path.Combine(ModuleDirectory, "AI/NPCBehavior"),
			System.IO.Path.Combine(ModuleDirectory, "AI/NPCBehaviorSystem"),
			System.IO.Path.Combine(ModuleDirectory, "Animation"),
			System.IO.Path.Combine(ModuleDirectory, "Animation/Core"),
			System.IO.Path.Combine(ModuleDirectory, "Animation/IK"),
			System.IO.Path.Combine(ModuleDirectory, "Animation/MotionMatching"),
			System.IO.Path.Combine(ModuleDirectory, "Animation/Player"),
			System.IO.Path.Combine(ModuleDirectory, "Architecture"),
			System.IO.Path.Combine(ModuleDirectory, "Audio"),
			System.IO.Path.Combine(ModuleDirectory, "Audio/AudioSystem"),
			System.IO.Path.Combine(ModuleDirectory, "Audio/Core"),
			System.IO.Path.Combine(ModuleDirectory, "Audio/MetaSounds"),
			System.IO.Path.Combine(ModuleDirectory, "Characters"),
			System.IO.Path.Combine(ModuleDirectory, "Characters/Animation"),
			System.IO.Path.Combine(ModuleDirectory, "Characters/CharacterSystem"),
			System.IO.Path.Combine(ModuleDirectory, "Combat"),
			System.IO.Path.Combine(ModuleDirectory, "Core"),
			System.IO.Path.Combine(ModuleDirectory, "Core/Animation"),
			System.IO.Path.Combine(ModuleDirectory, "Core/Architecture"),
			System.IO.Path.Combine(ModuleDirectory, "Core/Audio"),
			System.IO.Path.Combine(ModuleDirectory, "Core/Characters"),
			System.IO.Path.Combine(ModuleDirectory, "Core/CombatAI"),
			System.IO.Path.Combine(ModuleDirectory, "Core/CrowdSimulation"),
			System.IO.Path.Combine(ModuleDirectory, "Core/Environment"),
			System.IO.Path.Combine(ModuleDirectory, "Core/EnvironmentArt"),
			System.IO.Path.Combine(ModuleDirectory, "Core/GameFramework"),
			System.IO.Path.Combine(ModuleDirectory, "Core/Integration"),
			System.IO.Path.Combine(ModuleDirectory, "Core/Lighting"),
			System.IO.Path.Combine(ModuleDirectory, "Core/NPCBehavior"),
			System.IO.Path.Combine(ModuleDirectory, "Core/Narrative"),
			System.IO.Path.Combine(ModuleDirectory, "Core/Performance"),
			System.IO.Path.Combine(ModuleDirectory, "Core/PerformanceCore"),
			System.IO.Path.Combine(ModuleDirectory, "Core/PhysicsCore"),
			System.IO.Path.Combine(ModuleDirectory, "Core/QuestSystem"),
			System.IO.Path.Combine(ModuleDirectory, "Core/VFX"),
			System.IO.Path.Combine(ModuleDirectory, "Core/WorldGeneration"),
			System.IO.Path.Combine(ModuleDirectory, "Crowd"),
			System.IO.Path.Combine(ModuleDirectory, "Crowd/MassFragments"),
			System.IO.Path.Combine(ModuleDirectory, "CrowdSimulation"),
			System.IO.Path.Combine(ModuleDirectory, "CrowdSimulation/MassProcessors"),
			System.IO.Path.Combine(ModuleDirectory, "Environment"),
			System.IO.Path.Combine(ModuleDirectory, "EnvironmentArt"),
			System.IO.Path.Combine(ModuleDirectory, "Integration"),
			System.IO.Path.Combine(ModuleDirectory, "Lighting"),
			System.IO.Path.Combine(ModuleDirectory, "Narrative"),
			System.IO.Path.Combine(ModuleDirectory, "PCG"),
			System.IO.Path.Combine(ModuleDirectory, "PCG/Architecture"),
			System.IO.Path.Combine(ModuleDirectory, "Performance"),
			System.IO.Path.Combine(ModuleDirectory, "Physics"),
			System.IO.Path.Combine(ModuleDirectory, "QA"),
			System.IO.Path.Combine(ModuleDirectory, "QA/Core"),
			System.IO.Path.Combine(ModuleDirectory, "QA/Integration"),
			System.IO.Path.Combine(ModuleDirectory, "QA/Tests"),
			System.IO.Path.Combine(ModuleDirectory, "Quest"),
			System.IO.Path.Combine(ModuleDirectory, "VFX"),
			System.IO.Path.Combine(ModuleDirectory, "VFX/Core"),
			System.IO.Path.Combine(ModuleDirectory, "World"),
			System.IO.Path.Combine(ModuleDirectory, "WorldGeneration")
		});

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
			"Niagara",
			"AIModule",
			"GameplayTasks",
			"NavigationSystem", "PoseSearch", "AudioGameplayVolume", "AudioGameplay",
			"MassEntity",
			"MassCommon",
			"MassMovement",
			"MassSpawner",
			"MassActors",
			"MassSimulation",
			"MassRepresentation",
			"MassLOD",
			"MassSignals",
			"MassSmartObjects",
			"MassReplication",
			"StructUtils",
			"StateTreeModule",
			"SmartObjectsModule",
			"PCG",
			"GeometryCollectionEngine",
			"PhysicsCore", "ChaosVehicles", "ProceduralMeshComponent", "Water", "Foliage", "Landscape", "MetasoundGraphCore", "MetasoundFrontend", "MetasoundEngine", "IKRig", "RigVM", "ControlRig", "GameplayTags", "GameplayAbilities", "MetaHumanSDKRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[] 
		{
			"RenderCore",
			"RHI",
			"ApplicationCore",
			"Json",
			"JsonUtilities",
			"HTTP",
			"NetCore",
			"Sockets",
			"AssetRegistry",
			"DeveloperSettings",
			"AnimationCore",
			"MovieScene",
			"LevelSequence",
			"TimeManagement", "AutomationController"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"EditorSubsystem",
				"ToolMenus",
				"EditorWidgets",
				"PropertyEditor",
				"DetailCustomizations",
				"Kismet",
				"KismetCompiler",
				"BlueprintGraph",
				"AnimGraph",
				"AnimGraphRuntime"
			});
		}

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicDefinitions.Add("PLATFORM_SUPPORTS_HARDWARE_RT=1");
		}

		PublicDefinitions.AddRange(new string[]
		{
			"TRANSPERSONAL_PERFORMANCE_TRACKING=1",
			"TRANSPERSONAL_MASS_AI_ENABLED=1"
		});

		CppStandard = CppStandardVersion.Cpp20;
		bUseRTTI = false;
		bUseUnity = true;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
	}
}
