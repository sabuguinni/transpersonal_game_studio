#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "PerformanceManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformance, Log, All);

UENUM(BlueprintType)
enum class EPerformanceTarget : uint8
{
    PC_60FPS        UMETA(DisplayName = "PC 60 FPS"),
    Console_30FPS   UMETA(DisplayName = "Console 30 FPS"),
    Mobile_30FPS    UMETA(DisplayName = "Mobile 30 FPS")
};

USTRUCT(BlueprintType)
struct FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float FrameTime;

    UPROPERTY(BlueprintReadOnly)
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly)
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly)
    float GPUTime;

    UPROPERTY(BlueprintReadOnly)
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly)
    int32 VisiblePrimitives;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveAIAgents;

    UPROPERTY(BlueprintReadOnly)
    int32 StreamingTextures;

    FPerformanceMetrics()
    {
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        VisiblePrimitives = 0;
        MemoryUsageMB = 0.0f;
        ActiveAIAgents = 0;
        StreamingTextures = 0;
    }
};

USTRUCT(BlueprintType)
struct FPerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "8.0", ClampMax = "33.33"))
    float TargetFrameTime = 16.67f; // 60fps default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2.0", ClampMax = "15.0"))
    float MaxGameThreadTime = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2.0", ClampMax = "15.0"))
    float MaxRenderThreadTime = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2.0", ClampMax = "15.0"))
    float MaxGPUTime = 14.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100", ClampMax = "5000"))
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1000", ClampMax = "50000"))
    int32 MaxVisiblePrimitives = 15000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "500", ClampMax = "8000"))
    float MaxMemoryUsageMB = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "10", ClampMax = "1000"))
    int32 MaxActiveAIAgents = 200;
};

/**
 * Central Performance Management System
 * Monitors frame budgets, implements dynamic LOD scaling, and manages performance-critical systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    APerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerformanceTarget CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerformanceBudget PerformanceBudget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Monitoring")
    float MetricsUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Adaptive")
    bool bEnableAdaptiveQuality = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Adaptive")
    float AdaptiveResponseTime = 2.0f;

private:
    float LastMetricsUpdate;
    TArray<float> FrameTimeHistory;
    int32 FrameHistorySize = 60;
    
    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceBudgets();
    void ApplyAdaptiveQuality();
    
    // Scalability management
    void SetScalabilityLevel(int32 Level);
    void AdjustDinosaurLOD(float PerformanceRatio);
    void AdjustVegetationDensity(float PerformanceRatio);
    void AdjustShadowQuality(float PerformanceRatio);

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EPerformanceTarget NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPerformanceRatio() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance")
    void OnPerformanceTargetChanged(EPerformanceTarget NewTarget);

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance")
    void OnBudgetExceeded(const FPerformanceMetrics& Metrics);
};