#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Perf_MilestonePerformanceAnalyzer.generated.h"

// Forward declarations
class UCore_MilestonePhysics;
class ATranspersonalCharacter;

/**
 * Performance analysis structure for Milestone 1 "WALK AROUND" requirements
 * Tracks frame rate, physics performance, and character movement optimization
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MilestoneMetrics
{
    GENERATED_BODY()

    // Frame Rate Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Frame Rate")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Frame Rate")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Frame Rate")
    float MinFPS = 999.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Frame Rate")
    float MaxFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Frame Rate")
    float FrameTime = 0.0f;

    // Physics Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    float PhysicsTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    float CollisionDetectionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    int32 CollisionPairs = 0;

    // Character Movement Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    float CharacterMovementTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    float TerrainInteractionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    bool bCharacterMovementOptimal = true;

    // Memory Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PhysicsMemoryUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float CharacterMemoryUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TotalMemoryUsage = 0.0f;

    FPerf_MilestoneMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        PhysicsTickTime = 0.0f;
        CollisionDetectionTime = 0.0f;
        ActiveRigidBodies = 0;
        CollisionPairs = 0;
        CharacterMovementTime = 0.0f;
        TerrainInteractionTime = 0.0f;
        bCharacterMovementOptimal = true;
        PhysicsMemoryUsage = 0.0f;
        CharacterMemoryUsage = 0.0f;
        TotalMemoryUsage = 0.0f;
    }
};

/**
 * Milestone performance targets for different hardware configurations
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MilestoneTargets
{
    GENERATED_BODY()

    // Target FPS for different platforms
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float TargetFPS_PC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float TargetFPS_Console = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float TargetFPS_Mobile = 30.0f;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float MaxPhysicsTickTime = 5.0f; // milliseconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float MaxCharacterMovementTime = 2.0f; // milliseconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float MaxMemoryUsage = 512.0f; // MB

    FPerf_MilestoneTargets()
    {
        TargetFPS_PC = 60.0f;
        TargetFPS_Console = 30.0f;
        TargetFPS_Mobile = 30.0f;
        MaxPhysicsTickTime = 5.0f;
        MaxCharacterMovementTime = 2.0f;
        MaxMemoryUsage = 512.0f;
    }
};

/**
 * Performance Analyzer for Milestone 1 "WALK AROUND" requirements
 * Integrates with Core Systems Programmer's physics validation to ensure
 * 60fps PC / 30fps console performance targets are met
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_MilestonePerformanceAnalyzer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_MilestonePerformanceAnalyzer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance Analysis Methods
    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    void StartMilestoneAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    void StopMilestoneAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    FPerf_MilestoneMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    bool IsMilestone1PerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    float GetMilestone1CompletionPercentage() const;

    // Integration with Core Physics Systems
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void IntegrateWithPhysicsValidation(UCore_MilestonePhysics* PhysicsValidator);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void AnalyzeCharacterPhysicsPerformance(ATranspersonalCharacter* Character);

    // Optimization Methods
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeForMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyPerformanceSettings(const FPerf_MilestoneTargets& Targets);

    // Reporting Methods
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FString GeneratePerformanceReport() const;

    UFUNCTION(BlueprintCallable, Category = "Reporting", CallInEditor = true)
    void LogPerformanceStatus() const;

protected:
    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    FPerf_MilestoneMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FPerf_MilestoneTargets PerformanceTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float AnalysisInterval = 0.1f; // seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDetailedProfiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoOptimize = false;

    // Internal tracking
    UPROPERTY()
    bool bAnalysisActive = false;

    UPROPERTY()
    float LastAnalysisTime = 0.0f;

    UPROPERTY()
    TArray<float> FPSSamples;

    UPROPERTY()
    int32 MaxSamples = 300; // 30 seconds at 10Hz

    // Physics integration
    UPROPERTY()
    TWeakObjectPtr<UCore_MilestonePhysics> PhysicsValidator;

    UPROPERTY()
    TWeakObjectPtr<ATranspersonalCharacter> TrackedCharacter;

private:
    // Internal analysis methods
    void UpdateFrameRateMetrics(float DeltaTime);
    void UpdatePhysicsMetrics();
    void UpdateCharacterMetrics();
    void UpdateMemoryMetrics();
    bool CheckPerformanceThresholds() const;
    void ApplyOptimizations();
    void ResetMetrics();
};