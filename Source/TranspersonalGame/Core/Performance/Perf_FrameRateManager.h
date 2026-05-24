#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Perf_FrameRateManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceTarget : uint8
{
    Console_30FPS    UMETA(DisplayName = "Console 30 FPS"),
    PC_60FPS         UMETA(DisplayName = "PC 60 FPS"),
    PC_120FPS        UMETA(DisplayName = "PC 120 FPS"),
    Adaptive         UMETA(DisplayName = "Adaptive")
};

USTRUCT(BlueprintType)
struct FPerf_FrameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly)
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly)
    float MinFPS;

    UPROPERTY(BlueprintReadOnly)
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly)
    float FrameTime;

    UPROPERTY(BlueprintReadOnly)
    float RenderTime;

    UPROPERTY(BlueprintReadOnly)
    float GameTime;

    FPerf_FrameData()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        RenderTime = 0.0f;
        GameTime = 0.0f;
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

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameData GetCurrentFrameData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EPerf_PerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptivePerformance(bool bEnable);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForCurrentFrameRate();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxPhysicsActors(int32 MaxActors);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODDistanceMultiplier(float Multiplier);

    // Debug and logging
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void LogPerformanceStats();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ResetPerformanceStats();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceTarget PerformanceTarget;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bAdaptivePerformanceEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float PerformanceCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxPhysicsActors;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float LODDistanceMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_FrameData CurrentFrameData;

private:
    bool bMonitoringActive;
    float MonitoringTimer;
    TArray<float> FrameTimeHistory;
    int32 FrameCount;
    float TotalFrameTime;

    void UpdateFrameData(float DeltaTime);
    void ApplyPerformanceOptimizations();
    void CheckAdaptivePerformance();
};