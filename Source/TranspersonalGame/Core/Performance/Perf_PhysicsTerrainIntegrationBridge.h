#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Perf_PhysicsTerrainIntegrationBridge.generated.h"

class UPerf_TerrainPhysicsPerformanceMonitor;
class UCore_PhysicsTerrainIntegrator;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_IntegrationBridgeMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float PhysicsToTerrainSyncTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float TerrainToPhysicsSyncTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActiveIntegrationPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 PendingUpdates;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationHealthy;

    FPerf_IntegrationBridgeMetrics()
        : PhysicsToTerrainSyncTime(0.0f)
        , TerrainToPhysicsSyncTime(0.0f)
        , ActiveIntegrationPoints(0)
        , PendingUpdates(0)
        , bIntegrationHealthy(true)
    {
    }
};

UENUM(BlueprintType)
enum class EPerf_IntegrationMode : uint8
{
    Realtime UMETA(DisplayName = "Realtime"),
    Batched UMETA(DisplayName = "Batched"),
    Deferred UMETA(DisplayName = "Deferred"),
    Minimal UMETA(DisplayName = "Minimal")
};

/**
 * Bridges performance monitoring between physics and terrain systems
 * Coordinates optimization between Core_PhysicsTerrainIntegrator and Perf_TerrainPhysicsPerformanceMonitor
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsTerrainIntegrationBridge : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PhysicsTerrainIntegrationBridge();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterTerrainIntegrator(UCore_PhysicsTerrainIntegrator* Integrator);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterPerformanceMonitor(UPerf_TerrainPhysicsPerformanceMonitor* Monitor);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UnregisterTerrainIntegrator(UCore_PhysicsTerrainIntegrator* Integrator);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UnregisterPerformanceMonitor(UPerf_TerrainPhysicsPerformanceMonitor* Monitor);

    // Performance coordination
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void NotifyPhysicsUpdate(float UpdateTime, int32 UpdateCount);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void NotifyTerrainDeformation(float DeformationTime, int32 DeformationPoints);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void NotifyIntegrationSync(float SyncTime, bool bPhysicsToTerrain);

    // Integration mode control
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SetIntegrationMode(EPerf_IntegrationMode Mode);

    UFUNCTION(BlueprintPure, Category = "Integration")
    EPerf_IntegrationMode GetCurrentIntegrationMode() const { return CurrentIntegrationMode; }

    // Metrics access
    UFUNCTION(BlueprintPure, Category = "Integration")
    FPerf_IntegrationBridgeMetrics GetIntegrationMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    bool IsIntegrationHealthy() const { return CurrentMetrics.bIntegrationHealthy; }

    // Optimization triggers
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void TriggerIntegrationOptimization();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void EnableAdaptiveIntegration(bool bEnable) { bAdaptiveIntegrationEnabled = bEnable; }

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void RunIntegrationBenchmark();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogIntegrationState();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void SetDebugVisualization(bool bEnable) { bDebugVisualizationEnabled = bEnable; }

protected:
    // Registered components
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<TWeakObjectPtr<UCore_PhysicsTerrainIntegrator>> RegisteredIntegrators;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<TWeakObjectPtr<UPerf_TerrainPhysicsPerformanceMonitor>> RegisteredMonitors;

    // Integration state
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FPerf_IntegrationBridgeMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EPerf_IntegrationMode CurrentIntegrationMode;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAdaptiveIntegrationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float IntegrationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxPendingUpdates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PerformanceThreshold;

    // Debug settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugVisualizationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bLogIntegrationEvents;

private:
    // Internal state
    FTimerHandle IntegrationUpdateTimer;
    float LastOptimizationTime;
    float OptimizationCooldown;

    // Performance tracking
    TArray<float> RecentSyncTimes;
    int32 SyncTimeHistoryIndex;
    static const int32 SyncTimeHistorySize = 30;

    // Internal methods
    void UpdateIntegrationMetrics();
    void CheckIntegrationHealth();
    void OptimizeIntegrationMode();
    void ApplyIntegrationMode(EPerf_IntegrationMode Mode);
    void ProcessPendingUpdates();
    void CleanupInvalidReferences();
    void NotifyMonitorsOfOptimization();
    void NotifyIntegratorsOfModeChange();
};