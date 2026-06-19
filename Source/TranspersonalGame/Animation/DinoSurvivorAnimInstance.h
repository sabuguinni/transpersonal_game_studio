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

    // Movement state
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float LeanAngle;

    // Survival state
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted;

    // IK
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float IKAlpha;

    // Aiming
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aiming")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aiming")
    float AimYaw;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aiming")
    bool bIsAiming;

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    void UpdateMovementState(float DeltaSeconds);
    void UpdateSurvivalState();
    void UpdateFootIK();
    void UpdateAimState();

    float SmoothSpeed;
    float SmoothLean;
};
