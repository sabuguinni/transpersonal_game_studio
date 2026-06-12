#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_PrimitiveCharacterController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Combat      UMETA(DisplayName = "Combat"),
    Injured     UMETA(DisplayName = "Injured")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Building        UMETA(DisplayName = "Building"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Hiding          UMETA(DisplayName = "Hiding"),
    Climbing        UMETA(DisplayName = "Climbing")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_CharacterAnimData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Action")
    EAnim_ActionState ActionState;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel;

    FAnim_CharacterAnimData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
        ActionState = EAnim_ActionState::None;
        HealthPercent = 1.0f;
        StaminaPercent = 1.0f;
        FearLevel = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_PrimitiveCharacterController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrimitiveCharacterController();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    class UCharacterMovementComponent* CharacterMovement;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_CharacterAnimData AnimData;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations|Locomotion")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations|Actions")
    class UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations|Actions")
    class UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations|Actions")
    class UAnimMontage* HuntingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations|Combat")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations|Combat")
    class UAnimMontage* DefendMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations|Survival")
    class UAnimMontage* InjuredMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations|Survival")
    class UAnimMontage* FearMontage;

public:
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayActionMontage(EAnim_ActionState ActionType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopActionMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetActionState(EAnim_ActionState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsPlayingActionMontage() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return AnimData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_ActionState GetCurrentActionState() const { return AnimData.ActionState; }

private:
    void UpdateMovementData();
    void UpdateActionData();
    void UpdateSurvivalData();
    EAnim_MovementState CalculateMovementState();
};