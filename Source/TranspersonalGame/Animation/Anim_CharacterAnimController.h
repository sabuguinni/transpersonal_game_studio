#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "Anim_CharacterAnimController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None        UMETA(DisplayName = "None"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Building    UMETA(DisplayName = "Building"),
    Eating      UMETA(DisplayName = "Eating"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Fighting    UMETA(DisplayName = "Fighting"),
    Throwing    UMETA(DisplayName = "Throwing"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    FVector LeftFootLocation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    FVector RightFootLocation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    FRotator RightFootRotation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    float LeftFootAlpha;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    float RightFootAlpha;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    float HipOffset;

    FAnim_FootIKData()
    {
        LeftFootLocation = FVector::ZeroVector;
        RightFootLocation = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        LeftFootAlpha = 0.0f;
        RightFootAlpha = 0.0f;
        HipOffset = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimController();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement State Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetActionState(EAnim_ActionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayActionMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    // Foot IK System
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FVector PerformFootTrace(FName SocketName, float TraceDistance = 50.0f);

protected:
    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    UCharacterMovementComponent* MovementComponent;

    // Movement Variables
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsAccelerating;

    // State Variables
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EAnim_ActionState CurrentActionState;

    // Animation Assets
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation Assets")
    UBlendSpace* MovementBlendSpace;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation Assets")
    UAnimMontage* LandMontage;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation Assets")
    TMap<EAnim_ActionState, UAnimMontage*> ActionMontages;

    // Foot IK
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FAnim_FootIKData FootIKData;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Foot IK")
    float FootIKTraceDistance;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Foot IK")
    float FootIKInterpSpeed;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Foot IK")
    bool bEnableFootIK;

    // Animation Smoothing
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
    float SpeedSmoothingRate;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
    float DirectionSmoothingRate;

private:
    // Internal state tracking
    float LastSpeed;
    float LastDirection;
    float StateTransitionTimer;
    bool bWasInAir;
};