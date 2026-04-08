// Copyright Transpersonal Game Studio. All Rights Reserved.
// EngineArchitecture.h - Core engine architecture definitions and rules

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MODULE_MAPPING.h"
#include "EngineArchitecture.generated.h"

/**
 * TRANSPERSONAL GAME ENGINE ARCHITECTURE
 * 
 * This header defines the core architectural principles and systems
 * that ALL agents must follow. These are the laws of the engine.
 * 
 * ESTABLISHED BY: Agent #02 - Engine Architect
 * VERSION: 1.0 - Consolidation Phase
 */

// ============================================================================
// CORE ARCHITECTURAL PRINCIPLES
// ============================================================================

/**
 * PRINCIPLE 1: WORLD PARTITION MANDATORY
 * All worlds > 4km² MUST use World Partition with these settings:
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTranspersonalWorldPartitionSettings
{
    GENERATED_BODY()

    // Cell size for streaming (meters)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Partition")
    int32 CellSize = 51200; // 512m x 512m cells

    // Loading range around player (meters)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Partition")
    float LoadingRange = 1536.0f; // 3x cell size

    // Enable HLOD for distant geometry
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Partition")
    bool bEnableHLOD = true;

    // Maximum HLOD distance
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Partition")
    float MaxHLODDistance = 10000.0f; // 10km
};

/**
 * PRINCIPLE 2: NANITE MANDATORY
 * All static geometry > 1000 triangles MUST use Nanite
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTranspersonalNaniteSettings
{
    GENERATED_BODY()

    // Minimum triangle count for Nanite enablement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nanite")
    int32 MinTriangleCount = 1000;

    // Enable Nanite Foliage for vegetation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nanite")
    bool bEnableNaniteFoliage = true;

    // Enable Nanite Assemblies for complex objects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nanite")
    bool bEnableNaniteAssemblies = true;

    // Enable Nanite Voxelization for aggregate geometry
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nanite")
    bool bEnableNaniteVoxelization = true;
};

/**
 * PRINCIPLE 3: LUMEN MANDATORY
 * All lighting MUST use Lumen Global Illumination
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTranspersonalLumenSettings
{
    GENERATED_BODY()

    // Enable Lumen Global Illumination
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lumen")
    bool bEnableLumenGI = true;

    // Enable Lumen Reflections
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lumen")
    bool bEnableLumenReflections = true;

    // Lumen scene view distance (meters)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lumen")
    float LumenSceneViewDistance = 800.0f;

    // Enable Hardware Ray Tracing if supported
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lumen")
    bool bEnableHardwareRayTracing = true;
};

/**
 * PRINCIPLE 4: VIRTUAL SHADOW MAPS MANDATORY
 * All shadows MUST use Virtual Shadow Maps
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTranspersonalVSMSettings
{
    GENERATED_BODY()

    // Enable Virtual Shadow Maps
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VSM")
    bool bEnableVirtualShadowMaps = true;

    // Virtual shadow map resolution
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VSM")
    int32 VirtualShadowMapResolution = 16384; // 16K

    // Enable caching for performance
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VSM")
    bool bEnableCaching = true;

    // Enable soft shadows with SMRT
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VSM")
    bool bEnableSoftShadows = true;
};

/**
 * PRINCIPLE 5: MASS AI MANDATORY
 * All crowd simulation MUST use Mass Entity framework
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTranspersonalMassAISettings
{
    GENERATED_BODY()

    // Maximum concurrent entities
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass AI")
    int32 MaxConcurrentEntities = 50000;

    // LOD distances for entity processing
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass AI")
    TArray<float> LODDistances = {100.0f, 500.0f, 1000.0f, 5000.0f};

    // Enable Mass Movement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass AI")
    bool bEnableMassMovement = true;

    // Enable Mass Spawning
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass AI")
    bool bEnableMassSpawning = true;
};

// ============================================================================
// PERFORMANCE TARGETS (NON-NEGOTIABLE)
// ============================================================================

/**
 * PERFORMANCE LAW: These targets are MANDATORY
 * Any system that cannot meet these targets is REJECTED
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTranspersonalPerformanceTargets
{
    GENERATED_BODY()

    // Target framerate on PC (minimum)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TargetFPS_PC = 60;

    // Target framerate on console (minimum)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TargetFPS_Console = 30;

    // Maximum memory usage (MB)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxMemoryUsage_MB = 8192; // 8GB

    // Maximum GPU time per frame (ms)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MaxGPUTime_MS = 16.67f; // 60fps = 16.67ms

    // Maximum CPU time per frame (ms)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MaxCPUTime_MS = 16.67f; // 60fps = 16.67ms
};

// ============================================================================
// SYSTEM INTEGRATION RULES
// ============================================================================

/**
 * INTEGRATION LAW: Module Communication Rules
 * All inter-module communication MUST follow these patterns
 */
