#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Perf_PhysicsArchitectIntegrator.generated.h"

// Forward declarations
class UCore_PhysicsArchitect;
class UTranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsIntegrationMetrics
{
    GENERATED_BODY()

    // Physics validation metrics from Core_PhysicsArchitect
    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    float PhysicsValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    int32 ValidatedCharacterCount;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    int32 ValidatedMovementComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    int32 ValidatedCollisionComponents;

    // Performance optimization metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float OptimizationOverhead;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsOptimizationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 OptimizedActorCount;

    // Integration health metrics
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationLatency;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 IntegrationErrorCount;

    FPerf_PhysicsIntegrationMetrics()
        : PhysicsValidationTime(0.0f)
        , ValidatedCharacterCount(0)
        , ValidatedMovementComponents(0)
        , ValidatedCollisionComponents(0)
        , OptimizationOverhead(0.0f)
        , PhysicsOptimizationTime(0.0f)
        , OptimizedActorCount(0)
        , bIntegrationHealthy(true)
        , IntegrationLatency(0.0f)
        , IntegrationErrorCount(0)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    // Physics optimization thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float MaxPhysicsTickTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxActiveRigidBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float MaxCollisionDetectionTime;

    // Character physics optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Optimization")
    float CharacterCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Optimization")
    bool bEnableCharacterPhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Optimization")
    int32 MaxSimulatedCharacters;

    // Movement optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Optimization")
    bool bOptimizeMovementComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Optimization")
    float MovementUpdateFrequency;

    FPerf_PhysicsOptimizationSettings()
        : MaxPhysicsTickTime(16.67f) // Target 60fps
        , MaxActiveRigidBodies(500)
        , MaxCollisionDetectionTime(5.0f)
        , CharacterCullingDistance(5000.0f)
        , bEnableCharacterPhysicsLOD(true)
        , MaxSimulatedCharacters(50)
        , bOptimizeMovementComponents(true)
        , MovementUpdateFrequency(60.0f)
    {
    }
};

/**
 * Performance Optimizer component that integrates with Core_PhysicsArchitect
 * to provide physics performance optimization based on validation metrics.
 * Monitors physics validation data and applies targeted optimizations.
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsArchitectIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsArchitectIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics architect integration
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void IntegrateWithPhysicsArchitect();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ProcessPhysicsValidationData();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    bool IsPhysicsArchitectAvailable() const;

    // Performance optimization methods
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeCharacterPhysics();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeMovementComponents();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeCollisionSystems();

    // Metrics and monitoring
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FPerf_PhysicsIntegrationMetrics GetIntegrationMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdatePerformanceMetrics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    bool ValidateIntegrationHealth() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetOptimizationSettings(const FPerf_PhysicsOptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    FPerf_PhysicsOptimizationSettings GetOptimizationSettings() const;

    // Debug and validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPhysicsIntegration();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidatePhysicsOptimization();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogIntegrationStatus();

protected:
    // Core integration data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    FPerf_PhysicsIntegrationMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    // Physics architect reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<UCore_PhysicsArchitect> PhysicsArchitectRef;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float LastOptimizationTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 OptimizationCycleCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bOptimizationActive;

private:
    // Internal optimization methods
    void FindPhysicsArchitectComponent();
    void ProcessValidationMetrics();
    void ApplyPhysicsOptimizations();
    void UpdateIntegrationHealth();

    // Performance optimization internals
    void CullDistantCharacters();
    void OptimizeRigidBodySimulation();
    void AdjustCollisionSettings();
    void ManagePhysicsLOD();

    // Metrics calculation
    void CalculateOptimizationOverhead(float StartTime);
    void UpdateIntegrationLatency(float DeltaTime);
    void TrackOptimizationEffectiveness();
};