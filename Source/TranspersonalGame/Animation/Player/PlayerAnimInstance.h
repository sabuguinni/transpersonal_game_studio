#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
// FIXME: Missing header - #include "Animation/PoseSearchDatabase.h"
// DISABLED: #include "Animation/MotionMatchingAnimNodeLibrary.h"
// DISABLED: #include "IKRigInterface.h"
#include "Engine/Engine.h"
#include "../Core/AnimationSystemManager.h"
#include "PlayerAnimInstance.generated.h"

/**
 * Player Animation Instance
 * Implements Motion Matching with vulnerability-focused movement
 * Integrates IK for terrain adaptation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaTimeX) override;

public:
    // Motion Matching Variables
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class UPoseSearchDatabase* CurrentDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector Acceleration;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsFalling;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsJumping;

    // Emotional State Variables
    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float TensionLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    bool bIsBeingHunted;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float CautionMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float MovementHesitation;

    // IK Variables
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float RightFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float HipOffset;

    // Look At Variables (for vulnerability)
    UPROPERTY(BlueprintReadOnly, Category = "Look At")
    FVector LookAtTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Look At")
    float LookAtAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Look At")
    bool bShouldLookAround;

    UPROPERTY(BlueprintReadOnly, Category = "Look At")
    float HeadTurnSpeed;

    // Animation System Reference
    UPROPERTY(BlueprintReadOnly, Category = "System")
    UAnimationSystemManager* AnimationManager;

private:
    // Internal calculation functions
    void UpdateMovementVariables();
    void UpdateEmotionalState();
    void UpdateIKVariables();
    void UpdateLookAtBehavior();
    void CalculateFootIK(const FString& FootBoneName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha);
    FVector PerformFootTrace(const FVector& FootLocation) const;
    
    // Cached references
    class ACharacter* OwnerCharacter;
    class UCharacterMovementComponent* MovementComponent;
    class USkeletalMeshComponent* SkeletalMeshComponent;
    
    // Internal state
    FVector PreviousVelocity;
    float LookAroundTimer;
    float NextLookTime;
    FVector CurrentLookTarget;
};