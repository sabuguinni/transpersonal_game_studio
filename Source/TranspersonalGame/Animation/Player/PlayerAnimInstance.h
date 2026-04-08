#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearch.h"
#include "Engine/Engine.h"
#include "../Core/AnimationSystemManager.h"
#include "PlayerAnimInstance.generated.h"

class UCharacterMovementComponent;
class UPoseSearchDatabase;
class UPoseSearchSchema;

USTRUCT(BlueprintType)
struct FFootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly)
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsGrounded = false;
};

/**
 * Animation Instance para o jogador - implementa Motion Matching e IK de pés
 * Baseado no sistema do Game Animation Sample mas adaptado para o contexto pré-histórico
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

    // Character References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UCharacterMovementComponent* CharacterMovement;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UAnimationSystemManager* AnimationManager;

    // Motion Matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UPoseSearchDatabase* LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UPoseSearchDatabase* CrouchingDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UPoseSearchDatabase* InjuredDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UPoseSearchSchema* DefaultSchema;

    // Movement Data
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsFalling = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsJumping = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    // Emotional State Data
    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float ConfidenceLevel = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float InjuryLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float ExhaustionLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    bool bIsAfraid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    bool bIsInjured = false;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    bool bIsExhausted = false;

    // Foot IK System
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FFootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FFootIKData RightFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName LeftFootBoneName = "foot_l";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName RightFootBoneName = "foot_r";

    // Animation Blending
    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float MotionMatchingBlendTime = 0.2f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float EmotionalBlendWeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float InjuryBlendWeight = 0.0f;

private:
    // Internal update functions
    void UpdateMovementData();
    void UpdateEmotionalData();
    void UpdateFootIK(float DeltaTime);
    void UpdateMotionMatchingDatabase();

    // Foot IK helpers
    FFootIKData CalculateFootIK(const FName& FootBoneName, float DeltaTime);
    bool PerformFootTrace(const FVector& FootLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    // Previous frame data for interpolation
    FVector PreviousVelocity = FVector::ZeroVector;
    float PreviousFearLevel = 0.0f;
    float PreviousInjuryLevel = 0.0f;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerFearAnimation(float Intensity, float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerInjuryAnimation(float Severity);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateExhaustionLevel(float NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    UPoseSearchDatabase* GetCurrentMotionMatchingDatabase() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldUseCrouchingDatabase() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldUseInjuredDatabase() const;
};