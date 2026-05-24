#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Core_PhysicsSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECore_SimulationQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Realistic   UMETA(DisplayName = "Realistic Physics"),
    Arcade      UMETA(DisplayName = "Arcade Physics"),
    Cinematic   UMETA(DisplayName = "Cinematic Physics")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_SimulationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    ECore_SimulationQuality Quality = ECore_SimulationQuality::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    ECore_PhysicsMode PhysicsMode = ECore_PhysicsMode::Realistic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float TimeStep = 0.016667f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    int32 MaxSubSteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float MaxSubStepDeltaTime = 0.008333f; // 120 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimulatedBodies = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 5000.0f;

    FCore_SimulationSettings()
    {
        Quality = ECore_SimulationQuality::Medium;
        PhysicsMode = ECore_PhysicsMode::Realistic;
        TimeStep = 0.016667f;
        MaxSubSteps = 6;
        MaxSubStepDeltaTime = 0.008333f;
        MaxSimulatedBodies = 1000;
        CullingDistance = 5000.0f;
    }
};

/**
 * Core Physics Simulation Manager
 * Manages physics simulation parameters, quality settings, and performance optimization
 * for the prehistoric survival game environment
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Simulation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation")
    FCore_SimulationSettings SimulationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation")
    bool bEnablePhysicsSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation")
    bool bEnableAsyncPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation")
    bool bEnableSubstepping = true;

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentSimulatedBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentPhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AveragePhysicsTime = 0.0f;

    // Simulation Control Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void SetSimulationQuality(ECore_SimulationQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void SetPhysicsMode(ECore_PhysicsMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void ApplySimulationSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void PausePhysicsSimulation();

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void ResumePhysicsSimulation();

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void ResetPhysicsSimulation();

    // Performance Functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetPhysicsFrameRate() const;

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPhysicsPerformanceGood() const;

    // Diagnostic Functions
    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    void LogSimulationStatus();

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    void DumpPhysicsStats();

private:
    // Internal state
    bool bSimulationPaused = false;
    float PhysicsTimeAccumulator = 0.0f;
    int32 PhysicsFrameCounter = 0;
    TArray<float> PhysicsTimeSamples;
    
    // Performance tracking
    float LastPerformanceCheck = 0.0f;
    const float PerformanceCheckInterval = 1.0f;
    
    // Internal functions
    void UpdatePhysicsSettings();
    void MonitorPerformance(float DeltaTime);
    void AdjustQualityBasedOnPerformance();
    ECore_SimulationQuality GetOptimalQualityForPerformance() const;
};