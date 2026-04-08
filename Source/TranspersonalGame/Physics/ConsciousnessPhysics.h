/**
 * @file ConsciousnessPhysics.h
 * @brief Core physics system for consciousness expansion mechanics
 * 
 * Implements specialized physics for astral projection, reality layer transitions,
 * and temporal mechanics. Designed for 60 FPS performance on RTX 3070 / RX 6700 XT.
 * 
 * @author Core Systems Programmer
 * @version 1.0
 * @date 2024
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
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

UENUM(BlueprintType)
enum class ETemporalState : uint8
{
    Normal      UMETA(DisplayName = "Normal Time"),
    Accelerated UMETA(DisplayName = "Accelerated Time"),
    Decelerated UMETA(DisplayName = "Decelerated Time"),
    Frozen      UMETA(DisplayName = "Time Frozen"),
    Reversed    UMETA(DisplayName = "Time Reversed")
};

USTRUCT(BlueprintType)
struct FConsciousnessPhysicsSettings
{
    GENERATED_BODY()

    /** Gravity multiplier for each consciousness layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Physics")
    TMap<EConsciousnessLayer, float> GravityMultipliers;

    /** Air resistance coefficients per layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Physics")
    TMap<EConsciousnessLayer, float> AirResistance;

    /** Maximum velocity in each layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Physics")
    TMap<EConsciousnessLayer, float> MaxVelocities;

    /** Temporal distortion strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temporal", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TemporalDistortionStrength = 1.0f;

    /** Astral projection movement speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Astral", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float AstralMovementSpeed = 1200.0f;

    /** Phase transition duration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float PhaseTransitionTime = 2.0f;

    FConsciousnessPhysicsSettings()
    {
        // Initialize default values
        GravityMultipliers.Add(EConsciousnessLayer::Physical, 1.0f);
        GravityMultipliers.Add(EConsciousnessLayer::Etheric, 0.7f);
        GravityMultipliers.Add(EConsciousnessLayer::Astral, 0.3f);
        GravityMultipliers.Add(EConsciousnessLayer::Mental, 0.1f);
        GravityMultipliers.Add(EConsciousnessLayer::Causal, 0.0f);

        AirResistance.Add(EConsciousnessLayer::Physical, 1.0f);
        AirResistance.Add(EConsciousnessLayer::Etheric, 0.8f);
        AirResistance.Add(EConsciousnessLayer::Astral, 0.5f);
        AirResistance.Add(EConsciousnessLayer::Mental, 0.2f);
        AirResistance.Add(EConsciousnessLayer::Causal, 0.1f);

        MaxVelocities.Add(EConsciousnessLayer::Physical, 2000.0f);
        MaxVelocities.Add(EConsciousnessLayer::Etheric, 3000.0f);
        MaxVelocities.Add(EConsciousnessLayer::Astral, 5000.0f);
        MaxVelocities.Add(EConsciousnessLayer::Mental, 8000.0f);
        MaxVelocities.Add(EConsciousnessLayer::Causal, 15000.0f);
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLayerTransition, EConsciousnessLayer, FromLayer, EConsciousnessLayer, ToLayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTemporalStateChanged, ETemporalState, OldState, ETemporalState, NewState);

/**
 * @class UConsciousnessPhysicsComponent
 * @brief Manages physics behavior for consciousness expansion mechanics
 * 
 * This component handles:
 * - Layer-specific physics properties
 * - Astral projection movement
 * - Temporal mechanics
 * - Reality transition effects
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessPhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Physics settings for consciousness mechanics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Physics")
    FConsciousnessPhysicsSettings PhysicsSettings;

    /** Current consciousness layer */
    UPROPERTY(BlueprintReadOnly, Category = "Consciousness State")
    EConsciousnessLayer CurrentLayer = EConsciousnessLayer::Physical;

    /** Current temporal state */
    UPROPERTY(BlueprintReadOnly, Category = "Temporal State")
    ETemporalState CurrentTemporalState = ETemporalState::Normal;

    /** Is actor in astral projection mode */
    UPROPERTY(BlueprintReadOnly, Category = "Astral State")
    bool bIsAstralProjecting = false;

    /** Layer transition events */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLayerTransition OnLayerTransition;

    /** Temporal state change events */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTemporalStateChanged OnTemporalStateChanged;

    /**
     * Transition to a different consciousness layer
     * @param TargetLayer The layer to transition to
     * @param bInstant Whether to transition instantly or over time
     */
    UFUNCTION(BlueprintCallable, Category = "Consciousness Physics")
    void TransitionToLayer(EConsciousnessLayer TargetLayer, bool bInstant = false);

    /**
     * Begin astral projection
     * @param ProjectionDirection Initial direction for astral body
     */
    UFUNCTION(BlueprintCallable, Category = "Astral Physics")
    void BeginAstralProjection(const FVector& ProjectionDirection = FVector::ZeroVector);

    /**
     * End astral projection and return to physical body
     */
    UFUNCTION(BlueprintCallable, Category = "Astral Physics")
    void EndAstralProjection();

    /**
     * Set temporal state for time manipulation
     * @param NewTemporalState The new temporal state
     * @param Duration How long the effect lasts (-1 for permanent)
     */
    UFUNCTION(BlueprintCallable, Category = "Temporal Physics")
    void SetTemporalState(ETemporalState NewTemporalState, float Duration = -1.0f);

    /**
     * Get effective gravity for current layer
     */
    UFUNCTION(BlueprintPure, Category = "Consciousness Physics")
    float GetEffectiveGravity() const;

    /**
     * Get effective time dilation factor
     */
    UFUNCTION(BlueprintPure, Category = "Temporal Physics")
    float GetTimeDilationFactor() const;

    /**
     * Check if actor can interact with objects in current layer
     */
    UFUNCTION(BlueprintPure, Category = "Consciousness Physics")
    bool CanInteractWithPhysicalObjects() const;

private:
    /** Timer handle for layer transitions */
    FTimerHandle LayerTransitionTimer;

    /** Timer handle for temporal effects */
    FTimerHandle TemporalEffectTimer;

    /** Target layer for ongoing transition */
    EConsciousnessLayer TargetTransitionLayer;

    /** Transition progress (0.0 to 1.0) */
    float TransitionProgress = 0.0f;

    /** Original physics settings before modifications */
    TMap<FString, float> OriginalPhysicsSettings;

    /** Cached primitive component for physics modifications */
    UPROPERTY()
    class UPrimitiveComponent* CachedPrimitiveComponent;

    /** Apply physics settings for current layer */
    void ApplyLayerPhysics();

    /** Update transition progress */
    void UpdateLayerTransition(float DeltaTime);

    /** Complete layer transition */
    void CompleteLayerTransition();

    /** Reset temporal state to normal */
    void ResetTemporalState();

    /** Update astral projection physics */
    void UpdateAstralProjection(float DeltaTime);
};