#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Perf_PhysicsCoordinatorOptimizer.generated.h"

// Forward declarations
class UCore_PhysicsSystemCoordinator;
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsCoordinationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Coordination")
    float CoordinationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Coordination")
    float SystemRegistrationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Coordination")
    float CharacterPhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Coordination")
    float QualityAdjustmentTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Coordination")
    float ValidationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Coordination")
    int32 RegisteredSystems = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Coordination")
    int32 ActiveCharacters = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Coordination")
    float MemoryUsageMB = 0.0f;

    FPerf_PhysicsCoordinationMetrics()
    {
        CoordinationTime = 0.0f;
        SystemRegistrationTime = 0.0f;
        CharacterPhysicsTime = 0.0f;
        QualityAdjustmentTime = 0.0f;
        ValidationTime = 0.0f;
        RegisteredSystems = 0;
        ActiveCharacters = 0;
        MemoryUsageMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CoordinatorOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFrameTime60FPS = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFrameTime30FPS = 33.33f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Thresholds")
    float CoordinationTimeThreshold = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Thresholds")
    float SystemRegistrationThreshold = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Thresholds")
    float CharacterPhysicsThreshold = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Settings")
    int32 MaxRegisteredSystems = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Settings")
    int32 MaxActiveCharacters = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Management")
    float MaxMemoryUsageMB = 512.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Quality")
    bool bEnableAdaptiveQuality = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Quality")
    float QualityAdjustmentRate = 0.1f;

    FPerf_CoordinatorOptimizationSettings()
    {
        TargetFrameTime60FPS = 16.67f;
        TargetFrameTime30FPS = 33.33f;
        CoordinationTimeThreshold = 2.0f;
        SystemRegistrationThreshold = 1.0f;
        CharacterPhysicsThreshold = 5.0f;
        MaxRegisteredSystems = 20;
        MaxActiveCharacters = 50;
        MaxMemoryUsageMB = 512.0f;
        bEnableAdaptiveQuality = true;
        QualityAdjustmentRate = 0.1f;
    }
};

UENUM(BlueprintType)
enum class EPerf_CoordinatorOptimizationState : uint8
{
    Initializing    UMETA(DisplayName = "Initializing"),
    Monitoring      UMETA(DisplayName = "Monitoring"),
    Optimizing      UMETA(DisplayName = "Optimizing"),
    Degraded        UMETA(DisplayName = "Degraded"),
    Critical        UMETA(DisplayName = "Critical"),
    Disabled        UMETA(DisplayName = "Disabled")
};

/**
 * Performance optimizer specifically designed for Agent #3's Core_PhysicsSystemCoordinator.
 * Monitors and optimizes physics coordination performance to maintain 60fps on PC and 30fps on console.
 * Integrates with the coordinator's adaptive quality system for real-time optimization.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PhysicsCoordinatorOptimizer : public AActor
{
    GENERATED_BODY()

public:
    APerf_PhysicsCoordinatorOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    FPerf_CoordinatorOptimizationSettings OptimizationSettings;

    // Current performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_PhysicsCoordinationMetrics CurrentMetrics;

    // Optimization state
    UPROPERTY(BlueprintReadOnly, Category = "Optimization State")
    EPerf_CoordinatorOptimizationState OptimizationState;

    // Reference to physics coordinator
    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    UCore_PhysicsSystemCoordinator* PhysicsCoordinator;

    // Performance monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float MonitoringInterval = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
    float LastMonitoringTime = 0.0f;

    // Optimization history
    UPROPERTY(BlueprintReadOnly, Category = "Performance History")
    TArray<float> FrameTimeHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance History")
    int32 MaxHistorySize = 100;

public:
    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void UpdatePerformanceMetrics();

    // Optimization functions
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizePhysicsCoordination();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyQualityOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeSystemRegistration();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeCharacterPhysics();

    // Integration functions
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    bool RegisterWithPhysicsCoordinator();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void IntegrateWithAdaptiveQuality();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ValidatePhysicsIntegration();

    // Diagnostic functions
    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    FPerf_PhysicsCoordinationMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    float GetAverageFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    bool IsPerformanceWithinTargets() const;

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    void LogPerformanceReport();

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void RunPerformanceTest();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ResetOptimizationState();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ValidateCoordinatorIntegration();

private:
    // Internal optimization functions
    void UpdateOptimizationState();
    void ProcessPerformanceHistory();
    void ApplyAdaptiveOptimizations();
    void ValidatePerformanceTargets();

    // Monitoring timers
    float MonitoringTimer = 0.0f;
    float OptimizationTimer = 0.0f;

    // Performance tracking
    bool bIsMonitoring = false;
    bool bOptimizationEnabled = true;
    float LastFrameTime = 0.0f;
    float AccumulatedFrameTime = 0.0f;
    int32 FrameCount = 0;
};