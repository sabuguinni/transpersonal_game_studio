#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Perf_PhysicsRegistryIntegrator.generated.h"

// Forward declarations
class UCore_PhysicsRegistryBridge;

/**
 * Performance metrics structure for Physics Registry integration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsRegistryMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionChecksPerFrame = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PerformanceBudget = 8.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float BudgetUsagePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bWithinBudget = true;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 RegisteredSystemsCount = 0;

    FPerf_PhysicsRegistryMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActivePhysicsBodies = 0;
        CollisionChecksPerFrame = 0;
        PerformanceBudget = 8.0f;
        BudgetUsagePercent = 0.0f;
        bWithinBudget = true;
        RegisteredSystemsCount = 0;
    }
};

/**
 * Performance optimization directive for physics systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationDirective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Optimization")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Optimization")
    float TargetBudget = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Optimization")
    float CurrentUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Optimization")
    bool bRequiresOptimization = false;

    UPROPERTY(BlueprintReadOnly, Category = "Optimization")
    TArray<FString> OptimizationSuggestions;

    FPerf_PhysicsOptimizationDirective()
    {
        SystemName = TEXT("");
        TargetBudget = 1.0f;
        CurrentUsage = 0.0f;
        bRequiresOptimization = false;
        OptimizationSuggestions.Empty();
    }
};

/**
 * Performance profile configuration for different platforms
 */
UENUM(BlueprintType)
enum class EPerf_PerformanceProfile : uint8
{
    Development    UMETA(DisplayName = "Development"),
    Console        UMETA(DisplayName = "Console"),
    PC_High        UMETA(DisplayName = "PC High"),
    PC_Ultra       UMETA(DisplayName = "PC Ultra"),
    Mobile         UMETA(DisplayName = "Mobile")
};

/**
 * Physics Registry Integrator - Bridges Performance Optimizer with Core Physics Registry Bridge
 * Provides real-time performance monitoring and optimization for all physics systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsRegistryIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PhysicsRegistryIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Connect to Core Physics Registry Bridge and establish performance monitoring
     */
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    bool ConnectToPhysicsRegistry();

    /**
     * Get current physics performance metrics from Registry Bridge
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PhysicsRegistryMetrics GetPhysicsRegistryMetrics();

    /**
     * Set performance profile for physics systems
     */
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void SetPerformanceProfile(EPerf_PerformanceProfile Profile);

    /**
     * Send optimization directive to specific physics system
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool SendOptimizationDirective(const FString& SystemName, const FPerf_PhysicsOptimizationDirective& Directive);

    /**
     * Analyze all physics systems and generate optimization recommendations
     */
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    TArray<FPerf_PhysicsOptimizationDirective> AnalyzePhysicsPerformance();

    /**
     * Emergency performance intervention - reduce physics quality to maintain frame rate
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerEmergencyOptimization();

    /**
     * Check if physics systems are within performance budget
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ArePhysicsSystemsWithinBudget();

    /**
     * Get performance budget allocation for current profile
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentPerformanceBudget() const;

protected:
    /**
     * Reference to Core Physics Registry Bridge
     */
    UPROPERTY()
    TObjectPtr<UCore_PhysicsRegistryBridge> PhysicsRegistryBridge;

    /**
     * Current performance profile
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceProfile CurrentProfile;

    /**
     * Performance budget per profile (in milliseconds)
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    TMap<EPerf_PerformanceProfile, float> PerformanceBudgets;

    /**
     * Last recorded physics metrics
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PhysicsRegistryMetrics LastMetrics;

    /**
     * Performance monitoring enabled
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bPerformanceMonitoringEnabled;

    /**
     * Emergency optimization threshold (percentage of budget)
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "50.0", ClampMax = "95.0"))
    float EmergencyOptimizationThreshold;

private:
    /**
     * Initialize performance budgets for all profiles
     */
    void InitializePerformanceBudgets();

    /**
     * Update performance metrics from Registry Bridge
     */
    void UpdatePerformanceMetrics();

    /**
     * Generate optimization suggestions for a system
     */
    TArray<FString> GenerateOptimizationSuggestions(const FString& SystemName, float CurrentUsage, float TargetBudget);

    /**
     * Timer handle for performance monitoring
     */
    FTimerHandle PerformanceMonitoringTimer;

    /**
     * Performance monitoring interval (seconds)
     */
    float MonitoringInterval;

    /**
     * Connection established to Physics Registry Bridge
     */
    bool bConnectedToRegistry;
};