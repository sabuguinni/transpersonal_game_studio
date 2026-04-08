#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "IKRigInterface.h"
#include "../Core/AnimationSystemCore.h"
#include "PlayerAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;

/**
 * Player Animation Instance
 * Main animation controller for the paleontologist character
 * 
 * Features:
 * - Motion Matching for responsive movement
 * - IK for foot placement and terrain adaptation
 * - Fear-based animation modulation
 * - Contextual animation selection based on environment
 */

USTRUCT(BlueprintType)
struct FPlayerAnimationData
{
    GENERATED_BODY()

    // Movement Data
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    // Character State
    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    float CautionLevel = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsHiding = false;

    // Environment Data
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    ETerrainType CurrentTerrain = ETerrainType::Flat;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float GroundSlope = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bNearDinosaur = false;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float DinosaurThreatLevel = 0.0f;

    FPlayerAnimationData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        bIsMoving = false;
        bIsInAir = false;
        FearLevel = 0.0f;
        CautionLevel = 0.5f;
        bIsCrouching = false;
        bIsHiding = false;
        CurrentTerrain = ETerrainType::Flat;
        GroundSlope = 0.0f;
        bNearDinosaur = false;
        DinosaurThreatLevel = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;
    virtual void NativeBeginPlay() override;

public:
    // Animation Data
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FPlayerAnimationData AnimationData;

    // Motion Matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> CurrentDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MotionMatchingBlendTime = 0.3f;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float MaxFootIKOffset = 20.0f;

    // Fear Response Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear Response")
    float FearResponseInterpSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear Response")
    float MaxFearTrembleAmount = 2.0f;

    // Animation System Reference
    UPROPERTY(BlueprintReadOnly, Category = "System")
    TObjectPtr<UAnimationSystemCore> AnimationSystem;

protected:
    // Character References
    UPROPERTY()
    TObjectPtr<ACharacter> OwningCharacter;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    // Internal Animation State
    UPROPERTY()
    ECharacterAnimationState CurrentAnimationState = ECharacterAnimationState::Idle;

    UPROPERTY()
    float StateTimer = 0.0f;

    // IK Data
    UPROPERTY()
    FVector LeftFootIKOffset = FVector::ZeroVector;

    UPROPERTY()
    FVector RightFootIKOffset = FVector::ZeroVector;

    UPROPERTY()
    FRotator LeftFootIKRotation = FRotator::ZeroRotator;

    UPROPERTY()
    FRotator RightFootIKRotation = FRotator::ZeroRotator;

    // Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCharacterState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateEnvironmentData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateIKData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMotionMatchingDatabase();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    ECharacterAnimationState DetermineAnimationState() const;

    // IK Helper Functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector CalculateFootIKOffset(const FName& SocketName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FRotator CalculateFootIKRotation(const FName& SocketName, const FVector& IKOffset);

    // Fear Response Functions
    UFUNCTION(BlueprintCallable, Category = "Fear")
    float CalculateFearTrembleAmount() const;

    UFUNCTION(BlueprintCallable, Category = "Fear")
    void ApplyFearResponse(float DeltaTime);

private:
    // Internal timers and states
    float LastFearUpdateTime = 0.0f;
    float FearDecayRate = 0.5f; // Fear decreases over time when no threat
    
    // IK trace settings
    static constexpr float IK_TRACE_DISTANCE = 50.0f;
    static constexpr float IK_FOOT_HEIGHT = 5.0f;
};