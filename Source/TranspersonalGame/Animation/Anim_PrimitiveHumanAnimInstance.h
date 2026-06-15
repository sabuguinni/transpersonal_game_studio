#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Anim_PrimitiveHumanAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    SpearReady  UMETA(DisplayName = "Spear Ready"),
    AxeReady    UMETA(DisplayName = "Axe Ready"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Blocking    UMETA(DisplayName = "Blocking"),
    Dodging     UMETA(DisplayName = "Dodging")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalAction : uint8
{
    None        UMETA(DisplayName = "None"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Building    UMETA(DisplayName = "Building"),
    Cooking     UMETA(DisplayName = "Cooking"),
    Healing     UMETA(DisplayName = "Healing")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundDistance = 0.0f;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        Velocity = FVector::ZeroVector;
        GroundDistance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootOffset = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootOffset = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float PelvisOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float IKAlpha = 1.0f;

    FAnim_IKFootData()
    {
        LeftFootOffset = FVector::ZeroVector;
        RightFootOffset = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        PelvisOffset = 0.0f;
        IKAlpha = 1.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_PrimitiveHumanAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrimitiveHumanAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    // Movement State Machine
    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_MovementState CurrentMovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_CombatState CurrentCombatState = EAnim_CombatState::Unarmed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_SurvivalAction CurrentSurvivalAction = EAnim_SurvivalAction::None;

    // Movement Data
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FAnim_MovementData MovementData;

    // IK Foot Placement
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_IKFootData IKFootData;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwningCharacter = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* CharacterMovement = nullptr;

    // Animation Blending
    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float IdleToWalkBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float WalkToRunBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float CombatBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float SurvivalActionBlend = 0.0f;

    // Survival Stats Integration
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FatigueLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInjured = false;

public:
    // Animation State Control
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetCombatState(EAnim_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetSurvivalAction(EAnim_SurvivalAction NewAction);

    // IK System
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector TraceForGround(FVector FootLocation, float TraceDistance = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void CalculateFootOffset(FVector FootLocation, FVector& OutOffset, FRotator& OutRotation, bool bIsLeftFoot);

    // Movement Analysis
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateMovementData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    EAnim_MovementState DetermineMovementState();

    // Survival Integration
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalStats();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetAnimationSpeedModifier() const;

    // Combat Integration
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TriggerAttackAnimation();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TriggerBlockAnimation();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TriggerDodgeAnimation();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsMoving() const { return MovementData.Speed > 1.0f; }

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsRunning() const { return MovementData.Speed > 300.0f; }

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsInCombat() const { return CurrentCombatState != EAnim_CombatState::Unarmed; }

private:
    // Internal update functions
    void UpdateBlendValues(float DeltaTime);
    void UpdateStateTransitions();
    void SmoothBlendValue(float& CurrentValue, float TargetValue, float BlendSpeed, float DeltaTime);

    // IK trace parameters
    UPROPERTY(EditAnywhere, Category = "IK Settings")
    float IKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, Category = "IK Settings")
    float IKInterpSpeed = 15.0f;

    // Blend speeds
    UPROPERTY(EditAnywhere, Category = "Blend Settings")
    float MovementBlendSpeed = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Blend Settings")
    float CombatBlendSpeed = 8.0f;

    UPROPERTY(EditAnywhere, Category = "Blend Settings")
    float SurvivalBlendSpeed = 3.0f;
};