#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "AnimationSystemCore.generated.h"

/**
 * Core Animation System for Transpersonal Game
 * Handles Motion Matching, IK, and character expression systems
 */

UENUM(BlueprintType)
enum class ECharacterEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Terrified   UMETA(DisplayName = "Terrified"),
    Curious     UMETA(DisplayName = "Curious"),
    Focused     UMETA(DisplayName = "Focused")
};

UENUM(BlueprintType)
enum class EMovementIntention : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Sneaking    UMETA(DisplayName = "Sneaking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting")
};

USTRUCT(BlueprintType)
struct FCharacterAnimationState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterEmotionalState EmotionalState = ECharacterEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMovementIntention MovementIntention = EMovementIntention::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroundSlope = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MovementDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCarryingObject = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ObjectWeight = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAnimationSystemCore();

    // Motion Matching Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* InteractionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* EmotionalDatabase;

    // IK Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKRange = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKInterpSpeed = 15.0f;

    // Animation Blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float EmotionalBlendSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float MovementBlendSpeed = 5.0f;

    // Character Expression
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCharacterState(const FCharacterAnimationState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FCharacterAnimationState GetCurrentAnimationState() const { return CurrentAnimationState; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetFearLevel(float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerEmotionalResponse(ECharacterEmotionalState NewEmotion, float Duration = 2.0f);

private:
    UPROPERTY()
    FCharacterAnimationState CurrentAnimationState;

    UPROPERTY()
    FCharacterAnimationState TargetAnimationState;

    float EmotionalTransitionTimer = 0.0f;
    float EmotionalTransitionDuration = 0.0f;
};