#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeInfo.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Perf_TerrainDeformationOptimizer.generated.h"

// Forward declarations
class UCore_TerrainDeformationSystem;
class ALandscape;

UENUM(BlueprintType)
enum class EPerf_TerrainDeformationQuality : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainDeformationBudget
{
    GENERATED_BODY()

    // Maximum number of active deformations per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDeformationsPerFrame = 5;

    // Maximum number of simultaneous deformations in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousDeformations = 200;

    // Distance at which deformations are culled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 5000.0f;

    // Memory budget for deformation data in MB
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryBudgetMB = 50.0f;

    // Time budget for deformation processing per frame in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TimeBudgetMS = 2.0f;

    FPerf_TerrainDeformationBudget()
    {
        MaxDeformationsPerFrame = 5;
        MaxSimultaneousDeformations = 200;
        CullingDistance = 5000.0f;
        MemoryBudgetMB = 50.0f;
        TimeBudgetMS = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainDeformationMetrics
{
    GENERATED_BODY()

    // Current number of active deformations
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActiveDeformations = 0;

    // Current memory usage in MB
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentMemoryUsageMB = 0.0f;

    // Current frame time for deformation processing in MS
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFrameTimeMS = 0.0f;

    // Number of deformations culled this frame
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CulledDeformationsThisFrame = 0;

    // Performance efficiency ratio (0-1)
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PerformanceEfficiency = 1.0f;

    FPerf_TerrainDeformationMetrics()
    {
        ActiveDeformations = 0;
        CurrentMemoryUsageMB = 0.0f;
        CurrentFrameTimeMS = 0.0f;
        CulledDeformationsThisFrame = 0;
        PerformanceEfficiency = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainLODSettings
{
    GENERATED_BODY()

    // Distance thresholds for different LOD levels
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance = 6000.0f;

    // Resolution multipliers for each LOD level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0ResolutionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1ResolutionMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2ResolutionMultiplier = 0.25f;

    FPerf_TerrainLODSettings()
    {
        LOD0Distance = 1000.0f;
        LOD1Distance = 3000.0f;
        LOD2Distance = 6000.0f;
        LOD0ResolutionMultiplier = 1.0f;
        LOD1ResolutionMultiplier = 0.5f;
        LOD2ResolutionMultiplier = 0.25f;
    }
};

/**
 * Performance optimizer for terrain deformation systems
 * Manages LOD, culling, memory budgets, and frame time budgets for terrain modifications
 * Ensures 60fps target is maintained even with extensive terrain deformation
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_TerrainDeformationOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_TerrainDeformationOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_TerrainDeformationQuality QualityLevel = EPerf_TerrainDeformationQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_TerrainDeformationBudget PerformanceBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_TerrainLODSettings LODSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_TerrainDeformationMetrics CurrentMetrics;

    // Optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAutomaticQualityAdjustment = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableLODOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableMemoryPooling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TargetFrameRate = 60.0f;

    // Performance monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float PerformanceCheckInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bEnableDetailedProfiling = false;

    // Core optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainDeformations();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityLevel(EPerf_TerrainDeformationQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceBudget(const FPerf_TerrainDeformationBudget& NewBudget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantDeformations(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetOptimalLODLevel(float Distance) const;

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void CleanupInactiveDeformations();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    float GetCurrentMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool IsWithinMemoryBudget() const;

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void AdjustQualityForPerformance();

    // Integration with terrain systems
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterTerrainDeformationSystem(UCore_TerrainDeformationSystem* System);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UnregisterTerrainDeformationSystem(UCore_TerrainDeformationSystem* System);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void OptimizeLandscapeComponents(ALandscape* Landscape);

    // Debug and profiling
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void PrintPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void ResetPerformanceCounters();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void SetDebugVisualization(bool bEnabled);

private:
    // Internal tracking
    UPROPERTY()
    TArray<TWeakObjectPtr<UCore_TerrainDeformationSystem>> RegisteredSystems;

    UPROPERTY()
    TArray<TWeakObjectPtr<ALandscape>> ManagedLandscapes;

    // Performance tracking
    float LastPerformanceCheck = 0.0f;
    float AccumulatedFrameTime = 0.0f;
    int32 FrameTimesamples = 0;

    // Optimization state
    bool bCurrentlyOptimizing = false;
    float LastOptimizationTime = 0.0f;

    // Internal optimization functions
    void PerformDistanceCulling();
    void UpdateLODLevels();
    void ManageMemoryUsage();
    void CheckPerformanceThresholds();
    void ApplyQualitySettings();
    
    // Utility functions
    float CalculateDistanceToViewer(const FVector& Location) const;
    void UpdateBudgetBasedOnQuality();
    void LogPerformanceWarning(const FString& Warning);
};

/**
 * Global subsystem for managing terrain deformation performance across the entire game world
 */
UCLASS()
class TRANSPERSONALGAME_API UPerf_TerrainDeformationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Global performance management
    UFUNCTION(BlueprintCallable, Category = "Global Performance")
    void SetGlobalTerrainQuality(EPerf_TerrainDeformationQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Global Performance")
    void OptimizeAllTerrainSystems();

    UFUNCTION(BlueprintCallable, Category = "Global Performance")
    FPerf_TerrainDeformationMetrics GetGlobalPerformanceMetrics() const;

    // System registration
    void RegisterOptimizer(UPerf_TerrainDeformationOptimizer* Optimizer);
    void UnregisterOptimizer(UPerf_TerrainDeformationOptimizer* Optimizer);

private:
    UPROPERTY()
    TArray<TWeakObjectPtr<UPerf_TerrainDeformationOptimizer>> RegisteredOptimizers;

    EPerf_TerrainDeformationQuality GlobalQualityLevel = EPerf_TerrainDeformationQuality::High;
};