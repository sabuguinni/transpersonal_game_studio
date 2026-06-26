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
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // --- Locomotion State ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_DinoLocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsRoaring;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsDead;

    // --- Blend Space Params ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|BlendSpace")
    float WalkRunAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|BlendSpace")
    float TurnRate;

    // --- IK ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float BodyIKOffset;

    // --- Speed thresholds ---
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float RunSpeedThreshold;

protected:
    void UpdateLocomotionState();
    void UpdateFootIK();
    void SolveFootIK(FName FootSocketName, FVector& OutIKLocation);

private:
    UPROPERTY()
    class APawn* OwnerPawn;
};
