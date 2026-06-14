#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Perf_PerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct FPerf_FrameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float DeltaTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float FPS = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 Triangles = 0;
};

USTRUCT(BlueprintType)
struct FPerf_MemoryData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float UsedPhysical = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float UsedVirtual = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float PeakUsedPhysical = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float PeakUsedVirtual = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float TexturePoolSize = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnPerformanceAlert, const FString&, AlertMessage);

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_PerformanceMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameData GetCurrentFrameData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_MemoryData GetCurrentMemoryData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetFPSTarget(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceWarnings(bool bEnable);

    UPROPERTY(BlueprintAssignable)
    FPerf_OnPerformanceAlert OnPerformanceAlert;

protected:
    UPROPERTY()
    bool bIsMonitoring = false;

    UPROPERTY()
    float TargetFPS = 60.0f;

    UPROPERTY()
    bool bPerformanceWarningsEnabled = true;

    UPROPERTY()
    TArray<float> FPSHistory;

    UPROPERTY()
    int32 MaxHistorySize = 300; // 5 seconds at 60fps

    FTimerHandle MonitoringTimer;

    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void LogPerformanceData();
};