namespace TranspersonalGame
{
    namespace ArchitecturalRules
    {
        // Rule 1: No direct dependencies between gameplay modules
        // Use interfaces and subsystems only
        
        // Rule 2: All core systems communicate through UGameInstanceSubsystem
        
        // Rule 3: Physics systems have priority over all other systems
        
        // Rule 4: Performance monitoring is mandatory in all systems
        
        // Rule 5: All systems must support hot-reloading
    }
}

/**
 * TRANSPERSONAL ENGINE ARCHITECTURE SUBSYSTEM
 * 
 * This subsystem enforces architectural rules and monitors compliance
 */
UCLASS()
class TRANSPERSONALGAME_API UTranspersonalEngineArchitectureSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateWorldPartitionSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateNaniteSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateLumenSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateVSMSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateMassAISettings() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FTranspersonalPerformanceTargets GetCurrentPerformanceMetrics() const;

    // System status
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsSystemCompliant(const FString& SystemName) const;

protected:
    // Architecture settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    FTranspersonalWorldPartitionSettings WorldPartitionSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    FTranspersonalNaniteSettings NaniteSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    FTranspersonalLumenSettings LumenSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    FTranspersonalVSMSettings VSMSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    FTranspersonalMassAISettings MassAISettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    FTranspersonalPerformanceTargets PerformanceTargets;

private:
    // Internal validation functions
    void ValidateEngineSettings();
    void SetupPerformanceMonitoring();
    void EnforceArchitecturalRules();

    // Performance tracking
    mutable float LastFrameTime;
    mutable float LastGPUTime;
    mutable int32 LastMemoryUsage;
};

// ============================================================================
// AGENT COMPLIANCE MACROS
// ============================================================================

/**
 * Use these macros to ensure your systems comply with architecture
 */
#define TRANSPERSONAL_VALIDATE_ARCHITECTURE() \
    if (UTranspersonalEngineArchitectureSubsystem* ArchSubsystem = GetGameInstance()->GetSubsystem<UTranspersonalEngineArchitectureSubsystem>()) \
    { \
        ensureAlways(ArchSubsystem->IsSystemCompliant(GetClass()->GetName())); \
    }

#define TRANSPERSONAL_CHECK_PERFORMANCE() \
    if (UTranspersonalEngineArchitectureSubsystem* ArchSubsystem = GetGameInstance()->GetSubsystem<UTranspersonalEngineArchitectureSubsystem>()) \
    { \
        if (!ArchSubsystem->IsPerformanceTargetMet()) \
        { \
            UE_LOG(LogTemp, Warning, TEXT("Performance target not met in %s"), *GetClass()->GetName()); \
        } \
    }

// ============================================================================
// FORWARD DECLARATIONS FOR CORE SYSTEMS
// ============================================================================

class UPhysicsSystemManager;
class UPerformanceOptimizer;
class UWorldPartitionManager;
class UMassAIController;

/**
 * END OF ENGINE ARCHITECTURE
 * 
 * This architecture is MANDATORY and NON-NEGOTIABLE.
 * All agents must comply with these rules.
 * 
 * Violations will be rejected by the QA agent.
 */