#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Perf_SurvivalPhysicsOptimizer.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class UCore_SurvivalPhysicsIntegration;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_SurvivalPhysicsMetrics
{
    GENERATED_BODY()

    // Survival physics integration performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float SurvivalIntegrationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float StaminaCalculationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float MovementModifierTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float TerrainInteractionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CollisionDamageTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float EnvironmentalEffectTime;

    // Memory usage tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float SurvivalMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ActiveSurvivalComponents;

    FPerf_SurvivalPhysicsMetrics()
        : SurvivalIntegrationTime(0.0f)
        , StaminaCalculationTime(0.0f)
        , MovementModifierTime(0.0f)
        , TerrainInteractionTime(0.0f)
        , CollisionDamageTime(0.0f)
        , EnvironmentalEffectTime(0.0f)
        , SurvivalMemoryUsage(0.0f)
        , ActiveSurvivalComponents(0)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_SurvivalOptimizationSettings
{
    GENERATED_BODY()

    // Target performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetSurvivalIntegrationTime_PC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetSurvivalIntegrationTime_Console;

    // Optimization parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float SurvivalTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableStaminaLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableTerrainLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableEnvironmentalLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxSurvivalProcessingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxConcurrentSurvivalCalculations;

    FPerf_SurvivalOptimizationSettings()
        : TargetSurvivalIntegrationTime_PC(0.5f)  // 0.5ms for 60fps
        , TargetSurvivalIntegrationTime_Console(1.0f)  // 1.0ms for 30fps
        , SurvivalTickRate(10.0f)  // 10Hz for survival calculations
        , bEnableStaminaLOD(true)
        , bEnableTerrainLOD(true)
        , bEnableEnvironmentalLOD(true)
        , MaxSurvivalProcessingDistance(5000.0f)
        , MaxConcurrentSurvivalCalculations(4)
    {}
};

UENUM(BlueprintType)
enum class EPerf_SurvivalOptimizationLevel : uint8
{
    Disabled = 0,
    Low = 1,
    Medium = 2,
    High = 3,
    Ultra = 4
};

/**
 * Performance optimizer specifically designed for survival physics integration systems.
 * Monitors and optimizes the performance of survival mechanics integration with physics,
 * ensuring 60fps on PC and 30fps on console while maintaining realistic survival gameplay.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_SurvivalPhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_SurvivalPhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core optimization interface
    UFUNCTION(BlueprintCallable, Category = "Survival Physics Optimization")
    void OptimizeSurvivalPhysicsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Survival Physics Optimization")
    void UpdateSurvivalPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Survival Physics Optimization")
    void ApplySurvivalOptimizationLevel(EPerf_SurvivalOptimizationLevel Level);

    // Specific survival system optimizations
    UFUNCTION(BlueprintCallable, Category = "Survival Physics Optimization")
    void OptimizeStaminaCalculations();

    UFUNCTION(BlueprintCallable, Category = "Survival Physics Optimization")
    void OptimizeMovementModifiers();

    UFUNCTION(BlueprintCallable, Category = "Survival Physics Optimization")
    void OptimizeTerrainInteraction();

    UFUNCTION(BlueprintCallable, Category = "Survival Physics Optimization")
    void OptimizeCollisionDamage();

    UFUNCTION(BlueprintCallable, Category = "Survival Physics Optimization")
    void OptimizeEnvironmentalEffects();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FPerf_SurvivalPhysicsMetrics GetSurvivalPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    float GetSurvivalFrameImpact() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    bool IsSurvivalPerformanceOptimal() const;

    // Integration with Agent #3's survival physics system
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ConnectToSurvivalPhysicsIntegration(UCore_SurvivalPhysicsIntegration* SurvivalSystem);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ValidateSurvivalArchitectureConnection();

    // Milestone validation for Walk Around prototype
    UFUNCTION(BlueprintCallable, Category = "Milestone Validation")
    bool ValidateWalkAroundSurvivalPerformance() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone Validation")
    void GenerateSurvivalPerformanceReport();

protected:
    // Performance metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_SurvivalPhysicsMetrics CurrentMetrics;

    // Optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    FPerf_SurvivalOptimizationSettings OptimizationSettings;

    // Current optimization level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPerf_SurvivalOptimizationLevel CurrentOptimizationLevel;

    // System references
    UPROPERTY()
    TWeakObjectPtr<UCore_SurvivalPhysicsIntegration> SurvivalPhysicsSystem;

    UPROPERTY()
    TWeakObjectPtr<ATranspersonalCharacter> TargetCharacter;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Tracking")
    float LastFrameTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Tracking")
    float AverageFrameTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Tracking")
    int32 OptimizationCycles;

private:
    // Internal optimization methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void ApplyDynamicOptimizations();
    void MonitorSurvivalSystemPerformance();
    void AdjustOptimizationLevel();

    // Performance measurement helpers
    double MeasureSurvivalIntegrationTime();
    double MeasureStaminaCalculationTime();
    double MeasureMovementModifierTime();
    double MeasureTerrainInteractionTime();

    // Optimization helpers
    void OptimizeSurvivalTickRates();
    void ApplySurvivalLODOptimizations();
    void OptimizeSurvivalMemoryUsage();

    // Frame time tracking
    TArray<float> FrameTimeHistory;
    static constexpr int32 MaxFrameHistory = 60;
    int32 FrameHistoryIndex;
};