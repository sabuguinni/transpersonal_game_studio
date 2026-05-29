#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "PerformanceBudgetManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformanceBudget, Log, All);

/**
 * Performance Budget Categories for the Jurassic Survival Game
 * Each category has specific frame time allocations
 */
UENUM(BlueprintType)
enum class EPerformanceBudgetCategory : uint8
{
    // Core Systems (8.5ms @ 60fps target)
    GameThread_Core         UMETA(DisplayName = "Game Thread - Core"),
    GameThread_AI           UMETA(DisplayName = "Game Thread - AI & Behavior"),
    GameThread_Physics      UMETA(DisplayName = "Game Thread - Physics"),
    
    // Rendering (8.5ms @ 60fps target) 
    RenderThread_Geometry   UMETA(DisplayName = "Render Thread - Geometry"),
    RenderThread_Lighting   UMETA(DisplayName = "Render Thread - Lighting"),
    RenderThread_Effects    UMETA(DisplayName = "Render Thread - VFX"),
    
    // GPU (16.67ms @ 60fps target)
    GPU_BasePass           UMETA(DisplayName = "GPU - Base Pass"),
    GPU_Shadows            UMETA(DisplayName = "GPU - Shadow Rendering"),
    GPU_PostProcess        UMETA(DisplayName = "GPU - Post Processing"),
    GPU_Nanite             UMETA(DisplayName = "GPU - Nanite Rendering"),
    GPU_Lumen              UMETA(DisplayName = "GPU - Lumen GI"),
    
    // Memory Systems
    Memory_Streaming       UMETA(DisplayName = "Memory - Asset Streaming"),
    Memory_AI              UMETA(DisplayName = "Memory - AI Data"),
    
    MAX                    UMETA(Hidden)
};

/**
 * Performance Budget Entry
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceBudgetEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerformanceBudgetCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetTimeMS_PC = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetTimeMS_Console = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentTimeMS = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float WarningThresholdPercent = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CriticalThresholdPercent = 95.0f;

    FPerformanceBudgetEntry()
    {
        Category = EPerformanceBudgetCategory::GameThread_Core;
        TargetTimeMS_PC = 0.0f;
        TargetTimeMS_Console = 0.0f;
        CurrentTimeMS = 0.0f;
        WarningThresholdPercent = 80.0f;
        CriticalThresholdPercent = 95.0f;
    }
};

/**
 * Performance Metrics for Dinosaur-specific systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Performance")
    int32 TotalDinosaursInScene = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Performance")
    int32 ActiveAIDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Performance")
    int32 VisibleDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Performance")
    float AverageAITickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Performance")
    float TotalAIMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Performance")
    int32 ActiveBehaviorTrees = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Performance")
    int32 DinosaursInLOD0 = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Performance")
    int32 DinosaursInLOD1 = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Performance")
    int32 DinosaursInLOD2 = 0;
};

/**
 * Performance Budget Manager
 * Manages frame time budgets and provides real-time performance monitoring
 * specifically tailored for the Jurassic Survival game requirements
 */
UCLASS()
class TRANSPERSONALGAME_API UPerformanceBudgetManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Performance Budget Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializePerformanceBudgets();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceBudgetExceeded(EPerformanceBudgetCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const { return CurrentFrameTime; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FDinosaurPerformanceMetrics GetDinosaurMetrics() const { return DinosaurMetrics; }

    // Performance Scaling
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ScalePerformanceForDinosaurCount(int32 DinosaurCount);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyEmergencyPerformanceScaling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RestoreNormalPerformanceScaling();

    // Debugging and Visualization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceHUD(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

protected:
    // Performance budgets for different categories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budgets")
    TArray<FPerformanceBudgetEntry> PerformanceBudgets;

    // Current performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FDinosaurPerformanceMetrics DinosaurMetrics;

    // Performance scaling state
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bEmergencyScalingActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PerformanceScaleFactor = 1.0f;

private:
    // Internal tracking
    TArray<float> FrameTimeHistory;
    int32 FrameHistoryIndex = 0;
    static constexpr int32 MaxFrameHistory = 120; // 2 seconds at 60fps

    // Performance monitoring
    void UpdateFrameTimeHistory(float FrameTime);
    void UpdateDinosaurMetrics();
    void CheckPerformanceThresholds();
    
    // Platform detection
    bool IsConsole() const;
    bool IsPC() const;
    
    // Emergency scaling
    void ReduceAITickFrequency();
    void ReduceRenderingQuality();
    void RestoreAITickFrequency();
    void RestoreRenderingQuality();
};