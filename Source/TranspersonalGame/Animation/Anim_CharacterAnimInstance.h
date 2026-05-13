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
    Crouching   UMETA(DisplayName = "Crouching")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    None        UMETA(DisplayName = "None"),
    Ready       UMETA(DisplayName = "Ready"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Blocking    UMETA(DisplayName = "Blocking"),
    Dodging     UMETA(DisplayName = "Dodging")
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
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ACharacter* OwnerCharacter = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    UCharacterMovementComponent* MovementComponent = nullptr;

    // Movement state
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_CombatState CurrentCombatState = EAnim_CombatState::None;

    // Movement data
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FAnim_MovementData MovementData;

    // Animation parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float WalkThreshold = 50.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float RunThreshold = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float JumpThreshold = 200.0f;

    // IK settings
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    bool bEnableFootIK = true;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootOffset = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootOffset = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    // Animation functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatState(EAnim_CombatState NewState);

private:
    // Internal helpers
    void PerformFootTrace(const FName& SocketName, FVector& OutOffset, FRotator& OutRotation);
    EAnim_MovementState DetermineMovementState() const;
};