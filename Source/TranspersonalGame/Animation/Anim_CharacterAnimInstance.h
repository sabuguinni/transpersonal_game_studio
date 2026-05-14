#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
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

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    MeleeWeapon UMETA(DisplayName = "Melee Weapon"),
    RangedWeapon UMETA(DisplayName = "Ranged Weapon"),
    Blocking    UMETA(DisplayName = "Blocking"),
    Attacking   UMETA(DisplayName = "Attacking")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundSpeed;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        bIsAccelerating = false;
        Velocity = FVector::ZeroVector;
        GroundSpeed = 0.0f;
    }
};

/**
 * Main Animation Instance for TranspersonalCharacter
 * Handles state machine logic, movement blending, and combat animations
 * Designed for realistic prehistoric human movement with weight and intention
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement State Management
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_CombatState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData MovementData;

    // Character References
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* CharacterMovement;

    // Animation Parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float WalkRunBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float TurnInPlaceAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    bool bShouldTurnInPlace;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float LeanAmount;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float AimYaw;

    // Animation Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float TurnInPlaceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float IdleTimeBeforeTurnInPlace;

    // Survival Animation States
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsHungry;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsThirsty;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsTired;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsAfraid;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel;

private:
    // Internal state tracking
    float IdleTime;
    float LastMovementTime;
    FRotator LastFrameRotation;
    
    // Update functions
    void UpdateMovementData();
    void UpdateMovementState();
    void UpdateCombatState();
    void UpdateAnimationParameters();
    void UpdateSurvivalStates();
    void UpdateTurnInPlace(float DeltaTime);
    
    // Helper functions
    EAnim_MovementState CalculateMovementState() const;
    float CalculateDirection() const;
    float CalculateWalkRunBlend() const;
    bool ShouldTurnInPlace() const;
};