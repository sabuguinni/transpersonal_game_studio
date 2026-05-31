#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Perf_PerformanceProfiler.generated.h"

UENUM(BlueprintType)
enum class EPerf_ProfilerCategory : uint8
{
    Rendering,
    Physics,
    AI,
    Audio,
    Memory,
    Network,
    Gameplay,
    World
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetric
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString MetricName;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentValue;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageValue;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinValue;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxValue;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_ProfilerCategory Category;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FDateTime LastUpdate;

    FPerf_PerformanceMetric()
    {
        MetricName = TEXT("Unknown");
        CurrentValue = 0.0f;
        AverageValue = 0.0f;
        MinValue = 0.0f;
        MaxValue = 0.0f;
        Category = EPerf_ProfilerCategory::Gameplay;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FPerf_FrameData()
    {
        FrameTime = 0.0f;
        FPS = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceProfiler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PerformanceProfiler();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsProfilingActive() const { return bIsProfilingActive; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameData GetCurrentFrameData() const { return CurrentFrameData; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_PerformanceMetric> GetAllMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetric GetMetric(const FString& MetricName) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AddCustomMetric(const FString& MetricName, float Value, EPerf_ProfilerCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SaveProfileDataToFile(const FString& FileName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const { return AverageFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMinFPS() const { return MinFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMaxFPS() const { return MaxFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float NewTargetFPS) { TargetFPS = NewTargetFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFPS() const { return TargetFPS; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsProfilingActive;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_FrameData CurrentFrameData;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TMap<FString, FPerf_PerformanceMetric> PerformanceMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 FrameCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TotalFrameTime;

    FTimerHandle ProfilingTimerHandle;

private:
    void UpdateFrameData();
    void UpdateMetrics();
    void CalculateAverages();
    FString GetProfileDataAsString() const;
};