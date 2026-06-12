#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "QA_PerformanceMonitor.generated.h"

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
        DrawCalls = 0;
        Triangles = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQA_PerformanceMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool IsPerformanceAcceptable();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void LogPerformanceReport();

    // Performance thresholds
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void SetFrameRateThreshold(float MinFrameRate) { MinAcceptableFrameRate = MinFrameRate; }

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void SetActorCountThreshold(int32 MaxActors) { MaxAcceptableActors = MaxActors; }

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void SetMemoryThreshold(float MaxMemoryMB) { MaxAcceptableMemoryMB = MaxMemoryMB; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    TArray<FQA_PerformanceMetrics> MetricsHistory;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MinAcceptableFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 MaxAcceptableActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MaxAcceptableMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 MaxAcceptableDrawCalls;

    // Monitoring timer
    FTimerHandle MonitoringTimer;

    void UpdateMetrics();
    void CollectFrameStats();
    void CollectActorStats();
    void CollectMemoryStats();
    void CollectRenderStats();
};