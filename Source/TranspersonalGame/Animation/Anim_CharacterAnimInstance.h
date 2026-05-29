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
    Combat      UMETA(DisplayName = "Combat"),
    Gathering   UMETA(DisplayName = "Gathering")
};

UENUM(BlueprintType)
enum class EAnim_EmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement State
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsAccelerating;

    // Emotional State
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Emotion", meta = (AllowPrivateAccess = "true"))
    EAnim_EmotionalState CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Emotion", meta = (AllowPrivateAccess = "true"))
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Emotion", meta = (AllowPrivateAccess = "true"))
    float StaminaLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Emotion", meta = (AllowPrivateAccess = "true"))
    float HealthLevel;

    // IK and Procedural Animation
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
    float LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
    float RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
    FRotator RightFootIKRotation;

    // Animation Blending
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blending", meta = (AllowPrivateAccess = "true"))
    float IdleToWalkBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blending", meta = (AllowPrivateAccess = "true"))
    float WalkToRunBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blending", meta = (AllowPrivateAccess = "true"))
    float EmotionalBlend;

private:
    // Cached references
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    // Helper functions
    void UpdateMovementState();
    void UpdateEmotionalState();
    void UpdateIKValues();
    void UpdateBlendValues();
    
    EAnim_MovementState CalculateMovementState() const;
    EAnim_EmotionalState CalculateEmotionalState() const;
    
    float CalculateFootIKOffset(const FName& SocketName) const;
    FRotator CalculateFootIKRotation(const FName& SocketName) const;
};