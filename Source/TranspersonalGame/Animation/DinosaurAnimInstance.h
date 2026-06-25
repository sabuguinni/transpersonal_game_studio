#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinosaurAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinoLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Attack      UMETA(DisplayName = "Attack"),
    Roar        UMETA(DisplayName = "Roar"),
    Death       UMETA(DisplayName = "Death"),
    Eat         UMETA(DisplayName = "Eat"),
    Sleep       UMETA(DisplayName = "Sleep"),
};

USTRUCT(BlueprintType)
struct FAnim_DinoLocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Animation")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Animation")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Animation")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Animation")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Animation")
    bool bIsRoaring = false;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Animation")
    bool bIsDead = false;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur Animation")
    EAnim_DinoLocomotionState CurrentState = EAnim_DinoLocomotionState::Idle;
};

/**
 * UDinosaurAnimInstance
 * Animation instance for all dinosaur species.
 * Drives locomotion blend spaces, attack montages, and IK foot placement.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // --- Locomotion Data ---
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Animation")
    FAnim_DinoLocomotionData LocomotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Animation")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Animation")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Animation")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Animation")
    EAnim_DinoLocomotionState LocomotionState = EAnim_DinoLocomotionState::Idle;

    // --- IK Foot Placement ---
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float IKFootBlendWeight = 1.0f;

    // --- Blend Weights ---
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Animation")
    float WalkRunBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Animation")
    float AttackBlendWeight = 0.0f;

    // --- Functions ---
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetLocomotionState(EAnim_DinoLocomotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerAttack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerRoar();

    UFUNCTION(BlueprintPure, Category = "Dinosaur Animation")
    bool IsInAttackState() const;

private:
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateIKFootPlacement();
    void UpdateBlendWeights(float DeltaSeconds);

    float AttackCooldown = 0.0f;
    float RoarCooldown = 0.0f;
};
