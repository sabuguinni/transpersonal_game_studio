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

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_DinoLocomotionState LocomotionState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Config")
    float RunSpeedThreshold;

    UFUNCTION(BlueprintCallable, Category = "Anim|Control")
    void SetAttacking(bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Anim|Control")
    void SetDead(bool bDead);

private:
    UPROPERTY()
    class APawn* OwnerPawn;

    void UpdateLocomotionState();
};
