#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "PerformanceManager.generated.h"

// Performance monitoring data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleTriangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceGood = true;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        VisibleTriangles = 0;
        MemoryUsageMB = 0.0f;
        bIsPerformanceGood = true;
    }
};

// Performance quality levels
UENUM(BlueprintType)
enum class EPerf_QualityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra"),
    Auto        UMETA(DisplayName = "Auto")
};

// Performance optimization strategies
UENUM(BlueprintType)
enum class EPerf_OptimizationStrategy : uint8
{
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Balanced        UMETA(DisplayName = "Balanced"),
    Conservative    UMETA(DisplayName = "Conservative"),
    Custom          UMETA(DisplayName = "Custom")
};

/**
 * Core Performance Manager - Monitors and optimizes game performance
 * Ensures 60fps on PC and 30fps on console across all game scenarios
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerformanceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === PERFORMANCE MONITORING ===
    
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFPS() const { return TargetFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float NewTargetFPS);

    // === QUALITY MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityLevel(EPerf_QualityLevel NewQualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_QualityLevel GetCurrentQualityLevel() const { return CurrentQualityLevel; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoQualityAdjustment(bool bEnable);

    // === OPTIMIZATION CONTROLS ===

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizationStrategy(EPerf_OptimizationStrategy Strategy);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLightingSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRenderingSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMemorySettings();

    // === PROFILING TOOLS ===

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void StartPerformanceProfiling();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void StopPerformanceProfiling();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void GeneratePerformanceReport();

    // === EVENTS ===

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceChanged, const FPerf_PerformanceMetrics&, Metrics);
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceChanged OnPerformanceChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQualityLevelChanged, EPerf_QualityLevel, NewQualityLevel);
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnQualityLevelChanged OnQualityLevelChanged;

protected:
    // === CONFIGURATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MinAcceptableFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bAutoQualityAdjustment = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float QualityAdjustmentInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_QualityLevel CurrentQualityLevel = EPerf_QualityLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_OptimizationStrategy OptimizationStrategy = EPerf_OptimizationStrategy::Balanced;

    // === PERFORMANCE BUDGETS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets", meta = (AllowPrivateAccess = "true"))
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets", meta = (AllowPrivateAccess = "true"))
    int32 MaxVisibleTriangles = 2000000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets", meta = (AllowPrivateAccess = "true"))
    float MaxMemoryUsageMB = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets", meta = (AllowPrivateAccess = "true"))
    float MaxFrameTimeMS = 16.67f; // 60 FPS

private:
    // === INTERNAL STATE ===

    FPerf_PerformanceMetrics CurrentMetrics;
    float LastQualityAdjustmentTime = 0.0f;
    bool bProfilingActive = false;
    TArray<FPerf_PerformanceMetrics> ProfilingData;

    // === INTERNAL METHODS ===

    void UpdatePerformanceMetrics();
    void CheckPerformanceBudgets();
    void AutoAdjustQuality();
    void ApplyQualitySettings();
    void LogPerformanceWarning(const FString& Warning);
};