#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimationController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Combat      UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Defending       UMETA(DisplayName = "Defending"),
    Interacting     UMETA(DisplayName = "Interacting")
};

USTRUCT(BlueprintType)
struct FAnim_AnimationSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* FallMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* CrouchMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* LocomotionBlendSpace;

    FAnim_AnimationSet()
    {
        IdleMontage = nullptr;
        WalkMontage = nullptr;
        RunMontage = nullptr;
        JumpMontage = nullptr;
        FallMontage = nullptr;
        CrouchMontage = nullptr;
        LocomotionBlendSpace = nullptr;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetActionState(EAnim_ActionState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_ActionState GetActionState() const { return CurrentActionState; }

    // Animation playback
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAnimationMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAnimationMontage(UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAllMontages();

    // Movement animation
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayLandAnimation();

    // Action animations
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayGatherAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayCraftAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAttackAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayDefendAnimation();

    // Animation properties
    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMovementSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMovementDirection() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsInAir() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsCrouching() const;

protected:
    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FAnim_AnimationSet AnimationSet;

    // State tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    EAnim_ActionState CurrentActionState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    EAnim_MovementState PreviousMovementState;

    // Animation parameters
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    float MovementSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    float MovementDirection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    bool bIsInAir;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    bool bIsCrouching;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    bool bIsMoving;

    // Component references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Animation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float MovementSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float AnimationBlendTime;

private:
    void InitializeReferences();
    void UpdateAnimationParameters();
    EAnim_MovementState CalculateMovementState();
    void HandleStateTransition(EAnim_MovementState NewState);
    void PlayStateAnimation(EAnim_MovementState State);
};