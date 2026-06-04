#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_PerformanceValidator.generated.h"

USTRUCT(BlueprintType)
struct FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ComponentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 Triangles;

    FQA_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        FrameTime = 0.0f;
        ActorCount = 0;
        ComponentCount = 0;
        MemoryUsageMB = 0.0f;
        GPUMemoryUsageMB = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_PerformanceValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_PerformanceValidator();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Performance validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StartPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StopPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateFrameRate(float MinFrameRate);

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateMemoryUsage(float MaxMemoryMB);

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateActorCount(int32 MaxActors);

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void LogPerformanceReport();

    // Stress testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void RunStressTest(int32 ActorsToSpawn, float TestDuration);

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void CleanupStressTest();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Performance")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Performance")
    bool bIsTestingActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Performance")
    float TestStartTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Performance")
    TArray<AActor*> StressTestActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MetricsUpdateInterval;

private:
    float LastMetricsUpdate;
    TArray<float> FrameTimeHistory;
    
    void UpdateMetrics();
    void CollectFrameTimeData(float DeltaTime);
    float CalculateAverageFrameTime() const;
};