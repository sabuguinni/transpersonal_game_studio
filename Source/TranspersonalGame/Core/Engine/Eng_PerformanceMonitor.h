#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Eng_PerformanceMonitor.generated.h"

UENUM(BlueprintType)
enum class EEng_PerformanceLevel : uint8
{
    Excellent = 0,  // 60+ FPS
    Good = 1,       // 45-60 FPS
    Fair = 2,       // 30-45 FPS
    Poor = 3,       // 15-30 FPS
    Critical = 4    // <15 FPS
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
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
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EEng_PerformanceLevel PerformanceLevel;

    FEng_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 0.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsageMB = 0.0f;
        PerformanceLevel = EEng_PerformanceLevel::Good;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_PerformanceMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_PerformanceMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EEng_PerformanceLevel GetPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMonitoringActive() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<float> GetFPSHistory() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    float TargetFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    float MonitoringInterval;

    UPROPERTY(BlueprintReadOnly, Category = "History")
    TArray<float> FPSHistory;

    UPROPERTY(BlueprintReadOnly, Category = "History")
    int32 MaxHistorySize;

private:
    float LastMonitorTime;
    float TotalFrameTime;
    int32 FrameCount;
    
    void UpdateMetrics(float DeltaTime);
    void CalculateAverages();
    EEng_PerformanceLevel DeterminePerformanceLevel(float FPS) const;
    void LogPerformanceWarning(const FString& Warning);
};

UCLASS(BlueprintType, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_PerformanceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_PerformanceComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceTracking(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetActorPerformanceCost() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(float Budget);

protected:
    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    bool bTrackPerformance;

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float PerformanceBudget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentPerformanceCost;

private:
    float LastTickTime;
    void CalculatePerformanceCost(float DeltaTime);
};