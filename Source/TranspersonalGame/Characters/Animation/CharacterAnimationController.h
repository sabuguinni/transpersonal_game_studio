#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "MotionMatchingSystem.h"
#include "AdaptiveIKSystem.h"
#include "CharacterAnimationController.generated.h"

class UAnimSequence;
class UBlendSpace;
class UAnimMontage;

/**
 * Animation state for the character
 */
UENUM(BlueprintType)
enum class ECharacterAnimationState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Jumping         UMETA(DisplayName = "Jumping"),
    Falling         UMETA(DisplayName = "Falling"),
    Landing         UMETA(DisplayName = "Landing"),
    Interacting     UMETA(DisplayName = "Interacting"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Hiding          UMETA(DisplayName = "Hiding"),
    Injured         UMETA(DisplayName = "Injured"),
    Dead            UMETA(DisplayName = "Dead")
};

/**
 * Emotional animation modifiers
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEmotionalAnimationState
{
    GENERATED_BODY()

    // Fear level affects posture and movement speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearLevel;

    // Alertness affects head movement and reaction animations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AlertnessLevel;

    // Stress affects breathing and micro-movements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StressLevel;

    // Confidence affects posture and stride length
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConfidenceLevel;

    // Fatigue affects animation speed and posture
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FatigueLevel;

    FEmotionalAnimationState()
    {
        FearLevel = 0.0f;
        AlertnessLevel = 0.5f;
        StressLevel = 0.0f;
        ConfidenceLevel = 0.5f;
        FatigueLevel = 0.0f;
    }
};

/**
 * Animation layer weights for blending different animation systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnimationLayerWeights
{
    GENERATED_BODY()

    // Base locomotion weight
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LocomotionWeight;

    // Upper body additive weight
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float UpperBodyWeight;

    // Facial animation weight
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FacialWeight;

    // IK system weight
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IKWeight;

    // Emotional overlay weight
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmotionalWeight;

    FAnimationLayerWeights()
    {
        LocomotionWeight = 1.0f;
        UpperBodyWeight = 1.0f;
        FacialWeight = 1.0f;
        IKWeight = 1.0f;
        EmotionalWeight = 0.5f;
    }
};

/**
 * Character Animation Controller
 * Central hub for managing all character animation systems
 * Coordinates Motion Matching, IK, emotional states, and archetype behaviors
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetAnimationState(ECharacterAnimationState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation State")
    ECharacterAnimationState GetAnimationState() const { return CurrentAnimationState; }

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void TransitionToState(ECharacterAnimationState NewState, float TransitionTime = 0.2f);

    // Emotional state management
    UFUNCTION(BlueprintCallable, Category = "Emotional Animation")
    void SetEmotionalState(const FEmotionalAnimationState& NewEmotionalState);

    UFUNCTION(BlueprintPure, Category = "Emotional Animation")
    FEmotionalAnimationState GetEmotionalState() const { return EmotionalState; }

    UFUNCTION(BlueprintCallable, Category = "Emotional Animation")
    void ModifyEmotionalState(float DeltaFear, float DeltaAlertness, float DeltaStress, float DeltaConfidence, float DeltaFatigue);

    // Character archetype integration
    UFUNCTION(BlueprintCallable, Category = "Character Archetype")
    void SetCharacterArchetype(const FString& ArchetypeName);

    UFUNCTION(BlueprintPure, Category = "Character Archetype")
    FString GetCharacterArchetype() const { return CurrentArchetype; }

    // Animation layer management
    UFUNCTION(BlueprintCallable, Category = "Animation Layers")
    void SetLayerWeights(const FAnimationLayerWeights& NewWeights);

    UFUNCTION(BlueprintPure, Category = "Animation Layers")
    FAnimationLayerWeights GetLayerWeights() const { return LayerWeights; }

    // Motion Matching integration
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void EnableMotionMatching(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsMotionMatchingEnabled() const;

    // IK system integration
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void EnableAdaptiveIK(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "IK System")
    bool IsAdaptiveIKEnabled() const;

    // Environmental awareness
    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    void SetEnvironmentalFactors(bool bOnUnevenTerrain, bool bNearDanger, bool bInWater, float Temperature);

    UFUNCTION(BlueprintCallable, Category = "Environmental Animation")
    void SetWeatherConditions(const FString& WeatherType, float Intensity);

    // Interaction animations
    UFUNCTION(BlueprintCallable, Category = "Interaction Animation")
    void StartInteractionAnimation(const FString& InteractionType, UObject* InteractionTarget = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Interaction Animation")
    void EndInteractionAnimation();

    UFUNCTION(BlueprintPure, Category = "Interaction Animation")
    bool IsPlayingInteractionAnimation() const { return bIsPlayingInteraction; }

    // Survival state animations
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void SetSurvivalState(float Health, float Hunger, float Thirst, float Fatigue);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void TriggerInjuryAnimation(const FString& InjuryType, float Severity);

    // Combat animations
    UFUNCTION(BlueprintCallable, Category = "Combat Animation")
    void SetCombatStance(bool bInCombat, const FString& WeaponType = "");

    UFUNCTION(BlueprintCallable, Category = "Combat Animation")
    void TriggerAttackAnimation(const FString& AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat Animation")
    void TriggerDefenseAnimation(const FString& DefenseType);

    // Animation Blueprint integration
    UFUNCTION(BlueprintPure, Category = "Animation Blueprint")
    float GetAnimationParameter(const FString& ParameterName) const;

    UFUNCTION(BlueprintCallable, Category = "Animation Blueprint")
    void SetAnimationParameter(const FString& ParameterName, float Value);

    UFUNCTION(BlueprintPure, Category = "Animation Blueprint")
    FAdaptiveIKData GetIKData() const;

    // Debugging and visualization
    UFUNCTION(BlueprintCallable, Category = "Animation Debug")
    void SetDebugMode(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Animation Debug")
    void DebugDrawAnimationState() const;

    UFUNCTION(BlueprintPure, Category = "Animation Debug")
    FString GetDebugString() const;

protected:
    // Core state
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    ECharacterAnimationState CurrentAnimationState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    ECharacterAnimationState PreviousAnimationState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CurrentArchetype;

    // Emotional state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Animation")
    FEmotionalAnimationState EmotionalState;

    // Animation layers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Layers")
    FAnimationLayerWeights LayerWeights;

    // System components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UMotionMatchingSystem* MotionMatchingSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAdaptiveIKSystem* AdaptiveIKSystem;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Environmental state
    UPROPERTY(BlueprintReadOnly, Category = "Environmental")
    bool bOnUnevenTerrain;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental")
    bool bNearDanger;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental")
    bool bInWater;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental")
    float EnvironmentTemperature;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental")
    FString CurrentWeatherType;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental")
    float WeatherIntensity;

    // Interaction state
    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    bool bIsPlayingInteraction;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    FString CurrentInteractionType;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    UObject* CurrentInteractionTarget;

    // Survival state
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HungerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float ThirstLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FatigueLevel;

    // Combat state
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FString CurrentWeaponType;

    // Animation parameters (for Animation Blueprint communication)
    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    TMap<FString, float> AnimationParameters;

    // Debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugMode;

    // State transition
    UPROPERTY(BlueprintReadOnly, Category = "State Transition")
    bool bIsTransitioning;

    UPROPERTY(BlueprintReadOnly, Category = "State Transition")
    float TransitionProgress;

    UPROPERTY(BlueprintReadOnly, Category = "State Transition")
    float TransitionDuration;

private:
    // Internal state management
    void UpdateAnimationSystems(float DeltaTime);
    void UpdateEmotionalInfluence(float DeltaTime);
    void UpdateEnvironmentalInfluence(float DeltaTime);
    void UpdateSurvivalInfluence(float DeltaTime);
    
    // State transitions
    void ProcessStateTransition(float DeltaTime);
    bool CanTransitionToState(ECharacterAnimationState NewState) const;
    
    // System coordination
    void SynchronizeMotionMatching();
    void SynchronizeAdaptiveIK();
    void UpdateAnimationBlueprint();
    
    // Utility functions
    void CacheComponentReferences();
    float CalculateOverallAnimationWeight() const;
    void ApplyEmotionalModifiers();
};