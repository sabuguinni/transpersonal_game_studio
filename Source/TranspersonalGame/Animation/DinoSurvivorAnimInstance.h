#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // --- Locomotion ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    float LeanAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    bool bIsSprinting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    bool bIsMoving;

    // --- Foot IK ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK")
    FVector LeftFootIKLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK")
    FVector RightFootIKLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK")
    FRotator LeftFootIKRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK")
    FRotator RightFootIKRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK")
    float LeftFootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK")
    float RightFootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK")
    FVector PelvisOffset;

    // --- Aim Offset ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|AimOffset")
    float AimPitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|AimOffset")
    float AimYaw;

    // --- Survival State ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Survival")
    float StaminaNormalized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Survival")
    float FearNormalized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Survival")
    bool bIsExhausted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Survival")
    bool bIsInjured;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Survival")
    bool bIsCarrying;

    // --- Jump ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Jump")
    bool bJustLanded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Jump")
    float FallSpeed;

    // --- Foot IK Methods ---
    UFUNCTION(BlueprintCallable, Category = "Anim|FootIK")
    void UpdateFootIK(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category = "Anim|FootIK")
    FVector TraceFootIK(FName SocketName, FVector& OutNormal);

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    float LeanAngleTarget;
    float PreviousSpeed;
};
