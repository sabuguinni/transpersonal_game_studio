#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_PerformanceProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PropsCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTimeMS;

    FEng_PerformanceMetrics()
        : FrameRate(0.0f)
        , MemoryUsageMB(0.0f)
        , ActorCount(0)
        , DinosaurCount(0)
        , PropsCount(0)
        , GPUTimeMS(0.0f)
        , GameThreadTimeMS(0.0f)
        , RenderThreadTimeMS(0.0f)
    {}
};

UENUM(BlueprintType)
enum class EEng_PerformanceLevel : uint8
{
    Excellent,
    Good,
    Warning,
    Critical
};

UCLASS()
class TRANSPERSONALGAME_API UEng_PerformanceProfiler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EEng_PerformanceLevel GetPerformanceLevel();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsProfilingActive() const { return bIsProfilingActive; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float NewTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnforceActorLimits();

private:
    UPROPERTY()
    bool bIsProfilingActive;

    UPROPERTY()
    float TargetFrameRate;

    UPROPERTY()
    FTimerHandle ProfilingTimerHandle;

    UPROPERTY()
    TArray<FEng_PerformanceMetrics> MetricsHistory;

    void UpdateMetrics();
    void CheckPerformanceThresholds();
    void CleanupExcessActors();
    int32 CountActorsByType(const FString& TypeName);
};