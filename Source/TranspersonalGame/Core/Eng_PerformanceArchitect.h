#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Eng_PerformanceArchitect.generated.h"

/**
 * Engine Architect - Performance Architecture System
 * Defines and enforces performance standards across all game systems
 * Manages LOD chains, culling systems, and performance monitoring
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_PerformanceArchitect : public AActor
{
    GENERATED_BODY()

public:
    AEng_PerformanceArchitect();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === PERFORMANCE TARGETS ===
    
    /** Target FPS for PC platform */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFPS_PC;

    /** Target FPS for console platform */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFPS_Console;

    /** Maximum allowed frame time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxFrameTimeMS;

    /** Current measured FPS */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float CurrentFPS;

    /** Current frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float CurrentFrameTimeMS;

    // === LOD SYSTEM CONFIGURATION ===

    /** LOD distances for static meshes (3 levels) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    TArray<float> StaticMeshLODDistances;

    /** LOD distances for skeletal meshes (3 levels) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    TArray<float> SkeletalMeshLODDistances;

    /** LOD distances for particle systems (3 levels) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    TArray<float> ParticleLODDistances;

    /** Maximum visible actors before culling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    int32 MaxVisibleActors;

    // === CULLING CONFIGURATION ===

    /** Frustum culling enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling;

    /** Occlusion culling enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling;

    /** Distance culling enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableDistanceCulling;

    /** Maximum draw distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MaxDrawDistance;

    /** Small object culling threshold (world units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float SmallObjectCullingSize;

    // === MEMORY MANAGEMENT ===

    /** Maximum texture memory budget (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Management")
    float MaxTextureMemoryMB;

    /** Maximum mesh memory budget (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Management")
    float MaxMeshMemoryMB;

    /** Maximum audio memory budget (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Management")
    float MaxAudioMemoryMB;

    /** Current texture memory usage (MB) */
    UPROPERTY(BlueprintReadOnly, Category = "Memory Monitoring")
    float CurrentTextureMemoryMB;

    /** Current mesh memory usage (MB) */
    UPROPERTY(BlueprintReadOnly, Category = "Memory Monitoring")
    float CurrentMeshMemoryMB;

    // === PERFORMANCE MONITORING ===

    /** Performance monitoring enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    bool bEnablePerformanceMonitoring;

    /** Performance warning threshold (FPS below this triggers warnings) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float PerformanceWarningThreshold;

    /** Performance critical threshold (FPS below this triggers emergency measures) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float PerformanceCriticalThreshold;

    /** Frame time history for smoothing */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    TArray<float> FrameTimeHistory;

    /** Maximum frame time history samples */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    int32 MaxFrameTimeHistorySamples;

    // === CORE FUNCTIONS ===

    /** Initialize performance architecture system */
    UFUNCTION(BlueprintCallable, Category = "Performance System")
    void InitializePerformanceSystem();

    /** Update performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void UpdatePerformanceMetrics();

    /** Get current performance status */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    EPerformanceStatus GetPerformanceStatus();

    /** Get smoothed frame time */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    float GetSmoothedFrameTime();

    /** Apply LOD settings to actor */
    UFUNCTION(BlueprintCallable, Category = "LOD System")
    void ApplyLODSettings(AActor* Actor, ELODLevel LODLevel);

    /** Set LOD level based on distance */
    UFUNCTION(BlueprintCallable, Category = "LOD System")
    ELODLevel CalculateLODLevel(AActor* Actor, float DistanceToViewer);

    /** Enable/disable culling for actor */
    UFUNCTION(BlueprintCallable, Category = "Culling")
    void SetActorCullingSettings(AActor* Actor, bool bEnableCulling);

    // === OPTIMIZATION FUNCTIONS ===

    /** Optimize performance based on current metrics */
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizePerformance();

    /** Reduce quality settings to improve performance */
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void ReduceQualitySettings();

    /** Restore quality settings when performance improves */
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void RestoreQualitySettings();

    /** Cull distant objects to improve performance */
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void CullDistantObjects();

    // === VALIDATION FUNCTIONS ===

    /** Validate performance architecture settings */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    bool ValidatePerformanceSettings();

    /** Get performance architecture status report */
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FString GetPerformanceStatusReport();

    /** Check if performance targets are being met */
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ArePerformanceTargetsMet();

    // === MEMORY MONITORING ===

    /** Update memory usage statistics */
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void UpdateMemoryUsage();

    /** Get memory usage report */
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    FString GetMemoryUsageReport();

    /** Check if memory budgets are exceeded */
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    bool AreMemoryBudgetsExceeded();

protected:
    /** Setup default performance targets */
    void SetupDefaultPerformanceTargets();

    /** Setup default LOD distances */
    void SetupDefaultLODDistances();

    /** Setup default culling settings */
    void SetupDefaultCullingSettings();

    /** Setup default memory budgets */
    void SetupDefaultMemoryBudgets();

    /** Calculate current FPS from frame time */
    void CalculateCurrentFPS();

    /** Add frame time to history */
    void AddFrameTimeToHistory(float FrameTime);

    /** Check performance thresholds and trigger actions */
    void CheckPerformanceThresholds();

private:
    /** Last frame time for FPS calculation */
    float LastFrameTime;

    /** Performance monitoring timer */
    float PerformanceMonitoringTimer;

    /** Performance monitoring interval */
    float PerformanceMonitoringInterval;

    /** Quality settings backup for restoration */
    TMap<FString, float> QualitySettingsBackup;

    /** Performance optimization active */
    bool bPerformanceOptimizationActive;
};

/**
 * Engine Architect - Performance Component
 * Component that can be attached to actors for individual performance monitoring
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_PerformanceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_PerformanceComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === PERFORMANCE SETTINGS ===

    /** Enable LOD for this actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLOD;

    /** Enable culling for this actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableCulling;

    /** Current LOD level */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    ELODLevel CurrentLODLevel;

    /** Distance to viewer */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DistanceToViewer;

    /** Is currently culled */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsCulled;

    // === FUNCTIONS ===

    /** Update LOD based on distance */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLOD();

    /** Update culling state */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateCulling();

    /** Get distance to player camera */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetDistanceToCamera();

private:
    /** Reference to performance architect */
    UPROPERTY()
    AEng_PerformanceArchitect* PerformanceArchitect;

    /** LOD update timer */
    float LODUpdateTimer;

    /** LOD update interval */
    float LODUpdateInterval;
};