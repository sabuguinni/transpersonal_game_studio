#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "Perf_PhysicsGameplayIntegrationOptimizer.generated.h"

// Performance metrics for physics gameplay integration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsGameplayIntegrationMetrics
{
    GENERATED_BODY()

    // Core integration performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Integration")
    float IntegrationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Integration")
    float GameplayPhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Integration")
    float CharacterIntegrationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Integration")
    float DinosaurIntegrationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Integration")
    float TerrainIntegrationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Integration")
    float CollisionIntegrationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Integration")
    float ValidationTime;

    // Memory metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Memory")
    float MemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Memory")
    int32 ActiveIntegrationComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Memory")
    int32 QueuedIntegrationTasks;

    // Quality metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Quality")
    float IntegrationAccuracy;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Quality")
    float PhysicsStability;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Quality")
    float GameplayResponsiveness;

    FPerf_PhysicsGameplayIntegrationMetrics()
        : IntegrationTime(0.0f)
        , GameplayPhysicsTime(0.0f)
        , CharacterIntegrationTime(0.0f)
        , DinosaurIntegrationTime(0.0f)
        , TerrainIntegrationTime(0.0f)
        , CollisionIntegrationTime(0.0f)
        , ValidationTime(0.0f)
        , MemoryUsage(0.0f)
        , ActiveIntegrationComponents(0)
        , QueuedIntegrationTasks(0)
        , IntegrationAccuracy(1.0f)
        , PhysicsStability(1.0f)
        , GameplayResponsiveness(1.0f)
    {
    }
};

// Performance optimization settings for physics gameplay integration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsGameplayIntegrationSettings
{
    GENERATED_BODY()

    // Target performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float TargetFrameTime60FPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float TargetFrameTime30FPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float MaxIntegrationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float MaxGameplayPhysicsTime;

    // Quality vs performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Quality", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float IntegrationQualityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Quality", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float PhysicsDetailLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Quality", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float GameplayResponsivenessScale;

    // Optimization thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Optimization")
    float PerformanceWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Optimization")
    float PerformanceCriticalThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Optimization")
    int32 MaxConcurrentIntegrations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Optimization")
    int32 IntegrationBatchSize;

    // Memory management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    float MemoryBudgetMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    float GarbageCollectionThreshold;

    FPerf_PhysicsGameplayIntegrationSettings()
        : TargetFrameTime60FPS(16.67f)
        , TargetFrameTime30FPS(33.33f)
        , MaxIntegrationTime(8.0f)
        , MaxGameplayPhysicsTime(5.0f)
        , IntegrationQualityScale(1.0f)
        , PhysicsDetailLevel(1.0f)
        , GameplayResponsivenessScale(1.0f)
        , PerformanceWarningThreshold(0.75f)
        , PerformanceCriticalThreshold(0.9f)
        , MaxConcurrentIntegrations(16)
        , IntegrationBatchSize(4)
        , MemoryBudgetMB(256.0f)
        , GarbageCollectionThreshold(0.8f)
    {
    }
};

// Physics gameplay integration performance state
UENUM(BlueprintType)
enum class EPerf_PhysicsGameplayIntegrationState : uint8
{
    Optimal         UMETA(DisplayName = "Optimal Performance"),
    Good           UMETA(DisplayName = "Good Performance"),
    Warning        UMETA(DisplayName = "Performance Warning"),
    Critical       UMETA(DisplayName = "Critical Performance"),
    Emergency      UMETA(DisplayName = "Emergency Optimization"),
    Disabled       UMETA(DisplayName = "Integration Disabled")
};

/**
 * Performance optimizer specifically designed for the Core_PhysicsGameplayIntegrator system.
 * Monitors and optimizes physics gameplay integration performance to maintain 60fps on PC
 * and 30fps on console. Integrates with Agent #3's physics gameplay integration system.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsGameplayIntegrationOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsGameplayIntegrationOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance|Monitoring")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance|Monitoring")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance|Monitoring")
    FPerf_PhysicsGameplayIntegrationMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Monitoring")
    EPerf_PhysicsGameplayIntegrationState GetCurrentPerformanceState() const;

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Performance|Optimization")
    void OptimizeIntegrationPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance|Optimization")
    void SetOptimizationSettings(const FPerf_PhysicsGameplayIntegrationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance|Optimization")
    void ApplyPerformanceProfile(EPerformanceProfile Profile);

    UFUNCTION(BlueprintCallable, Category = "Performance|Optimization")
    void EnableEmergencyOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance|Optimization")
    void DisableEmergencyOptimization();

    // Integration-specific optimization
    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    void OptimizeGameplayPhysicsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    void OptimizeCharacterPhysicsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    void OptimizeDinosaurPhysicsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Performance|Integration")
    void OptimizeTerrainPhysicsIntegration();

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void OptimizeIntegrationMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    float GetMemoryUsagePercentage() const;

    // Quality adjustment
    UFUNCTION(BlueprintCallable, Category = "Performance|Quality")
    void AdjustIntegrationQuality(float QualityScale);

    UFUNCTION(BlueprintCallable, Category = "Performance|Quality")
    void SetPhysicsDetailLevel(float DetailLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance|Quality")
    void SetGameplayResponsiveness(float ResponsivenessScale);

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "Performance|Validation")
    bool ValidateIntegrationPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance|Validation")
    bool IsPerformanceWithinTargets() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Validation")
    TArray<FString> GetPerformanceWarnings() const;

    // Editor testing
    UFUNCTION(CallInEditor, Category = "Performance|Testing")
    void RunIntegrationPerformanceTest();

    UFUNCTION(CallInEditor, Category = "Performance|Testing")
    void GeneratePerformanceReport();

protected:
    // Core properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|State")
    FPerf_PhysicsGameplayIntegrationMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Settings")
    FPerf_PhysicsGameplayIntegrationSettings OptimizationSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|State")
    EPerf_PhysicsGameplayIntegrationState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Monitoring")
    bool bMonitoringEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Monitoring")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Optimization")
    bool bAutoOptimizationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Optimization")
    bool bEmergencyOptimizationActive;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|Tracking")
    float LastIntegrationTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|Tracking")
    float AverageIntegrationTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|Tracking")
    float PeakIntegrationTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|Tracking")
    int32 IntegrationFrameCount;

    // Timer handles
    FTimerHandle MonitoringTimerHandle;
    FTimerHandle OptimizationTimerHandle;

private:
    // Internal methods
    void UpdatePerformanceMetrics();
    void UpdatePerformanceState();
    void ApplyOptimizations();
    void HandlePerformanceWarning();
    void HandlePerformanceCritical();
    void ResetPerformanceTracking();

    // Optimization helpers
    void OptimizeIntegrationBatching();
    void OptimizeIntegrationPriority();
    void OptimizeIntegrationCaching();
    void OptimizeIntegrationThreading();

    // Performance calculation
    float CalculateIntegrationEfficiency() const;
    float CalculateMemoryEfficiency() const;
    float CalculateOverallPerformanceScore() const;

    // Validation helpers
    bool ValidateFrameTimeTargets() const;
    bool ValidateMemoryUsage() const;
    bool ValidateIntegrationStability() const;
};