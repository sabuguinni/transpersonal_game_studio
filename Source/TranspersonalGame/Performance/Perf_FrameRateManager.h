#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Perf_FrameRateManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceTarget : uint8
{
    PC_Ultra = 0    UMETA(DisplayName = "PC Ultra (60 FPS)"),
    PC_High = 1     UMETA(DisplayName = "PC High (45 FPS)"),
    Console = 2     UMETA(DisplayName = "Console (30 FPS)"),
    Mobile = 3      UMETA(DisplayName = "Mobile (24 FPS)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameMetrics
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
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DroppedFrames;

    FPerf_FrameMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        DroppedFrames = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_FrameRateManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_FrameRateManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EPerf_PerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceTarget GetPerformanceTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartFrameMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopFrameMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMonitoring() const { return bIsMonitoring; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableVSync(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxFPS(int32 MaxFPS);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceTarget CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_FrameMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MonitoringDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 FrameCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TotalFrameTime;

private:
    FTimerHandle MonitoringTimer;
    TArray<float> FrameTimeHistory;
    float LastFrameTime;

    void UpdateMetrics();
    void OnFrameTick();
    float CalculateAverageFPS() const;
};

#include "Perf_FrameRateManager.generated.h"