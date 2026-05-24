#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/Blueprint.h"
#include "Animation/AnimInstance.h"
#include "SurvivalAnimationBlueprint.generated.h"

/**
 * Survival Animation Blueprint - Core animation blueprint for prehistoric survival gameplay
 * Manages state machine for locomotion, survival actions, and fear responses
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API USurvivalAnimationBlueprint : public UAnimBlueprint
{
    GENERATED_BODY()

public:
    USurvivalAnimationBlueprint();

    // Animation state management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    bool bIsRunning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    bool bIsJumping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    bool bIsFearing;

    // Movement parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    float FearLevel;

    // Survival action states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    bool bIsGathering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    bool bIsCrafting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    bool bIsHiding;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    bool bIsClimbing;

    // Animation sequences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sequences")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sequences")
    class UAnimSequence* WalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sequences")
    class UAnimSequence* RunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sequences")
    class UAnimSequence* JumpAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sequences")
    class UAnimSequence* CrouchAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sequences")
    class UAnimSequence* FearAnimation;

    // Blend spaces
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    class UBlendSpace* CrouchBlendSpace;

    // Animation montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    class UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    class UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    class UAnimMontage* ClimbingMontage;

    // Blueprint functions
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void UpdateMovementState(float NewSpeed, float NewDirection);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void SetFearState(bool bNewFearing, float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void PlaySurvivalAction(const FString& ActionName);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void StopAllSurvivalActions();

    // State machine helpers
    UFUNCTION(BlueprintPure, Category = "Survival Animation")
    bool ShouldEnterFearState() const;

    UFUNCTION(BlueprintPure, Category = "Survival Animation")
    bool ShouldEnterRunState() const;

    UFUNCTION(BlueprintPure, Category = "Survival Animation")
    float GetBlendSpaceX() const;

    UFUNCTION(BlueprintPure, Category = "Survival Animation")
    float GetBlendSpaceY() const;

protected:
    // Internal state tracking
    float LastUpdateTime;
    float StateChangeThreshold;
    
    // Animation transition parameters
    UPROPERTY(EditAnywhere, Category = "Animation Settings")
    float BlendInTime;

    UPROPERTY(EditAnywhere, Category = "Animation Settings")
    float BlendOutTime;

    UPROPERTY(EditAnywhere, Category = "Animation Settings")
    float FearTransitionSpeed;

    UPROPERTY(EditAnywhere, Category = "Animation Settings")
    float MovementTransitionSpeed;
};