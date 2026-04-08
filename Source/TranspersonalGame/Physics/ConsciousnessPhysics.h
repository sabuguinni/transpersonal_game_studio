/**
 * @file ConsciousnessPhysics.h
 * @brief Core physics system for consciousness expansion mechanics
 * @author Core Systems Programmer
 * @version 1.0
 * 
 * Implements multi-layered reality physics with temporal manipulation
 * and astral projection mechanics. Designed for 60 FPS performance
 * on RTX 3070 / RX 6700 XT target hardware.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "ConsciousnessPhysics.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessLayer : uint8
{
    Physical    UMETA(DisplayName = "Physical Reality"),
    Emotional   UMETA(DisplayName = "Emotional Layer"),
    Mental      UMETA(DisplayName = "Mental Layer"),
    Spiritual   UMETA(DisplayName = "Spiritual Layer"),
    Astral      UMETA(DisplayName = "Astral Projection")
};

USTRUCT(BlueprintType)
struct FConsciousnessPhysicsSettings
{
    GENERATED_BODY()

    /** Gravity multiplier for each consciousness layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Physics")
    TMap<EConsciousnessLayer, float> LayerGravityMultipliers;

    /** Time dilation factor for temporal mechanics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temporal", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TimeDilationFactor = 1.0f;

    /** Maximum astral projection distance from physical body */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Astral", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float MaxAstralDistance = 5000.0f;

    /** Physics simulation substeps for consciousness layers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "8"))
    int32 ConsciousnessSubsteps = 4;

    FConsciousnessPhysicsSettings()
    {
        LayerGravityMultipliers.Add(EConsciousnessLayer::Physical, 1.0f);
        LayerGravityMultipliers.Add(EConsciousnessLayer::Emotional, 0.8f);
        LayerGravityMultipliers.Add(EConsciousnessLayer::Mental, 0.6f);
        LayerGravityMultipliers.Add(EConsciousnessLayer::Spiritual, 0.3f);
        LayerGravityMultipliers.Add(EConsciousnessLayer::Astral, 0.1f);
    }
};

USTRUCT(BlueprintType)
struct FLayerPhysicsState
{
    GENERATED_BODY()

    /** Current active layer */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EConsciousnessLayer ActiveLayer = EConsciousnessLayer::Physical;

    /** Transition progress between layers (0.0 to 1.0) */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TransitionProgress = 0.0f;

    /** Is currently transitioning between layers */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsTransitioning = false;

    /** Layer-specific velocity modifiers */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    TMap<EConsciousnessLayer, FVector> LayerVelocities;
};

/**
 * @class UConsciousnessPhysicsComponent
 * @brief Manages physics behaviour across consciousness layers
 * 
 * This component handles the physics simulation for objects that can
 * exist across multiple layers of reality. It provides smooth transitions
 * between layers and maintains performance through optimized calculations.
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessPhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Initialize consciousness physics for this actor */
    UFUNCTION(BlueprintCallable, Category = "Consciousness Physics")
    void InitializeConsciousnessPhysics();

    /** Transition to a specific consciousness layer */
    UFUNCTION(BlueprintCallable, Category = "Consciousness Physics")
    void TransitionToLayer(EConsciousnessLayer TargetLayer, float TransitionDuration = 2.0f);

    /** Get current active consciousness layer */
    UFUNCTION(BlueprintPure, Category = "Consciousness Physics")
    EConsciousnessLayer GetActiveLayer() const { return PhysicsState.ActiveLayer; }

    /** Check if currently transitioning between layers */
    UFUNCTION(BlueprintPure, Category = "Consciousness Physics")
    bool IsTransitioning() const { return PhysicsState.bIsTransitioning; }

    /** Apply temporal dilation effect */
    UFUNCTION(BlueprintCallable, Category = "Temporal Physics")
    void ApplyTemporalDilation(float DilationFactor, float Duration);

    /** Enable astral projection physics */
    UFUNCTION(BlueprintCallable, Category = "Astral Physics")
    void EnableAstralProjection(const FVector& AstralStartLocation);

    /** Disable astral projection and return to physical body */
    UFUNCTION(BlueprintCallable, Category = "Astral Physics")
    void DisableAstralProjection();

    /** Get physics settings */
    UFUNCTION(BlueprintPure, Category = "Consciousness Physics")
    const FConsciousnessPhysicsSettings& GetPhysicsSettings() const { return PhysicsSettings; }

    /** Set physics settings */
    UFUNCTION(BlueprintCallable, Category = "Consciousness Physics")
    void SetPhysicsSettings(const FConsciousnessPhysicsSettings& NewSettings);

