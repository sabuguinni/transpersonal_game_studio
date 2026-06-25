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
    Death       UMETA(DisplayName = "Death"),
    Roar        UMETA(DisplayName = "Roar"),
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_DinoLocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKLocation;

    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float RunSpeedThreshold;

private:
    UPROPERTY()
    class APawn* OwnerPawn;

    void UpdateLocomotionState();
    void UpdateFootIK();
    FVector ComputeFootIKLocation(FName SocketName) const;
};
