#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Perf_RagdollPerformanceOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RagdollPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRagdolls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CulledRagdolls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageRagdollDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RagdollFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 HighDetailRagdolls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LowDetailRagdolls = 0;

    FPerf_RagdollPerformanceMetrics()
    {
        ActiveRagdolls = 0;
        CulledRagdolls = 0;
        AverageRagdollDistance = 0.0f;
        RagdollFrameTime = 0.0f;
        HighDetailRagdolls = 0;
        LowDetailRagdolls = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RagdollLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxActiveRagdolls = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float UpdateFrequencyHigh = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float UpdateFrequencyMedium = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float UpdateFrequencyLow = 10.0f;

    FPerf_RagdollLODSettings()
    {
        HighDetailDistance = 1000.0f;
        MediumDetailDistance = 2500.0f;
        LowDetailDistance = 5000.0f;
        CullDistance = 10000.0f;
        MaxActiveRagdolls = 20;
        UpdateFrequencyHigh = 60.0f;
        UpdateFrequencyMedium = 30.0f;
        UpdateFrequencyLow = 10.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_RagdollPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_RagdollPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRagdollPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateRagdollLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantRagdolls();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void BatchRagdollUpdates();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AnalyzeRagdollPerformance();

    // Settings and metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_RagdollLODSettings LODSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FPerf_RagdollPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableRagdollOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PerformanceUpdateInterval = 0.1f;

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogRagdollPerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ResetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void TestRagdollOptimization();

private:
    // Internal optimization functions
    void UpdateRagdollDistances();
    void ApplyRagdollLOD(AActor* RagdollActor, float Distance);
    void CullRagdoll(AActor* RagdollActor);
    void RestoreRagdoll(AActor* RagdollActor);
    float CalculateRagdollFrameTime();

    // Internal state
    TArray<TWeakObjectPtr<AActor>> TrackedRagdolls;
    TArray<TWeakObjectPtr<AActor>> CulledRagdolls;
    float LastPerformanceUpdate;
    float AccumulatedFrameTime;
    int32 FrameTimesamples;

    // Performance thresholds
    static constexpr float TARGET_FRAME_TIME = 16.67f; // 60 FPS
    static constexpr float WARNING_FRAME_TIME = 20.0f; // 50 FPS
    static constexpr float CRITICAL_FRAME_TIME = 33.33f; // 30 FPS
};