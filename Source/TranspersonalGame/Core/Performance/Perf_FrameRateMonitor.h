#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Perf_FrameRateMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DeltaTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    FPerf_FrameData()
    {
        DeltaTime = 0.0f;
        FrameRate = 60.0f;
        AverageFrameTime = 16.67f;
        TotalActors = 0;
        VisibleActors = 0;
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Optimal     UMETA(DisplayName = "Optimal (60+ FPS)"),
    Good        UMETA(DisplayName = "Good (45-60 FPS)"),
    Acceptable  UMETA(DisplayName = "Acceptable (30-45 FPS)"),
    Poor        UMETA(DisplayName = "Poor (15-30 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<15 FPS)")
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_FrameRateMonitor : public UGameInstanceSubsystem
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
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetStatistics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMonitoring() const { return bIsMonitoring; }

private:
    void UpdateFrameData();
    void CalculatePerformanceLevel();

    UPROPERTY()
    bool bIsMonitoring = false;

    UPROPERTY()
    FPerf_FrameData CurrentFrameData;

    UPROPERTY()
    EPerf_PerformanceLevel CurrentPerformanceLevel = EPerf_PerformanceLevel::Optimal;

    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    float TotalFrameTime = 0.0f;

    UPROPERTY()
    int32 FrameCount = 0;

    UPROPERTY()
    float MonitoringStartTime = 0.0f;

    FTimerHandle MonitoringTimerHandle;
};