// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TechnicalArchitecture.generated.h"

/**
 * Technical Architecture Constants and Rules for Transpersonal Game Studio
 * 
 * This header defines the core technical constraints and architectural decisions
 * that ALL modules and systems must follow. These are non-negotiable rules
 * established by the Engine Architect.
 */

// =============================================================================
// CORE TECHNICAL CONSTANTS
// =============================================================================

namespace TPSArchitecture
{
	// Performance Targets
	static constexpr float TARGET_FRAMERATE_PC = 60.0f;
	static constexpr float TARGET_FRAMERATE_CONSOLE = 30.0f;
	static constexpr int32 TARGET_MEMORY_CONSOLE_MB = 8192;
	static constexpr int32 STREAMING_SPEED_MIN_MBPS = 200;

	// World Scale Limits
	static constexpr float WORLD_PARTITION_CELL_SIZE = 400000.0f; // 4km² cells
	static constexpr float MAX_WORLD_SIZE_KM = 10.0f;
	static constexpr int32 MAX_STREAMING_DISTANCE_M = 2000;

	// Mass Entity Limits
	static constexpr int32 MAX_DINOSAUR_AGENTS = 50000;
	static constexpr int32 MAX_AGENTS_PER_CELL = 2000;
	static constexpr float AGENT_SIMULATION_RADIUS = 1000.0f;

	// Nanite Requirements
	static constexpr int32 MIN_TRIANGLES_FOR_NANITE = 1000;
	static constexpr bool FORCE_NANITE_ON_ALL_MESHES = true;
	static constexpr float NANITE_FALLBACK_ERROR = 0.1f;

	// Lumen Settings
	static constexpr float LUMEN_SCENE_VIEW_DISTANCE = 800.0f;
	static constexpr bool DISABLE_STATIC_LIGHTING = true;
	static constexpr float LUMEN_FINAL_GATHER_QUALITY = 1.0f;

	// Virtual Shadow Maps
	static constexpr int32 VSM_RESOLUTION = 16384;
	static constexpr int32 VSM_MAX_PAGES = 8192;
	static constexpr float VSM_NORMAL_BIAS = 0.5f;
}

// =============================================================================
// MANDATORY ENGINE FEATURES
// =============================================================================

UENUM(BlueprintType)
enum class ETechnicalFeature : uint8
{
	Nanite			UMETA(DisplayName = "Nanite Virtualized Geometry"),
	Lumen			UMETA(DisplayName = "Lumen Global Illumination"),
	VSM				UMETA(DisplayName = "Virtual Shadow Maps"),
	WorldPartition	UMETA(DisplayName = "World Partition"),
	MassEntity		UMETA(DisplayName = "Mass Entity System"),
	ChaosPhysics	UMETA(DisplayName = "Chaos Physics"),
	MetaSounds		UMETA(DisplayName = "MetaSounds Audio"),
	Niagara			UMETA(DisplayName = "Niagara VFX")
};

// =============================================================================
// MODULE REGISTRATION SYSTEM
// =============================================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FModuleInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString ModuleName;

	UPROPERTY(BlueprintReadOnly)
	FString ResponsibleAgent;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Dependencies;

	UPROPERTY(BlueprintReadOnly)
	bool bIsCoreDependency = false;

	UPROPERTY(BlueprintReadOnly)
	int32 LoadPriority = 0;

	FModuleInfo()
	{
		ModuleName = TEXT("");
		ResponsibleAgent = TEXT("");
		bIsCoreDependency = false;
		LoadPriority = 0;
	}
};

// =============================================================================
// ARCHITECTURE ENFORCEMENT SUBSYSTEM
// =============================================================================

UCLASS()
class TRANSPERSONALGAME_API UTechnicalArchitectureSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Module Registration
	UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
	bool RegisterModule(const FModuleInfo& ModuleInfo);

	UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
	bool UnregisterModule(const FString& ModuleName);

	UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
	TArray<FModuleInfo> GetRegisteredModules() const;

	// Feature Validation
	UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
	bool ValidateRequiredFeatures();

	UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
	bool IsFeatureEnabled(ETechnicalFeature Feature) const;

	// Performance Monitoring
	UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
	float GetCurrentFramerate() const;

	UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
	bool IsPerformanceWithinTargets() const;

	// Architecture Rules Enforcement
	UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
	bool ValidateArchitectureCompliance();

protected:
	UPROPERTY()
	TArray<FModuleInfo> RegisteredModules;

	UPROPERTY()
	TMap<ETechnicalFeature, bool> FeatureStatus;

private:
	void InitializeFeatureValidation();
	void EnforceNaniteRequirements();
	void EnforceLumenSettings();
	void EnforceVSMConfiguration();
	void EnforceWorldPartitionSettings();
	
	bool ValidateModuleDependencies() const;
	void LogArchitectureStatus() const;
};

// =============================================================================
// ARCHITECTURE VALIDATION MACROS
// =============================================================================

#define VALIDATE_NANITE_ENABLED() \
	if (!GetDefault<URendererSettings>()->bEnableNanite) \
	{ \
		UE_LOG(LogTemp, Fatal, TEXT("Nanite must be enabled for Transpersonal Game Studio project")); \
	}

#define VALIDATE_LUMEN_ENABLED() \
	if (GetDefault<URendererSettings>()->DynamicGlobalIlluminationMethod != EDynamicGlobalIlluminationMethod::Lumen) \
	{ \
		UE_LOG(LogTemp, Fatal, TEXT("Lumen Global Illumination must be enabled")); \
	}

#define VALIDATE_WORLD_PARTITION() \
	if (!GetWorld() || !GetWorld()->IsPartitionedWorld()) \
	{ \
		UE_LOG(LogTemp, Fatal, TEXT("World Partition must be enabled for all levels")); \
	}

// =============================================================================
// PERFORMANCE PROFILING HELPERS
// =============================================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PerformanceMetrics_533
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float CurrentFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MemoryUsageMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 ActiveNaniteTriangles = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ActiveMassAgents = 0;

	UPROPERTY(BlueprintReadOnly)
	float LumenSceneUpdateTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 VSMPagesUsed = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bWithinPerformanceTargets = false;
};

// =============================================================================
// AGENT COMMUNICATION INTERFACE
// =============================================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UAgentSystemInterface : public UInterface
{
	GENERATED_BODY()
};

class TRANSPERSONALGAME_API IAgentSystemInterface
{
	GENERATED_BODY()

public:
	// Called when the system should initialize according to architecture rules
	UFUNCTION(BlueprintImplementableEvent, Category = "Agent System")
	void InitializeAgentSystem();

	// Called to validate the system follows architecture constraints
	UFUNCTION(BlueprintImplementableEvent, Category = "Agent System")
	bool ValidateSystemCompliance();

	// Get the agent responsible for this system
	UFUNCTION(BlueprintImplementableEvent, Category = "Agent System")
	FString GetResponsibleAgent() const;

	// Get system dependencies
	UFUNCTION(BlueprintImplementableEvent, Category = "Agent System")
	TArray<FString> GetSystemDependencies() const;
};