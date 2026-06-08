#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Anim_MotionMatchingSystem.h"
#include "Anim_CharacterAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float KneeTargetZ;

    FAnim_IKFootData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        KneeTargetZ = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
    // Motion Matching Variables
    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    FAnim_MotionData MotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    FName CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    class UAnim_MotionMatchingSystem* MotionMatchingSystem;

    // IK Variables
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_IKFootData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_IKFootData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float HipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    // Blend Space Variables
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsMoving;

    // Animation State Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayActionMontage(FName ActionName, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopActionMontage(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_IKFootData CalculateFootIK(FName SocketName, float DeltaSeconds);

protected:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class UAnimMontage* CurrentActionMontage;

private:
    void UpdateLocomotionVariables();
    void UpdateMotionMatchingData();
    FVector PerformFootTrace(const FVector& FootLocation, float& OutImpactZ) const;
};