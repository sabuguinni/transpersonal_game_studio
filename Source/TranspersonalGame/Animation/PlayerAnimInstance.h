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
    EAnim_LocomotionState LocomotionState;

    // --- State Flags ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    bool bJustLanded;

    // --- Weapon ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Weapon")
    EAnim_WeaponState WeaponState;

    // --- Survival Stats (drive animation intensity) ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float HealthNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float FearLevel;

    // --- IK ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float IKAlpha;

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    void UpdateLocomotionState();
    void UpdateIK(float DeltaSeconds);
    void UpdateSurvivalDrivenAnim();
    float SmoothFloat(float Current, float Target, float SmoothSpeed, float DeltaSeconds);
};
