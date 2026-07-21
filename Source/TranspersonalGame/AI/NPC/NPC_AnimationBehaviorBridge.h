#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "SharedTypes.h"
#include "NPC_AnimationBehaviorBridge.generated.h"

// Forward declarations
class UNPC_BaseBehaviorComponent;
class UAnim_MotionMatchingSystem;
class UAnim_IKFootPlacement;

UENUM(BlueprintType)
enum class ENPC_AnimationBehaviorState : uint8
{
    Idle = 0,
    Walking,
    Running,
    Hunting,
    Feeding,
    Sleeping,
    Alert,
    Aggressive,
    Fleeing,
    Socializing,
    Territorial,
    Mating
};

USTRUCT(BlueprintType)
struct FNPC_AnimationBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation Behavior")
    ENPC_AnimationBehaviorState CurrentState = ENPC_AnimationBehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Animation Behavior")
    ENPC_AnimationBehaviorState PreviousState = ENPC_AnimationBehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Animation Behavior")
    float StateTransitionBlendTime = 0.3f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation Behavior")
    float MovementSpeed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation Behavior")
    FVector MovementDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation Behavior")
    float EmotionalIntensity = 0.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation Behavior")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation Behavior")
    bool bIsInPack = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation Behavior")
    float TerrainAdaptation = 0.0f;
};

/**
 * Bridge component that connects NPC behavior systems with the animation system
 * Translates behavior states into animation parameters and coordinates motion matching
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_AnimationBehaviorBridge : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_AnimationBehaviorBridge();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core animation-behavior integration
    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void UpdateAnimationFromBehavior(ENPC_AnimationBehaviorState NewState, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void SetMotionMatchingParameters(float Speed, FVector Direction, bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void UpdateIKFootPlacement(bool bEnable, float TerrainAdaptation = 1.0f);

    // Behavior state management
    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    ENPC_AnimationBehaviorState GetCurrentAnimationState() const { return AnimationData.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void TransitionToState(ENPC_AnimationBehaviorState NewState, float BlendTime = 0.3f);

    // Pack coordination animation
    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void UpdatePackAnimationCoordination(bool bIsPackLeader, int32 PackSize, float PackCohesion);

    // Emotional state animation
    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void UpdateEmotionalAnimation(float Fear, float Aggression, float Curiosity);

    // Territorial animation
    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void UpdateTerritorialAnimation(bool bInTerritory, float TerritorialIntensity);

    // Communication animation
    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void TriggerCommunicationAnimation(ECommunicationType CommType, float Intensity);

    // Getters for animation blueprint
    UFUNCTION(BlueprintPure, Category = "Animation Behavior")
    FNPC_AnimationBehaviorData GetAnimationBehaviorData() const { return AnimationData; }

    UFUNCTION(BlueprintPure, Category = "Animation Behavior")
    float GetMovementSpeedNormalized() const;

    UFUNCTION(BlueprintPure, Category = "Animation Behavior")
    bool ShouldUseMotionMatching() const { return bUseMotionMatching; }

protected:
    // Animation data
    UPROPERTY(BlueprintReadOnly, Category = "Animation Behavior")
    FNPC_AnimationBehaviorData AnimationData;

    // Component references
    UPROPERTY()
    UNPC_BaseBehaviorComponent* BehaviorComponent;

    UPROPERTY()
    UAnim_MotionMatchingSystem* MotionMatchingSystem;

    UPROPERTY()
    UAnim_IKFootPlacement* IKFootSystem;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bUseMotionMatching = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bUseIKFootPlacement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float DefaultTransitionTime = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float MaxMovementSpeed = 1000.0f;

    // State tracking
    UPROPERTY()
    float StateChangeTimer = 0.0f;

    UPROPERTY()
    bool bInStateTransition = false;

private:
    // Internal methods
    void InitializeComponentReferences();
    void UpdateAnimationParameters(float DeltaTime);
    void HandleStateTransition(float DeltaTime);
    void SynchronizeWithBehaviorComponent();
    void UpdateMotionMatchingData();
    void UpdateIKParameters();

    // Animation state mapping
    ENPC_AnimationBehaviorState MapBehaviorToAnimationState(EDinosaurBehaviorState BehaviorState);
    float CalculateEmotionalIntensity();
    float CalculateMovementIntensity();
};