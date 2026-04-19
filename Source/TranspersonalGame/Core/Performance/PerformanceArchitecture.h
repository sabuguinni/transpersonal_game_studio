#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Stats/Stats.h"
#include "../../SharedTypes.h"
#include "PerformanceArchitecture.generated.h"

// Performance monitoring stats
DECLARE_STATS_GROUP(TEXT("TranspersonalGame_Performance"), STATGROUP_TranspersonalPerf, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Frame Time"), STAT_FrameTime, STATGROUP_TranspersonalPerf);
DECLARE_CYCLE_STAT(TEXT("AI Update"), STAT_AIUpdate, STATGROUP_TranspersonalPerf);
DECLARE_CYCLE_STAT(TEXT("World Generation"), STAT_WorldGen, STATGROUP_TranspersonalPerf);
DECLARE_CYCLE_STAT(TEXT("Crowd Simulation"), STAT_CrowdSim, STATGROUP_TranspersonalPerf);

/**
 * Performance Budget Tracker
 * Tracks CPU and memory usage per system to enforce architectural limits
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUBudgetMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryBudgetMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentCPUUsageMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bIsOverBudget;

    FEng_PerformanceBudget()
    {
        SystemName = TEXT("Unknown");
        CPUBudgetMS = 1.0f;
        MemoryBudgetMB = 100.0f;
        CurrentCPUUsageMS = 0.0f;
        CurrentMemoryUsageMB = 0.0f;
        bIsOverBudget = false;
    }
};

/**
 * LOD Configuration for different performance targets
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_LODConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<float> LODDistances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxLODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bUseAggressiveCulling;

    FEng_LODConfiguration()
    {
        LODDistances = {1000.0f, 2500.0f, 5000.0f};
        MaxLODLevel = 3;
        CullingDistance = 10000.0f;
        bUseAggressiveCulling = false;
    }
};

/**
 * Core Performance Architecture Manager
 * Enforces performance budgets and optimizations across all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Performance Budget Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterSystemBudget(const FString& SystemName, float CPUBudgetMS, float MemoryBudgetMB);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateSystemUsage(const FString& SystemName, float CPUUsageMS, float MemoryUsageMB);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsSystemOverBudget(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceBudget GetSystemBudget(const FString& SystemName) const;

    // Performance Target Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EEng_PerformanceTarget NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EEng_PerformanceTarget GetCurrentPerformanceTarget() const { return CurrentTarget; }

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_LODConfiguration GetLODConfigForTarget(EEng_PerformanceTarget Target) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODConfiguration(const FEng_LODConfiguration& Config);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const { return CurrentFrameTimeMS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameTime() const { return AverageFrameTimeMS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsage() const { return CurrentMemoryUsageMB; }

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForTarget();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMode(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetOverBudgetSystems() const;

protected:
    // Performance Budgets
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FEng_PerformanceBudget> SystemBudgets;

    // Current Performance State
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EEng_PerformanceTarget CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CurrentFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float AverageFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CurrentMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFrameTimeMS;

    // LOD Configurations
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    TMap<EEng_PerformanceTarget, FEng_LODConfiguration> LODConfigurations;

    // Performance Settings
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bPerformanceModeEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bMonitoringEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MonitoringInterval;

private:
    // Frame time tracking
    TArray<float> FrameTimeHistory;
    int32 FrameTimeHistoryIndex;
    static const int32 FrameTimeHistorySize = 60; // 1 second at 60fps

    // Performance monitoring
    float LastMonitoringTime;
    
    void InitializePerformanceBudgets();
    void InitializeLODConfigurations();
    void UpdateFrameTimeTracking(float DeltaTime);
    void MonitorPerformance();
    void ApplyPerformanceOptimizations();
    bool ShouldOptimizeForPerformance() const;
};

/**
 * World-specific performance manager
 * Handles per-world performance optimizations and culling
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UWorldPerformanceManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UWorldPerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void PostInitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // World Performance Management
    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void OptimizeWorldForPerformance();

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void SetCullingDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void UpdateActorLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void CullDistantActors();

    // Actor Performance Management
    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void RegisterPerformanceActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void UnregisterPerformanceActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    int32 GetActiveActorCount() const { return ManagedActors.Num(); }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "World Performance", meta = (AllowPrivateAccess = "true"))
    TArray<TWeakObjectPtr<AActor>> ManagedActors;

    UPROPERTY(BlueprintReadOnly, Category = "World Performance", meta = (AllowPrivateAccess = "true"))
    float CullingDistance;

    UPROPERTY(BlueprintReadOnly, Category = "World Performance", meta = (AllowPrivateAccess = "true"))
    float LastCullingUpdate;

    UPROPERTY(BlueprintReadOnly, Category = "World Performance", meta = (AllowPrivateAccess = "true"))
    float CullingUpdateInterval;

private:
    void InitializeWorldPerformance();
    void UpdateManagedActors(float DeltaTime);
    void CleanupInvalidActors();
    float GetDistanceToPlayer(AActor* Actor) const;
};