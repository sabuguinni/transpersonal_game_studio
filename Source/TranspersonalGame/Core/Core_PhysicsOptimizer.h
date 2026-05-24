#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Core_PhysicsOptimizer.generated.h"

class UCore_PhysicsSystemsIntegrator;

UENUM(BlueprintType)
enum class ECore_PhysicsQualityLevel : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    /** Average physics simulation time per frame (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AverageSimulationTime;

    /** Peak physics simulation time (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PeakSimulationTime;

    /** Number of active physics bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActivePhysicsBodies;

    /** Number of collision checks per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CollisionChecksPerFrame;

    /** Memory usage by physics system (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsMemoryUsage;

    /** Current frame rate (FPS) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentFrameRate;

    FCore_PhysicsPerformanceMetrics()
    {
        AverageSimulationTime = 0.0f;
        PeakSimulationTime = 0.0f;
        ActivePhysicsBodies = 0;
        CollisionChecksPerFrame = 0;
        PhysicsMemoryUsage = 0.0f;
        CurrentFrameRate = 60.0f;
    }
};

/**
 * Core Physics Optimizer
 * Monitors physics performance and automatically adjusts quality settings
 * Ensures stable 60fps on PC and 30fps on console platforms
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Set target frame rate for optimization */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetTargetFrameRate(float TargetFPS);

    /** Get current performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    FCore_PhysicsPerformanceMetrics GetPerformanceMetrics() const;

    /** Manually set physics quality level */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsQualityLevel(ECore_PhysicsQualityLevel QualityLevel);

    /** Get current physics quality level */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    ECore_PhysicsQualityLevel GetCurrentQualityLevel() const;

    /** Enable/disable automatic optimization */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetAutoOptimizationEnabled(bool bEnabled);

    /** Force immediate optimization check */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ForceOptimizationCheck();

    /** Get optimization recommendations */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    FString GetOptimizationRecommendations() const;

protected:
    /** Target frame rate to maintain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate;

    /** Current physics quality level */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Optimization Settings", meta = (AllowPrivateAccess = "true"))
    ECore_PhysicsQualityLevel CurrentQualityLevel;

    /** Whether automatic optimization is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings", meta = (AllowPrivateAccess = "true"))
    bool bAutoOptimizationEnabled;

    /** How often to check performance (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings", meta = (AllowPrivateAccess = "true"))
    float OptimizationCheckInterval;

    /** Frame rate threshold for quality reduction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings", meta = (AllowPrivateAccess = "true"))
    float QualityReductionThreshold;

    /** Frame rate threshold for quality increase */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings", meta = (AllowPrivateAccess = "true"))
    float QualityIncreaseThreshold;

    /** Maximum physics bodies before optimization kicks in */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings", meta = (AllowPrivateAccess = "true"))
    int32 MaxPhysicsBodies;

    /** Current performance metrics */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics", meta = (AllowPrivateAccess = "true"))
    FCore_PhysicsPerformanceMetrics CurrentMetrics;

    /** Reference to physics systems integrator */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_PhysicsSystemsIntegrator> PhysicsIntegrator;

private:
    /** Time since last optimization check */
    float TimeSinceLastCheck;

    /** Frame rate samples for averaging */
    TArray<float> FrameRateSamples;

    /** Maximum number of frame rate samples */
    static const int32 MaxFrameRateSamples = 60;

    /** Update performance metrics */
    void UpdatePerformanceMetrics(float DeltaTime);

    /** Check if optimization is needed */
    bool ShouldOptimize() const;

    /** Apply quality level settings */
    void ApplyQualityLevel(ECore_PhysicsQualityLevel QualityLevel);

    /** Calculate average frame rate */
    float CalculateAverageFrameRate() const;

    /** Get platform-specific target frame rate */
    float GetPlatformTargetFrameRate() const;

    /** Log optimization action */
    void LogOptimization(const FString& Action) const;
};