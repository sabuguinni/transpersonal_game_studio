#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_PerformanceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceCritical = false;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_PerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EEng_PerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceCritical() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLevel();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceTest();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EEng_PerformanceLevel CurrentPerformanceLevel = EEng_PerformanceLevel::Medium;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PerformanceCheckInterval = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CriticalFrameTimeThreshold = 33.33f;

    void UpdateMetrics();
    void CheckPerformanceThresholds();
    void ApplyPerformanceOptimizations();

private:
    FTimerHandle MetricsUpdateTimer;
    float LastFrameTime = 0.0f;
    int32 FrameCounter = 0;
};