protected:
    /** Physics settings for consciousness mechanics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FConsciousnessPhysicsSettings PhysicsSettings;

    /** Current physics state */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FLayerPhysicsState PhysicsState;

    /** Target layer for transitions */
    UPROPERTY()
    EConsciousnessLayer TargetLayer = EConsciousnessLayer::Physical;

    /** Transition duration */
    UPROPERTY()
    float TransitionDuration = 2.0f;

    /** Transition timer */
    UPROPERTY()
    float TransitionTimer = 0.0f;

    /** Physical body location for astral projection */
    UPROPERTY()
    FVector PhysicalBodyLocation = FVector::ZeroVector;

    /** Is astral projection active */
    UPROPERTY()
    bool bAstralProjectionActive = false;

    /** Temporal dilation timer */
    UPROPERTY()
    float TemporalDilationTimer = 0.0f;

    /** Original time dilation */
    UPROPERTY()
    float OriginalTimeDilation = 1.0f;

private:
    /** Update layer transition */
    void UpdateLayerTransition(float DeltaTime);

    /** Apply layer-specific physics */
    void ApplyLayerPhysics(float DeltaTime);

    /** Update temporal effects */
    void UpdateTemporalEffects(float DeltaTime);

    /** Validate astral projection distance */
    void ValidateAstralDistance();

    /** Calculate layer-specific gravity */
    float CalculateLayerGravity(EConsciousnessLayer Layer) const;

    /** Interpolate between layer physics */
    FVector InterpolateLayerVelocity(EConsciousnessLayer FromLayer, EConsciousnessLayer ToLayer, float Alpha) const;
};

/**
 * @class AConsciousnessPhysicsManager
 * @brief World-level manager for consciousness physics systems
 * 
 * Singleton manager that coordinates consciousness physics across
 * all actors in the world. Handles performance optimization and
 * layer synchronization.
 */
UCLASS()
class TRANSPERSONALGAME_API AConsciousnessPhysicsManager : public AActor
{
    GENERATED_BODY()

public:
    AConsciousnessPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    /** Get the singleton instance */
    UFUNCTION(BlueprintPure, Category = "Consciousness Physics", CallInEditor = true)
    static AConsciousnessPhysicsManager* GetInstance(const UObject* WorldContext);

    /** Register a consciousness physics component */
    UFUNCTION(BlueprintCallable, Category = "Consciousness Physics")
    void RegisterConsciousnessComponent(UConsciousnessPhysicsComponent* Component);

    /** Unregister a consciousness physics component */
    UFUNCTION(BlueprintCallable, Category = "Consciousness Physics")
    void UnregisterConsciousnessComponent(UConsciousnessPhysicsComponent* Component);

    /** Set global consciousness layer for all registered components */
    UFUNCTION(BlueprintCallable, Category = "Consciousness Physics")
    void SetGlobalConsciousnessLayer(EConsciousnessLayer Layer);

    /** Get performance metrics */
    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetConsciousnessPhysicsFrameTime() const { return FrameTimeMs; }

    /** Enable/disable consciousness physics globally */
    UFUNCTION(BlueprintCallable, Category = "Consciousness Physics")
    void SetConsciousnessPhysicsEnabled(bool bEnabled);

protected:
    /** Registered consciousness physics components */
    UPROPERTY()
    TArray<UConsciousnessPhysicsComponent*> RegisteredComponents;

    /** Global consciousness physics settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    FConsciousnessPhysicsSettings GlobalSettings;

    /** Is consciousness physics enabled globally */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bConsciousnessPhysicsEnabled = true;

    /** Performance monitoring */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMs = 0.0f;

    /** Maximum allowed frame time for consciousness physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "16.0"))
    float MaxFrameTimeMs = 2.0f;

private:
    /** Singleton instance */
    static AConsciousnessPhysicsManager* Instance;

    /** Performance timer */
    double PerformanceTimer = 0.0;

    /** Update all registered components */
    void UpdateRegisteredComponents(float DeltaTime);

    /** Monitor performance */
    void MonitorPerformance();

    /** Optimize performance if needed */
    void OptimizePerformance();
};