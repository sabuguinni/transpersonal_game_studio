#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PerformanceTargets.h"
#include "Perf_SurvivalPerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_SurvivalOptimizationLevel : uint8
{
    None        UMETA(DisplayName = "No Optimization"),
    Low         UMETA(DisplayName = "Low Optimization"),
    Medium      UMETA(DisplayName = "Medium Optimization"),
    High        UMETA(DisplayName = "High Optimization"),
    Extreme     UMETA(DisplayName = "Extreme Optimization")
};

USTRUCT(BlueprintType)
struct FPerf_SurvivalPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float SurvivalTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveSurvivalComponents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageStatUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsMultiplierUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CriticalStateCharacters = 0;

    FPerf_SurvivalPerformanceMetrics()
    {
        SurvivalTickTime = 0.0f;
        ActiveSurvivalComponents = 0;
        AverageStatUpdateTime = 0.0f;
        PhysicsMultiplierUpdateTime = 0.0f;
        CriticalStateCharacters = 0;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance))
class TRANSPERSONALGAME_API UPerf_SurvivalPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_SurvivalPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance optimization methods
    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    void OptimizeSurvivalSystems();

    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    void SetOptimizationLevel(EPerf_SurvivalOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    FPerf_SurvivalPerformanceMetrics GetPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    void OptimizeSurvivalTickFrequency();

    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    void OptimizeStatUpdateFrequency();

    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    void OptimizePhysicsMultiplierUpdates();

    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    bool ShouldReduceSurvivalUpdates() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_SurvivalOptimizationLevel OptimizationLevel = EPerf_SurvivalOptimizationLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MinFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float SurvivalUpdateInterval = 0.1f; // Base update interval

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float HealthyStateUpdateInterval = 0.5f; // Slower updates when healthy

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float CriticalStateUpdateInterval = 0.05f; // Faster updates when critical

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 MaxSurvivalComponentsPerFrame = 50;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_SurvivalPerformanceMetrics CurrentMetrics;

private:
    float LastOptimizationTime = 0.0f;
    float OptimizationCheckInterval = 1.0f;
    
    void UpdatePerformanceMetrics();
    void ApplyOptimizationSettings();
    void OptimizeBasedOnFrameRate();
    void AdjustSurvivalTickRates();
    void PoolSurvivalUpdates();
    
    TArray<class UCore_SurvivalPhysics*> TrackedSurvivalComponents;
    int32 CurrentUpdateIndex = 0;
};