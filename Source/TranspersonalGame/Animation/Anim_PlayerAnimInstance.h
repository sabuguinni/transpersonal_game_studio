#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_PlayerAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_PlayerMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_PlayerCombatState : uint8
{
    None            UMETA(DisplayName = "None"),
    Armed           UMETA(DisplayName = "Armed"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Dodging         UMETA(DisplayName = "Dodging"),
    Stunned         UMETA(DisplayName = "Stunned")
};

UENUM(BlueprintType)
enum class EAnim_PlayerEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Afraid      UMETA(DisplayName = "Afraid"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured")
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_PlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PlayerAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement State Variables
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    EAnim_PlayerMovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Pitch;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Yaw;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Roll;

    // Combat State Variables
    UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    EAnim_PlayerCombatState CombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsArmed;

    UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsBlocking;

    // Survival State Variables
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    EAnim_PlayerEmotionalState EmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float HungerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float ThirstLevel;

    // IK Foot Placement Variables
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    float LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    float RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FRotator RightFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    float PelvisIKOffset;

    // Animation Blending Variables
    UPROPERTY(BlueprintReadOnly, Category = "Blending", meta = (AllowPrivateAccess = "true"))
    float IdleToWalkBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Blending", meta = (AllowPrivateAccess = "true"))
    float WalkToRunBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Blending", meta = (AllowPrivateAccess = "true"))
    float CombatBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Blending", meta = (AllowPrivateAccess = "true"))
    float EmotionalBlend;

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Helper functions
    void UpdateMovementState();
    void UpdateCombatState();
    void UpdateSurvivalState();
    void UpdateIKFootPlacement();
    void UpdateAnimationBlending();

    // IK Helper functions
    float PerformFootTrace(FName SocketName, float& FootIKOffset, FRotator& FootIKRotation);
    void UpdatePelvisIK();

    // State transition helpers
    bool ShouldEnterCombatState() const;
    bool ShouldExitCombatState() const;
    EAnim_PlayerEmotionalState CalculateEmotionalState() const;

    // Animation speed modifiers
    float CalculateAnimationSpeedMultiplier() const;
    float GetStaminaSpeedModifier() const;
    float GetHealthSpeedModifier() const;
    float GetFearSpeedModifier() const;

    // Constants
    static constexpr float WALK_SPEED_THRESHOLD = 150.0f;
    static constexpr float RUN_SPEED_THRESHOLD = 400.0f;
    static constexpr float IK_TRACE_DISTANCE = 50.0f;
    static constexpr float IK_INTERPOLATION_SPEED = 15.0f;
    static constexpr float COMBAT_TRANSITION_SPEED = 8.0f;
    static constexpr float EMOTIONAL_TRANSITION_SPEED = 3.0f;
};