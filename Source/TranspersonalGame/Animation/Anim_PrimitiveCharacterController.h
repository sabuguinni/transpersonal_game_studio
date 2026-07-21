#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_PrimitiveCharacterController.generated.h"

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
    Hunting         UMETA(DisplayName = "Hunting"),
    Building        UMETA(DisplayName = "Building"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_CharacterAnimationData
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

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAnim_MovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAnim_ActionState ActionState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercentage;

    FAnim_CharacterAnimationData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
        ActionState = EAnim_ActionState::None;
        bIsInCombat = false;
        HealthPercentage = 100.0f;
        StaminaPercentage = 100.0f;
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
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Movement Component")
    class UCharacterMovementComponent* CharacterMovement;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_CharacterAnimationData AnimationData;

    // Animation Montages for survival actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* HuntingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* BuildingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* EatingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* DrinkingMontage;

    // Movement thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
    float RunSpeedThreshold;

private:
    void UpdateMovementData();
    void UpdateActionState();
    EAnim_MovementState CalculateMovementState();

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlaySurvivalAction(EAnim_ActionState ActionType);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void StopCurrentAction();

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    FAnim_CharacterAnimationData GetAnimationData() const { return AnimationData; }
};