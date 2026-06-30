#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Sneak       UMETA(DisplayName = "Sneak"),
    InAir       UMETA(DisplayName = "InAir"),
    Land        UMETA(DisplayName = "Land"),
    Climb       UMETA(DisplayName = "Climb")
};

UENUM(BlueprintType)
enum class EAnim_WeaponState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    SpearReady  UMETA(DisplayName = "SpearReady"),
    SpearThrow  UMETA(DisplayName = "SpearThrow"),
    BowReady    UMETA(DisplayName = "BowReady"),
    BowDraw     UMETA(DisplayName = "BowDraw")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // --- Locomotion ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float SmoothedSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float LeanAmount;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float VerticalVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsFalling;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_LocomotionState LocomotionState;

    // --- Weapon ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Weapon")
    EAnim_WeaponState WeaponState;

    // --- IK ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float LeftFootAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float RightFootAlpha;

    // --- Survival State ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsInjured;

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSurvivalState();
    float SmoothFloat(float Current, float Target, float Speed, float DeltaSeconds) const;
};
