// Copyright Epic Games, Inc. All Rights Reserved.

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
			"Niagara",
			"AIModule",
			"GameplayTasks",
			"NavigationSystem",
			"MassEntity",
			"MassCommon",
			"MassMovement",
			"MassSpawner",
			"MassAI",
			"MassGameplay",
			"MassReplication",
			"MassLOD",
			"MassRepresentation",
			"StructUtils",
			"StateTree",
			"SmartObjects",
			"WorldPartition",
			"PCG",
			"GeometryCollectionEngine",
			"ChaosVehicles",
			"PhysicsCore",
			"Chaos"
		});

		PrivateDependencyModuleNames.AddRange(new string[] 
		{
			"RenderCore",
			"RHI",
			"ApplicationCore",
			"Json",
			"JsonObjectConverter",
			"HTTP",
			"NetCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Sockets",
			"AssetRegistry",
			"DeveloperSettings",
			"ToolMenus",
			"EditorStyle",
			"EditorWidgets",
			"UnrealEd",
			"PropertyEditor",
			"Kismet",
			"KismetCompiler",
			"BlueprintGraph",
			"AnimGraph",
			"AnimationCore",
			"AnimGraphRuntime",
			"MovieScene",
			"LevelSequence",
			"TimeManagement"
		});

		// Conditional dependencies for editor builds
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"EditorSubsystem",
				"UnrealEd",
				"ToolMenus",
				"EditorWidgets",
				"Slate",
				"SlateCore",
				"PropertyEditor",
				"DetailCustomizations"
			});
		}

		// Platform-specific optimizations
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicDefinitions.Add("PLATFORM_SUPPORTS_HARDWARE_RT=1");
		}

		// Performance optimization flags
		PublicDefinitions.AddRange(new string[]
		{
			"UE_BUILD_SHIPPING_WITH_EDITOR=0",
			"TRANSPERSONAL_PERFORMANCE_TRACKING=1",
			"TRANSPERSONAL_MASS_AI_ENABLED=1"
		});

		// Enable modern C++ features
		CppStandard = CppStandardVersion.Cpp20;
		bUseRTTI = false;
		bUseUnity = true;
		
		// Optimization settings
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
		
		// Memory management
		bEnableBufferSecurityChecks = true;
		bEnableUndefinedIdentifierWarnings = true;
	}
}