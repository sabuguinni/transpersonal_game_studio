#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Perf_FrameRateManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DeltaTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    FPerf_FrameData()
    {
        DeltaTime = 0.0f;
        FrameRate = 0.0f;
        ActorCount = 0;
        PhysicsActorCount = 0;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsActors = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableFrameRateSmoothing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationThreshold = 0.8f;

    FPerf_PerformanceSettings()
    {
        TargetFrameRate = 60.0f;
        MinFrameRate = 30.0f;
        MaxPhysicsActors = 100;
        PhysicsCullingDistance = 5000.0f;
        bEnableAutomaticOptimization = true;
        bEnableFrameRateSmoothing = true;
        OptimizationThreshold = 0.8f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_FrameRateManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_FrameRateManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_PerformanceSettings PerformanceSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    FPerf_FrameData CurrentFrameData;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<FPerf_FrameData> FrameHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 FrameHistorySize = 60;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    bool bIsPerformanceCritical = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float AverageFrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float MinFrameRateRecorded = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float MaxFrameRateRecorded = 0.0f;

private:
    float FrameRateAccumulator = 0.0f;
    int32 FrameCount = 0;
    float LastOptimizationTime = 0.0f;
    static constexpr float OptimizationInterval = 1.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float NewTargetFrameRate);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutomaticOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameData GetCurrentFrameData() const { return CurrentFrameData; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameRate() const { return AverageFrameRate; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceCritical() const { return bIsPerformanceCritical; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceOptimizationPass();

    UFUNCTION(CallInEditor, Category = "Performance")
    void TestPerformanceBaseline();

private:
    void UpdateFrameData(float DeltaTime);
    void UpdateFrameHistory();
    void CheckPerformanceThresholds();
    void ApplyAutomaticOptimizations();
    void OptimizePhysicsActors();
    void OptimizeRenderingSettings();
    void LogPerformanceWarning(const FString& Warning);
};