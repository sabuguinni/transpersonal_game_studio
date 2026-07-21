#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Perf_PhysicsArchitecturalMonitor.generated.h"

// Forward declarations
class UCore_PhysicsArchitecturalIntegrator;

/**
 * Physics System Performance Budget Structure
 * Defines performance constraints for each physics system
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsSystemBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTimeBudgetMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryBudgetMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCollisionObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bIsActive;

    FPerf_PhysicsSystemBudget()
        : SystemName(TEXT("Unknown"))
        , FrameTimeBudgetMs(1.0f)
        , MemoryBudgetMB(100.0f)
        , MaxCollisionObjects(1000)
        , bIsActive(true)
    {}
};

/**
 * Real-time Physics Performance Metrics
 * Tracks actual performance against budgets
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsPerformanceSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveCollisionObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bBudgetViolation;

    FPerf_PhysicsPerformanceSnapshot()
        : CurrentFPS(60.0f)
        , FrameTimeMs(16.67f)
        , PhysicsTimeMs(0.0f)
        , CollisionTimeMs(0.0f)
        , MemoryUsageMB(0.0f)
        , ActiveCollisionObjects(0)
        , ActivePhysicsActors(0)
        , bBudgetViolation(false)
    {}
};

/**
 * Performance Violation Alert
 * Triggered when systems exceed their budgets
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceViolation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString ViolatingSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString ViolationType;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ActualValue;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float BudgetValue;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ViolationPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FDateTime Timestamp;

    FPerf_PerformanceViolation()
        : ViolatingSystem(TEXT("Unknown"))
        , ViolationType(TEXT("Unknown"))
        , ActualValue(0.0f)
        , BudgetValue(0.0f)
        , ViolationPercentage(0.0f)
        , Timestamp(FDateTime::Now())
    {}
};

/**
 * Physics Architectural Performance Monitor
 * Monitors physics systems performance against architectural budgets
 * Integrates with Core_PhysicsArchitecturalIntegrator for centralized monitoring
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsArchitecturalMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PhysicsArchitecturalMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Initialize performance monitoring for physics systems
     * Sets up budgets and monitoring parameters
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializePhysicsMonitoring();

    /**
     * Update performance monitoring
     * Called every frame to track physics performance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMonitoring(float DeltaTime);

    /**
     * Get current physics performance snapshot
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PhysicsPerformanceSnapshot GetCurrentPerformanceSnapshot() const;

    /**
     * Check if any physics systems are violating their budgets
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool HasPerformanceViolations() const;

    /**
     * Get list of current performance violations
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_PerformanceViolation> GetPerformanceViolations() const;

    /**
     * Set performance budget for a specific physics system
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsSystemBudget(const FString& SystemName, float FrameTimeBudgetMs, float MemoryBudgetMB, int32 MaxCollisionObjects);

    /**
     * Get performance budget for a specific physics system
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PhysicsSystemBudget GetPhysicsSystemBudget(const FString& SystemName) const;

    /**
     * Enable/disable performance monitoring
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMonitoringEnabled(bool bEnabled);

    /**
     * Get performance monitoring status
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMonitoringEnabled() const { return bMonitoringEnabled; }

    /**
     * Force performance optimization pass
     * Triggers immediate optimization of physics systems
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForcePerformanceOptimization();

    /**
     * Get performance statistics summary
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceStatsSummary() const;

protected:
    /**
     * Physics system performance budgets
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FPerf_PhysicsSystemBudget> PhysicsSystemBudgets;

    /**
     * Current performance snapshot
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_PhysicsPerformanceSnapshot CurrentSnapshot;

    /**
     * Active performance violations
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    TArray<FPerf_PerformanceViolation> ActiveViolations;

    /**
     * Reference to physics architectural integrator
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    UCore_PhysicsArchitecturalIntegrator* PhysicsIntegrator;

    /**
     * Performance monitoring enabled flag
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bMonitoringEnabled;

    /**
     * Performance update frequency (seconds)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MonitoringUpdateFrequency;

    /**
     * Time since last monitoring update
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TimeSinceLastUpdate;

    /**
     * Performance violation threshold percentage
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float ViolationThresholdPercentage;

private:
    /**
     * Initialize default physics system budgets
     */
    void InitializeDefaultBudgets();

    /**
     * Update physics performance metrics
     */
    void UpdatePhysicsMetrics();

    /**
     * Check for budget violations
     */
    void CheckBudgetViolations();

    /**
     * Log performance violation
     */
    void LogPerformanceViolation(const FString& SystemName, const FString& ViolationType, float ActualValue, float BudgetValue);

    /**
     * Calculate performance statistics
     */
    void CalculatePerformanceStats();

    /**
     * Get current frame time in milliseconds
     */
    float GetCurrentFrameTimeMs() const;

    /**
     * Get current memory usage in MB
     */
    float GetCurrentMemoryUsageMB() const;

    /**
     * Count active physics actors in the world
     */
    int32 CountActivePhysicsActors() const;

    /**
     * Count active collision objects
     */
    int32 CountActiveCollisionObjects() const;
};