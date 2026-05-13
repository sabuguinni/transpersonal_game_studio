#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SharedTypes.h"
#include "Anim_CharacterAnimInstance.generated.h"

class UAnim_MotionMatchingController;
class UAnim_IKFootPlacement;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

public:
    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* Character;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* CharacterMovement;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    UAnim_MotionMatchingController* MotionMatchingController;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    UAnim_IKFootPlacement* IKFootPlacement;

    // Movement Variables
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Acceleration;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsJumping;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsFalling;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float JumpHeight;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float FallSpeed;

    // Animation State
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_EmotionalState CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsInteracting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsCrafting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsGathering;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CombatAttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CombatBlockMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* InteractionMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* GatheringMontage;

    // IK Variables
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float RightFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float HipOffset;

    // Animation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float MovementThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float JumpThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float FallThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float DirectionSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float SpeedSmoothingSpeed;

    // Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementVariables(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateIKVariables();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayCombatAttack();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayCombatBlock();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayInteraction();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayCrafting();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayGathering();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAllMontages();

    // State Transition Functions
    UFUNCTION(BlueprintCallable, Category = "Animation|State")
    void TransitionToMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation|State")
    void TransitionToEmotionalState(EAnim_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation|State")
    bool CanTransitionToState(EAnim_MovementState TargetState) const;

    // Animation Blending
    UFUNCTION(BlueprintCallable, Category = "Animation|Blending")
    float GetMovementBlendWeight() const;

    UFUNCTION(BlueprintCallable, Category = "Animation|Blending")
    float GetEmotionalBlendWeight() const;

    UFUNCTION(BlueprintCallable, Category = "Animation|Blending")
    float GetCombatBlendWeight() const;

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Animation|Debug")
    void DebugPrintAnimationState();

    UFUNCTION(BlueprintCallable, Category = "Animation|Debug")
    void DebugDrawIKTargets();

private:
    // Internal state tracking
    float PreviousSpeed;
    float PreviousDirection;
    EAnim_MovementState PreviousMovementState;
    EAnim_EmotionalState PreviousEmotionalState;
    
    float StateTransitionTimer;
    float BlendTimer;
    
    bool bWasInAir;
    bool bWasMoving;
    
    // Helper functions
    void InitializeReferences();
    void SmoothMovementVariables(float DeltaTime);
    void UpdateMovementState();
    void UpdateEmotionalState();
    void HandleStateTransitions();
    void ValidateAnimationAssets();
    
    // Movement state detection
    EAnim_MovementState DetermineMovementState() const;
    EAnim_EmotionalState DetermineEmotionalState() const;
    
    // Animation blending helpers
    float CalculateDirectionAngle() const;
    void UpdateBlendWeights(float DeltaTime);
};