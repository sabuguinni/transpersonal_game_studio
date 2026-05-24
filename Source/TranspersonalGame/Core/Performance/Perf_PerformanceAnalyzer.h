#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Perf_PerformanceAnalyzer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 0.0f;
        MaxFPS = 0.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        GPUTime = 0.0f;
        CPUTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ActorPerformanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString ActorName;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString ActorClass;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DistanceToPlayer;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsVisible;

    FPerf_ActorPerformanceData()
    {
        ActorName = TEXT("");
        ActorClass = TEXT("");
        RenderTime = 0.0f;
        TriangleCount = 0;
        DistanceToPlayer = 0.0f;
        bIsVisible = false;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance))
class TRANSPERSONALGAME_API UPerf_PerformanceAnalyzer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceAnalyzer();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Performance analysis functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_ActorPerformanceData> GetActorPerformanceData();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceMetrics();

    // Performance optimization suggestions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetPerformanceOptimizationSuggestions();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldReduceQuality();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldIncreaseQuality();

protected:
    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    TArray<float> FPSHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsAnalyzing;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float AnalysisTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float AnalysisInterval;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MinAcceptableFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    int32 MaxDrawCalls;

private:
    void UpdateFPSHistory(float CurrentFPS);
    void CalculateAverageMetrics();
    void AnalyzeActorPerformance();
    float GetMemoryUsage();
    int32 GetDrawCallCount();
    float GetGPUTime();
    float GetCPUTime();
};