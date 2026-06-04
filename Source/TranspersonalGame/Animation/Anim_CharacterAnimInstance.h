#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming")
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    // Movement state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSwimming;

    // Animation blending
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float IdleToWalkBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float WalkToRunBlend;

    // IK system variables
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    UCharacterMovementComponent* MovementComponent;

private:
    void UpdateMovementState();
    void UpdateIKValues();
    void PerformFootIKTrace(FVector FootLocation, float& IKOffset, FRotator& IKRotation, bool bIsLeftFoot);
    
    // IK trace parameters
    UPROPERTY(EditAnywhere, Category = "IK Settings")
    float IKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, Category = "IK Settings")
    float IKInterpSpeed = 15.0f;
};