/**
 * @file ConsciousnessPhysics.h
 * @brief Core physics system for consciousness expansion mechanics
 * @author Core Systems Programmer
 * @version 1.0
 * 
 * Implements multi-layered physics simulation for astral projection,
 * reality distortion, and temporal mechanics in transpersonal gameplay.
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
    Etheric     UMETA(DisplayName = "Etheric Layer"),
    Astral      UMETA(DisplayName = "Astral Plane"),
    Mental      UMETA(DisplayName = "Mental Realm"),
    Causal      UMETA(DisplayName = "Causal Dimension")
};

USTRUCT(BlueprintType)
struct FConsciousnessPhysicsParams
{
    GENERATED_BODY()

    /** Gravity multiplier for this consciousness layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float GravityMultiplier = 1.0f;

    /** Time dilation factor for temporal mechanics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float TimeDilation = 1.0f;

    /** Physics material override for layer-specific interactions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UPhysicalMaterial* LayerPhysicsMaterial = nullptr;

    /** Collision response override */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TEnumAsByte<ECollisionResponse> CollisionResponse = ECR_Block;

    FConsciousnessPhysicsParams()
    {
        GravityMultiplier = 1.0f;
        TimeDilation = 1.0f;
        LayerPhysicsMaterial = nullptr;
        CollisionResponse = ECR_Block;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLayerTransition, EConsciousnessLayer, FromLayer, EConsciousnessLayer, ToLayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTemporalShift, float, NewTimeDilation);

/**
 * @class UConsciousnessPhysicsComponent
 * @brief Component that manages consciousness-based physics interactions
 * 
 * This component allows actors to exist and interact across multiple
 * consciousness layers with different physics properties.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessPhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Current consciousness layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EConsciousnessLayer CurrentLayer = EConsciousnessLayer::Physical;

    /** Physics parameters for each consciousness layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    TMap<EConsciousnessLayer, FConsciousnessPhysicsParams> LayerParams;

    /** Whether this actor can transition between layers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    bool bCanTransitionLayers = true;

    /** Transition duration in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float TransitionDuration = 1.0f;

    /** Events */
    UPROPERTY(BlueprintAssignable, Category = "Consciousness Events")
    FOnLayerTransition OnLayerTransition;

    UPROPERTY(BlueprintAssignable, Category = "Consciousness Events")
    FOnTemporalShift OnTemporalShift;

    /**
     * @brief Transition to a different consciousness layer
     * @param TargetLayer The layer to transition to
     * @param bInstant Whether to transition instantly or over time
     * @return True if transition was initiated successfully
     */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    bool TransitionToLayer(EConsciousnessLayer TargetLayer, bool bInstant = false);

    /**
     * @brief Get current effective gravity for this actor
     * @return Gravity vector adjusted for current consciousness layer
     */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    FVector GetEffectiveGravity() const;

    /**
     * @brief Get current time dilation factor
     * @return Time dilation multiplier for current layer
     */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    float GetCurrentTimeDilation() const;

    /**
     * @brief Check if actor can interact with objects in target layer
     * @param TargetLayer Layer to check interaction with
     * @return True if interaction is possible
     */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    bool CanInteractWithLayer(EConsciousnessLayer TargetLayer) const;

    /**
     * @brief Apply consciousness-based force to actor
     * @param Force Force vector to apply
     * @param bAccelChange Whether to treat as acceleration change
     */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void ApplyConsciousnessForce(const FVector& Force, bool bAccelChange = false);

private:
    /** Current transition state */
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    EConsciousnessLayer TransitionTarget = EConsciousnessLayer::Physical;

    /** Cached physics components */
    UPROPERTY()
    class UPrimitiveComponent* CachedPrimitiveComponent = nullptr;

    /** Original physics settings for restoration */
    float OriginalGravityScale = 1.0f;
    TEnumAsByte<ECollisionResponse> OriginalCollisionResponse = ECR_Block;

    /** Internal methods */
    void UpdatePhysicsForCurrentLayer();
    void ProcessLayerTransition(float DeltaTime);
    void ApplyLayerPhysicsSettings(const FConsciousnessPhysicsParams& Params);
    void InitializeLayerDefaults();
};

/**
 * @class AConsciousnessPhysicsManager
 * @brief World-level manager for consciousness physics systems
 * 
 * Manages global consciousness physics state, layer interactions,
 * and performance optimization for large-scale simulations.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AConsciousnessPhysicsManager : public AActor
{
    GENERATED_BODY()

public:
    AConsciousnessPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    /** Global consciousness layer visibility settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    TMap<EConsciousnessLayer, bool> LayerVisibility;

    /** Maximum number of actors that can be in non-physical layers simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "10", ClampMax = "500"))
    int32 MaxNonPhysicalActors = 100;

    /** Performance budget in milliseconds per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.5", ClampMax = "10.0"))
    float PerformanceBudgetMS = 2.0f;

    /**
     * @brief Get all actors currently in specified consciousness layer
     * @param Layer Target consciousness layer
     * @return Array of actors in the layer
     */
    UFUNCTION(BlueprintCallable, Category = "Consciousness Management")
    TArray<AActor*> GetActorsInLayer(EConsciousnessLayer Layer) const;

    /**
     * @brief Force all actors to transition to physical layer (emergency fallback)
     */
    UFUNCTION(BlueprintCallable, Category = "Consciousness Management")
    void ForceAllToPhysicalLayer();

    /**
     * @brief Get current performance metrics
     * @param OutFrameTimeMS Current frame time in milliseconds
     * @param OutActorCount Number of consciousness-enabled actors
     * @param OutTransitionCount Number of active transitions
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void GetPerformanceMetrics(float& OutFrameTimeMS, int32& OutActorCount, int32& OutTransitionCount) const;

    /**
     * @brief Enable or disable layer visibility globally
     * @param Layer Target layer
     * @param bVisible Whether layer should be visible
     */
    UFUNCTION(BlueprintCallable, Category = "Consciousness Management")
    void SetLayerVisibility(EConsciousnessLayer Layer, bool bVisible);

private:
    /** Tracked consciousness components for performance monitoring */
    UPROPERTY()
    TArray<UConsciousnessPhysicsComponent*> TrackedComponents;

    /** Performance tracking */
    float LastFrameTime = 0.0f;
    int32 ActiveTransitions = 0;

    /** Internal methods */
    void UpdateTrackedComponents();
    void OptimizePerformance();
    void RegisterComponent(UConsciousnessPhysicsComponent* Component);
    void UnregisterComponent(UConsciousnessPhysicsComponent* Component);

    friend class UConsciousnessPhysicsComponent;
};