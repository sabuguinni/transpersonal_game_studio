#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPhysicsOptimizationLevel : uint8
{
    None        UMETA(DisplayName = "No Optimization"),
    Light       UMETA(DisplayName = "Light Optimization"),
    Moderate    UMETA(DisplayName = "Moderate Optimization"),
    Aggressive  UMETA(DisplayName = "Aggressive Optimization"),
    Emergency   UMETA(DisplayName = "Emergency Optimization")
};

USTRUCT(BlueprintType)
struct FPhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsSubstepDeltaTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxPhysicsSubsteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float SolverIterationCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float CollisionDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableAsyncPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float SleepThresholdMultiplier;

    FPhysicsOptimizationSettings()
    {
        PhysicsSubstepDeltaTime = 0.016667f; // 60Hz
        MaxPhysicsSubsteps = 6;
        SolverIterationCount = 8;
        CollisionDistanceCulling = 1000.0f;
        bEnableAsyncPhysics = true;
        SleepThresholdMultiplier = 1.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsOptimizer();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Optimization Control
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetOptimizationLevel(EPhysicsOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ApplyCustomOptimizationSettings(const FPhysicsOptimizationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ResetToDefaultSettings();

    // Consciousness-Aware Optimizations
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeForConsciousnessDistance(float PlayerConsciousnessRadius);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetEmotionalPhysicsIntensity(float Intensity);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    float GetCurrentPhysicsTime() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    int32 GetActivePhysicsObjectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    bool IsPhysicsPerformanceOptimal() const;

    // LOD System for Physics
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UpdatePhysicsLOD();

protected:
    // Optimization settings for different levels
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization Presets")
    FPhysicsOptimizationSettings LightOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization Presets")
    FPhysicsOptimizationSettings ModerateOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization Presets")
    FPhysicsOptimizationSettings AggressiveOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization Presets")
    FPhysicsOptimizationSettings EmergencyOptimization;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EPhysicsOptimizationLevel CurrentOptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float TargetPhysicsTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float MaxAllowedPhysicsObjects;

    // Consciousness integration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consciousness Integration")
    float ConsciousnessRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consciousness Integration")
    float EmotionalIntensityMultiplier;

private:
    FPhysicsOptimizationSettings DefaultSettings;
    FPhysicsOptimizationSettings CurrentSettings;
    
    float LastPhysicsTime;
    int32 LastPhysicsObjectCount;
    float LastOptimizationUpdate;
    
    // Optimization methods
    void ApplyOptimizationSettings(const FPhysicsOptimizationSettings& Settings);
    void OptimizePhysicsObjectsInRadius(const FVector& Center, float Radius, float OptimizationFactor);
    void UpdateEmotionalPhysicsSettings();
    void CullDistantPhysicsObjects();
    void OptimizePhysicsSubstepping();
    
    // Performance tracking
    void UpdatePerformanceMetrics();
    bool ShouldIncreaseOptimization() const;
    bool ShouldDecreaseOptimization() const;
};