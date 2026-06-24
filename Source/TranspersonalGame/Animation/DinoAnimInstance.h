#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinoLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Attack      UMETA(DisplayName = "Attack"),
    Death       UMETA(DisplayName = "Death"),
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    EAnim_DinoLocomotionState LocomotionState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Config")
    float RunSpeedThreshold;

private:
    UPROPERTY()
    class APawn* OwnerPawn;
};
