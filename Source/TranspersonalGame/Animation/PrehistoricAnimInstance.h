#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionMatchingComponent.h"
#include "../SharedTypes.h"
#include "PrehistoricAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PrehistoricMovementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float GroundSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    FAnim_PrehistoricMovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        GroundSpeed = 0.0f;
        Velocity = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PrehistoricActionState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actions")
    bool bIsGathering = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actions")
    bool bIsCrafting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actions")
    bool bIsHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actions")
    bool bIsClimbing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actions")
    bool bIsCarryingObject = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actions")
    float ActionIntensity = 0.0f;

    FAnim_PrehistoricActionState()
    {
        bIsGathering = false;
        bIsCrafting = false;
        bIsHunting = false;
        bIsClimbing = false;
        bIsCarryingObject = false;
        ActionIntensity = 0.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UPrehistoricAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPrehistoricAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

public:
    // Character References
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* CharacterMovement;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UMotionMatchingComponent* MotionMatchingComponent;

    // Animation Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    FAnim_PrehistoricMovementData MovementData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    FAnim_PrehistoricActionState ActionState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    FAnim_TerrainAdaptation TerrainData;

    // IK Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float LeftFootIKOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float RightFootIKOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator LeftFootIKRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator RightFootIKRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float HipOffset = 0.0f;

    // Blend Space Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    float LocomotionSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    float LocomotionDirection = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    float AimPitch = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    float AimYaw = 0.0f;

    // State Machine Triggers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine")
    bool bShouldEnterAction = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine")
    bool bShouldExitAction = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine")
    int32 ActionType = 0;

    // Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateActionState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateIKData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateBlendSpaceParameters(float DeltaTime);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsMovingOnGround() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldPlayFootstepSound() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementSpeedRatio() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FVector GetCharacterVelocity() const;

    // Action Triggers
    UFUNCTION(BlueprintCallable, Category = "Actions")
    void TriggerGatheringAction();

    UFUNCTION(BlueprintCallable, Category = "Actions")
    void TriggerCraftingAction();

    UFUNCTION(BlueprintCallable, Category = "Actions")
    void TriggerHuntingAction();

    UFUNCTION(BlueprintCallable, Category = "Actions")
    void StopCurrentAction();

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MovementThreshold = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxWalkSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxRunSpeed = 400.0f;

private:
    // Internal state
    float LastMovementUpdateTime;
    FVector LastVelocity;
    bool bWasMovingLastFrame;

    void CacheCharacterReferences();
    void ValidateReferences() const;
};