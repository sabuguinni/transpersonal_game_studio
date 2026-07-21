#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SharedTypes.h"
#include "Anim_PlayerAnimationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnim_OnAnimationStateChanged, EAnim_MovementState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnim_OnMontageFinished, class UAnimMontage*, Montage);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_AnimationSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* WalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* RunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* JumpStartAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* JumpLoopAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* JumpEndAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* CrouchIdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* CrouchWalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* LocomotionBlendSpace;

    FAnim_AnimationSet()
    {
        IdleAnimation = nullptr;
        WalkAnimation = nullptr;
        RunAnimation = nullptr;
        JumpStartAnimation = nullptr;
        JumpLoopAnimation = nullptr;
        JumpEndAnimation = nullptr;
        CrouchIdleAnimation = nullptr;
        CrouchWalkAnimation = nullptr;
        LocomotionBlendSpace = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float LeanAngle;

    FAnim_BlendSpaceInput()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        LeanAngle = 0.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_PlayerAnimationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_PlayerAnimationManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontage(class UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopMontage(class UAnimMontage* Montage = nullptr);

    // Blend Space Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateBlendSpaceInput(float Speed, float Direction);

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_BlendSpaceInput GetBlendSpaceInput() const { return BlendSpaceInput; }

    // Animation Set Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAnimationSet(const FAnim_AnimationSet& NewAnimationSet);

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_AnimationSet GetAnimationSet() const { return AnimationSet; }

    // Character Integration
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void InitializeWithCharacter(class ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementAnimations(float DeltaTime);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Animation Events")
    FAnim_OnAnimationStateChanged OnAnimationStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Animation Events")
    FAnim_OnMontageFinished OnMontageFinished;

protected:
    // Core Properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState PreviousMovementState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FAnim_AnimationSet AnimationSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    FAnim_BlendSpaceInput BlendSpaceInput;

    // Character References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwnerCharacter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* CharacterMovement;

    // Animation Instance
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class UAnimInstance* AnimInstance;

    // Timing and Smoothing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float BlendSpaceSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float StateTransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bEnableRootMotion;

private:
    // Internal State
    float TimeSinceLastStateChange;
    bool bIsInitialized;

    // Helper Functions
    void UpdateMovementStateFromCharacter();
    void SmoothBlendSpaceInputs(float DeltaTime);
    void HandleStateTransition(EAnim_MovementState NewState);
    bool ShouldTransitionToState(EAnim_MovementState NewState) const;
};