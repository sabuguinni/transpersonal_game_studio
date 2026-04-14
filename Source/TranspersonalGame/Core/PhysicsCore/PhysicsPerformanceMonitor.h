#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "PhysicsPerformanceMonitor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsPerformanceAlert, ECore_PhysicsPerformanceLevel, AlertLevel, FString, AlertMessage);

/**
 * Performance metrics for physics simulation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    /** Current physics frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime;

    /** Number of active rigid bodies */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRigidBodies;

    /** Number of active constraints */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveConstraints;

    /** Number of collision pairs processed */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionPairs;

    /** Memory usage by physics simulation in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsMemoryUsage;

    /** Average FPS over last second */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    /** Current performance level */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    ECore_PhysicsPerformanceLevel PerformanceLevel;

    /** Timestamp when metrics were captured */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float Timestamp;

    FCore_PhysicsPerformanceMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActiveRigidBodies = 0;
        ActiveConstraints = 0;
        CollisionPairs = 0;
        PhysicsMemoryUsage = 0.0f;
        AverageFPS = 60.0f;
        PerformanceLevel = ECore_PhysicsPerformanceLevel::Optimal;
        Timestamp = 0.0f;
    }
};

/**
 * Performance threshold configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsPerformanceThresholds
{
    GENERATED_BODY()

    /** Maximum acceptable physics frame time (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxPhysicsFrameTime;

    /** Maximum number of rigid bodies before warning */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxRigidBodies;

    /** Maximum memory usage (MB) before warning */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxMemoryUsage;

    /** Minimum acceptable FPS */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MinFPS;

    /** Time window for FPS averaging (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float FPSAveragingWindow;

    FCore_PhysicsPerformanceThresholds()
    {
        MaxPhysicsFrameTime = 16.67f; // 60 FPS target
        MaxRigidBodies = 1000;
        MaxMemoryUsage = 512.0f; // 512 MB
        MinFPS = 30.0f;
        FPSAveragingWindow = 1.0f;
    }
};

/**
 * Real-time physics performance monitoring system
 * Tracks physics simulation performance and alerts when thresholds are exceeded
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsPerformanceMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPhysicsPerformanceMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Monitoring control
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void PauseMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ResumeMonitoring();

    UFUNCTION(BlueprintPure, Category = "Physics Performance")
    bool IsMonitoring() const { return bIsMonitoring; }

    // Performance data access
    UFUNCTION(BlueprintPure, Category = "Physics Performance")
    FCore_PhysicsPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Physics Performance")
    TArray<FCore_PhysicsPerformanceMetrics> GetMetricsHistory() const { return MetricsHistory; }

    UFUNCTION(BlueprintPure, Category = "Physics Performance")
    ECore_PhysicsPerformanceLevel GetCurrentPerformanceLevel() const { return CurrentMetrics.PerformanceLevel; }

    // Threshold configuration
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPerformanceThresholds(const FCore_PhysicsPerformanceThresholds& NewThresholds);

    UFUNCTION(BlueprintPure, Category = "Physics Performance")
    FCore_PhysicsPerformanceThresholds GetPerformanceThresholds() const { return PerformanceThresholds; }

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetAveragePhysicsFrameTime(float TimeWindow = 5.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetPeakPhysicsFrameTime(float TimeWindow = 5.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    int32 GetMaxRigidBodyCount(float TimeWindow = 5.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void GeneratePerformanceReport(FString& ReportText) const;

    // Alert system
    UPROPERTY(BlueprintAssignable, Category = "Physics Performance")
    FOnPhysicsPerformanceAlert OnPerformanceAlert;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetAlertThreshold(ECore_PhysicsPerformanceLevel Level, bool bEnabled);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Physics Performance", CallInEditor = true)
    void DrawPerformanceGraph() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void LogCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ResetMetricsHistory();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MonitoringFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxHistorySize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoOptimizeOnAlert;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bLogPerformanceWarnings;

protected:
    // Core monitoring
    void UpdatePerformanceMetrics();
    void AnalyzePerformanceLevel();
    void CheckPerformanceThresholds();
    void TriggerPerformanceAlert(ECore_PhysicsPerformanceLevel Level, const FString& Message);

    // Data collection
    void CollectPhysicsStats();
    void CollectMemoryStats();
    void UpdateFPSAverage(float DeltaTime);

    // Optimization
    void ApplyAutoOptimizations();
    void OptimizePhysicsSettings();
    void ReducePhysicsComplexity();

private:
    // Monitoring state
    UPROPERTY()
    bool bIsMonitoring;

    UPROPERTY()
    bool bIsPaused;

    UPROPERTY()
    float LastUpdateTime;

    // Performance data
    UPROPERTY()
    FCore_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY()
    TArray<FCore_PhysicsPerformanceMetrics> MetricsHistory;

    UPROPERTY()
    FCore_PhysicsPerformanceThresholds PerformanceThresholds;

    // FPS tracking
    TArray<float> FPSHistory;
    float FPSSum;
    int32 FPSHistoryIndex;

    // Alert management
    TMap<ECore_PhysicsPerformanceLevel, bool> AlertEnabled;
    TMap<ECore_PhysicsPerformanceLevel, float> LastAlertTime;
    float AlertCooldownTime;

    // Timer handles
    FTimerHandle MonitoringTimerHandle;
    FTimerHandle OptimizationTimerHandle;

    // Cached references
    UPROPERTY()
    class UPhysicsSystemManager* PhysicsSystemManager;

    UPROPERTY()
    class UChaosPhysicsManager* ChaosPhysicsManager;
};