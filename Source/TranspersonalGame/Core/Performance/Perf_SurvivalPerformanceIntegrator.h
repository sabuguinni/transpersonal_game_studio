#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "Perf_SurvivalPerformanceIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_SurvivalMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival Performance")
    float PlayerHealthUpdateCost;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Performance")
    float HungerSystemCost;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Performance")
    float ThirstSystemCost;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Performance")
    float StaminaSystemCost;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Performance")
    float TemperatureSystemCost;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Performance")
    int32 ActiveSurvivalComponents;

    FPerf_SurvivalMetrics()
        : PlayerHealthUpdateCost(0.0f)
        , HungerSystemCost(0.0f)
        , ThirstSystemCost(0.0f)
        , StaminaSystemCost(0.0f)
        , TemperatureSystemCost(0.0f)
        , ActiveSurvivalComponents(0)
    {}
};

UENUM(BlueprintType)
enum class EPerf_SurvivalOptimizationLevel : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Conservative    UMETA(DisplayName = "Conservative"),
    Balanced        UMETA(DisplayName = "Balanced"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Maximum         UMETA(DisplayName = "Maximum")
};

/**
 * Performance optimizer specifically for survival systems integration
 * Monitors and optimizes health, hunger, thirst, stamina, and temperature systems
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance))
class TRANSPERSONALGAME_API UPerf_SurvivalPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_SurvivalPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    void StartSurvivalPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    void StopSurvivalPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    FPerf_SurvivalMetrics GetCurrentSurvivalMetrics() const;

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    void SetSurvivalOptimizationLevel(EPerf_SurvivalOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    void OptimizeSurvivalSystems();

    // Integration with Core Systems
    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    void IntegrateWithPhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Survival Performance")
    void ValidateSurvivalPerformanceIntegration();

protected:
    // Performance monitoring state
    UPROPERTY(BlueprintReadOnly, Category = "Survival Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_SurvivalMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_SurvivalOptimizationLevel OptimizationLevel;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Performance", meta = (AllowPrivateAccess = "true"))
    float MaxAcceptableSurvivalCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Performance", meta = (AllowPrivateAccess = "true"))
    float TargetSurvivalUpdateRate;

    // Optimization parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Performance", meta = (AllowPrivateAccess = "true"))
    bool bEnableAdaptiveUpdateRates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Performance", meta = (AllowPrivateAccess = "true"))
    bool bEnableSurvivalLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Performance", meta = (AllowPrivateAccess = "true"))
    float SurvivalSystemBudgetMs;

private:
    // Internal monitoring
    void UpdateSurvivalMetrics(float DeltaTime);
    void ApplyOptimizations();
    void CheckPerformanceThresholds();

    // Timing data
    double LastUpdateTime;
    TArray<float> RecentFrameTimes;
    int32 FrameTimeIndex;
};