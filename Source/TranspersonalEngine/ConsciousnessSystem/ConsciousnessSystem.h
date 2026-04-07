#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "ConsciousnessSystem.generated.h"

/**
 * @file ConsciousnessSystem.h
 * @brief Core consciousness state machine for transpersonal psychology gameplay
 * @author Core Systems Programmer
 * @version 1.0
 * 
 * This system manages the player's consciousness states, transitions between
 * different levels of awareness, and the underlying mechanics that drive
 * transpersonal experiences in the game world.
 */

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    /** Normal waking consciousness - baseline state */
    Ordinary         UMETA(DisplayName = "Ordinary Consciousness"),
    
    /** Meditative state - increased awareness, reduced ego */
    Meditative       UMETA(DisplayName = "Meditative State"),
    
    /** Flow state - optimal performance, time distortion */
    Flow            UMETA(DisplayName = "Flow State"),
    
    /** Transcendent state - ego dissolution, unity experience */
    Transcendent    UMETA(DisplayName = "Transcendent State"),
    
    /** Shadow integration - confronting unconscious aspects */
    Shadow          UMETA(DisplayName = "Shadow Integration"),
    
    /** Archetypal - connection to collective unconscious */
    Archetypal      UMETA(DisplayName = "Archetypal State"),
    
    /** Integration - synthesizing transpersonal insights */
    Integration     UMETA(DisplayName = "Integration State")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALENGINE_API FConsciousnessMetrics
{
    GENERATED_BODY()

    /** Current awareness level (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float AwarenessLevel = 0.5f;

    /** Ego dissolution factor (0.0 = full ego, 1.0 = ego death) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float EgoDissolution = 0.0f;

    /** Connection to transpersonal realms (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float TranspersonalConnection = 0.0f;

    /** Meditation depth (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float MeditationDepth = 0.0f;

    /** Shadow integration progress (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ShadowIntegration = 0.0f;

    FConsciousnessMetrics()
    {
        AwarenessLevel = 0.5f;
        EgoDissolution = 0.0f;
        TranspersonalConnection = 0.0f;
        MeditationDepth = 0.0f;
        ShadowIntegration = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsciousnessStateChanged, EConsciousnessState, OldState, EConsciousnessState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeditationDepthChanged, float, NewDepth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTranscendentExperience, float, Intensity);

/**
 * @class UConsciousnessSystem
 * @brief Core component managing consciousness states and transpersonal experiences
 * 
 * This system implements a sophisticated state machine for consciousness states,
 * meditation mechanics, and the gradual progression through transpersonal experiences.
 * It serves as the foundation for all consciousness-related gameplay mechanics.
 */
UCLASS(ClassGroup=(TranspersonalEngine), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALENGINE_API UConsciousnessSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === STATE MANAGEMENT ===
    
    /** Get current consciousness state */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    EConsciousnessState GetCurrentState() const { return CurrentState; }

    /** Attempt to transition to a new consciousness state */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    bool TransitionToState(EConsciousnessState NewState, bool bForceTransition = false);

    /** Check if transition to target state is possible */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    bool CanTransitionToState(EConsciousnessState TargetState) const;

    // === MEDITATION MECHANICS ===
    
    /** Begin meditation session */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void BeginMeditation();

    /** End meditation session */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void EndMeditation();

    /** Update meditation focus (0.0 = distracted, 1.0 = perfect focus) */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void UpdateMeditationFocus(float FocusLevel);

    /** Check if currently meditating */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    bool IsMeditating() const { return bIsMeditating; }

    // === METRICS ACCESS ===
    
    /** Get current consciousness metrics */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    FConsciousnessMetrics GetConsciousnessMetrics() const { return Metrics; }

    /** Modify awareness level directly (for external systems) */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void ModifyAwareness(float Delta, float MaxChange = 0.1f);

    /** Trigger transcendent experience */
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void TriggerTranscendentExperience(float Intensity = 1.0f);

    // === EVENTS ===
    
    /** Fired when consciousness state changes */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnConsciousnessStateChanged OnStateChanged;

    /** Fired when meditation depth changes significantly */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMeditationDepthChanged OnMeditationDepthChanged;

    /** Fired when transcendent experience occurs */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTranscendentExperience OnTranscendentExperience;

protected:
    // === CORE STATE ===
    
    /** Current consciousness state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EConsciousnessState CurrentState;

    /** Previous consciousness state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EConsciousnessState PreviousState;

    /** Current consciousness metrics */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    FConsciousnessMetrics Metrics;

    // === MEDITATION STATE ===
    
    /** Whether currently in meditation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meditation")
    bool bIsMeditating;

    /** Current meditation session duration */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meditation")
    float MeditationDuration;

    /** Current focus level during meditation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meditation")
    float CurrentFocus;

    // === CONFIGURATION ===
    
    /** Rate of awareness decay when not actively maintained */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float AwarenessDecayRate = 0.1f;

    /** Rate of meditation depth increase */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MeditationDepthRate = 0.2f;

    /** Minimum meditation depth for state transitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MinMeditationDepthForTransition = 0.3f;

    /** Time required in meditation before deeper states become available */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MeditationTimeThreshold = 60.0f;

private:
    // === INTERNAL METHODS ===
    
    /** Update consciousness metrics based on current state and conditions */
    void UpdateConsciousnessMetrics(float DeltaTime);

    /** Update meditation progress */
    void UpdateMeditation(float DeltaTime);

    /** Process state transitions and validate them */
    void ProcessStateTransitions(float DeltaTime);

    /** Calculate state transition probability */
    float CalculateTransitionProbability(EConsciousnessState FromState, EConsciousnessState ToState) const;

    /** Apply state-specific effects */
    void ApplyStateEffects(EConsciousnessState State, float DeltaTime);

    /** Last time metrics were significantly updated */
    float LastMetricsUpdateTime;

    /** Accumulated meditation focus for this session */
    float AccumulatedFocus;
};