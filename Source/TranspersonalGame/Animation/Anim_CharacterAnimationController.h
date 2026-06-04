#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "Anim_CharacterAnimationController.generated.h"

class ACharacter;
class UAnimInstance;
class UAnimMontage;
class UBlendSpace;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    EAnim_MovementMode MovementMode = EAnim_MovementMode::Walking;

    FAnim_MovementState()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementMode = EAnim_MovementMode::Walking;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_ActionState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsBlocking = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsInteracting = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsCrafting = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    EAnim_ActionType CurrentAction = EAnim_ActionType::None;

    FAnim_ActionState()
    {
        bIsAttacking = false;
        bIsBlocking = false;
        bIsInteracting = false;
        bIsCrafting = false;
        CurrentAction = EAnim_ActionType::None;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Movement Animation Control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementAnimation(float Speed, float Direction, bool bInAir);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementMode(EAnim_MovementMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerLandingAnimation();

    // Action Animation Control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAttackAnimation(EAnim_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayInteractionAnimation(EAnim_InteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayEmoteAnimation(EAnim_EmoteType EmoteType);

    // Animation State Queries
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementState GetMovementState() const { return MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_ActionState GetActionState() const { return ActionState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsPlayingMontage() const;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> InteractionMontage;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<UAnimInstance> AnimInstance;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
    FAnim_ActionState ActionState;

    // Internal Methods
    void InitializeAnimationAssets();
    void UpdateAnimationState(float DeltaTime);
    void HandleMovementTransitions();
    void HandleActionTransitions();
};