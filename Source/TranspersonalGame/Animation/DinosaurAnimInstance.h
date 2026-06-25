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

    // ── Locomotion State ──
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
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsRoaring;

    // ── IK Foot Placement ──
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float IKFootTraceDistance;

    // ── Blend weights ──
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float WalkRunBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float AttackBlendWeight;

    // ── Speed thresholds ──
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float RunSpeedThreshold;

    // ── Foot IK update ──
    UFUNCTION(BlueprintCallable, Category = "Anim|IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "Anim|Locomotion")
    void SetAttacking(bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Anim|Locomotion")
    void SetDead(bool bDead);

    UFUNCTION(BlueprintCallable, Category = "Anim|Locomotion")
    void SetRoaring(bool bRoaring);

private:
    void UpdateLocomotionState();
    void UpdateWalkRunBlend();
    void TraceFootIK(FName FootSocketName, FVector& OutIKLocation);

    APawn* OwnerPawn;
};
