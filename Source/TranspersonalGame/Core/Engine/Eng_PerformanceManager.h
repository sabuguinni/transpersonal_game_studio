#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "HAL/Platform.h"
#include "Eng_PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EEng_PerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"), 
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Potato      UMETA(DisplayName = "Potato")
};

USTRUCT(BlueprintType)
struct FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FEng_PerformanceMetrics()
    {
        FrameRate = 60.0f;
        FrameTime = 16.67f;
        GPUTime = 8.0f;
        CPUTime = 8.0f;
        DrawCalls = 1000;
        Triangles = 100000;
        MemoryUsageMB = 512.0f;
    }
};

USTRUCT(BlueprintType)
struct FEng_PerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxGPUTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxCPUTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxTriangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxMemoryMB;

    FEng_PerformanceBudget()
    {
        MaxFrameTime = 16.67f;  // 60 FPS
        MaxGPUTime = 12.0f;
        MaxCPUTime = 12.0f;
        MaxDrawCalls = 2000;
        MaxTriangles = 500000;
        MaxMemoryMB = 1024.0f;
    }
};

/**
 * Engine Architect Performance Manager
 * Supreme authority for performance monitoring, optimization, and budget enforcement
 * Ensures 60fps PC / 30fps console targets are maintained across all systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_PerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_PerformanceManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinBudget() const;

    // Performance Level Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EEng_PerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EEng_PerformanceLevel GetCurrentPerformanceLevel() const;

    // Budget Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(const FEng_PerformanceBudget& Budget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceBudget GetPerformanceBudget() const;

    // Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForCurrentHardware();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceSettings();

    // Validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    void ValidatePerformanceCompliance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    void GeneratePerformanceReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EEng_PerformanceLevel CurrentPerformanceLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceBudget PerformanceBudget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

private:
    void UpdateMetrics();
    void CheckBudgetCompliance();
    void ApplyLevelSettings(EEng_PerformanceLevel Level);
    void OptimizeDrawCalls();
    void OptimizeMemoryUsage();
    void OptimizeLODSettings();

    FTimerHandle MetricsUpdateTimer;
    float MetricsUpdateInterval;
    bool bBudgetViolationDetected;
};