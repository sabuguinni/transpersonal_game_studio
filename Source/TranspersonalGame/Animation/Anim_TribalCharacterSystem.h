#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace1D.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_TribalCharacterSystem.generated.h"

// Tribal character states for animation
UENUM(BlueprintType)
enum class EAnim_TribalState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Communicating   UMETA(DisplayName = "Communicating"),
    Combat          UMETA(DisplayName = "Combat"),
    Injured         UMETA(DisplayName = "Injured"),
    Sleeping        UMETA(DisplayName = "Sleeping")
};

// Tribal animation data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    EAnim_TribalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    float StaminaLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    bool bIsCarryingObject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    bool bIsInjured;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    float EmotionalState; // 0.0 = fearful, 0.5 = neutral, 1.0 = confident

    FAnim_TribalAnimationData()
    {
        CurrentState = EAnim_TribalState::Idle;
        MovementSpeed = 0.0f;
        StaminaLevel = 1.0f;
        bIsCarryingObject = false;
        bIsInjured = false;
        EmotionalState = 0.5f;
    }
};

// Tribal character animation controller component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_TribalCharacterController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_TribalCharacterController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Animation montages for tribal actions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Montages")
    UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Montages")
    UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Montages")
    UAnimMontage* HuntingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Montages")
    UAnimMontage* CommunicationMontage;

    // Blend spaces for movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Blend Spaces")
    UBlendSpace1D* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Blend Spaces")
    UBlendSpace1D* CombatBlendSpace;

    // Current animation data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal Animation")
    FAnim_TribalAnimationData AnimationData;

    // Animation control functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void SetTribalState(EAnim_TribalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayGatheringAnimation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayCraftingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayHuntingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayCommunicationAnimation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void UpdateMovementSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void SetCarryingObject(bool bCarrying);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void SetInjuredState(bool bInjured);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void SetEmotionalState(float EmotionLevel);

    UFUNCTION(BlueprintPure, Category = "Tribal Animation")
    EAnim_TribalState GetCurrentTribalState() const;

    UFUNCTION(BlueprintPure, Category = "Tribal Animation")
    bool IsPlayingActionMontage() const;

protected:
    // Internal state management
    void UpdateAnimationState(float DeltaTime);
    void HandleStateTransition(EAnim_TribalState NewState);
    void ApplyMovementBlending();

    // Character reference
    UPROPERTY()
    ACharacter* OwnerCharacter;

    // Animation instance reference
    UPROPERTY()
    UAnimInstance* AnimInstance;

    // State transition timing
    float StateTransitionTimer;
    float StateTransitionDuration;

    // Movement blending parameters
    float CurrentBlendValue;
    float TargetBlendValue;
    float BlendTransitionSpeed;

    // Action montage tracking
    bool bIsPlayingActionMontage;
    UAnimMontage* CurrentActionMontage;
};

// Tribal character animation instance
UCLASS()
class TRANSPERSONALGAME_API UAnim_TribalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_TribalAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

public:
    // Animation properties exposed to Blueprint
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal Animation")
    FAnim_TribalAnimationData TribalData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating;

    // Tribal-specific animation states
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal States")
    bool bIsGathering;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal States")
    bool bIsCrafting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal States")
    bool bIsHunting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal States")
    bool bIsCommunicating;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal States")
    bool bIsCarryingLoad;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal States")
    float EmotionalIntensity;

protected:
    // Character reference
    UPROPERTY()
    ACharacter* Character;

    // Tribal controller reference
    UPROPERTY()
    UAnim_TribalCharacterController* TribalController;

    // Update functions
    void UpdateMovementValues();
    void UpdateTribalStates();
    void UpdateEmotionalBlending();

    // Cached movement values
    FVector Velocity;
    FRotator CharacterRotation;
    FRotator LastFrameRotation;
};