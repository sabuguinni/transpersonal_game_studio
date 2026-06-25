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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    float GroundSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    bool bIsAttacking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    bool bIsDead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    bool bIsRoaring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    EAnim_DinoLocomotionState LocomotionState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|IK")
    float FootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|IK")
    FVector LeftFootIKOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|IK")
    FVector RightFootIKOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Speed")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Speed")
    float RunSpeedThreshold;

private:
    void UpdateLocomotionState();
    void UpdateFootIK();

    UPROPERTY()
    class APawn* OwnerPawn;
};
