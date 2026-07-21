#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_SurvivalAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Injured     UMETA(DisplayName = "Injured"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Alert       UMETA(DisplayName = "Alert")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector LeftFootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector RightFootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator RightFootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float LeftFootAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float RightFootAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float HipOffset;

    FAnim_FootIKData()
    {
        LeftFootLocation = FVector::ZeroVector;
        RightFootLocation = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        LeftFootAlpha = 0.0f;
        RightFootAlpha = 0.0f;
        HipOffset = 0.0f;
    }
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
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Acceleration;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundDistance;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float LeanAmount;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        bIsInAir = false;
        bIsCrouching = false;
        GroundDistance = 0.0f;
        LeanAmount = 0.0f;
    }
};

/**
 * Advanced Animation Instance for primitive human survival gameplay
 * Handles complex state transitions, IK foot placement, and survival-specific animations
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_SurvivalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_SurvivalAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    UCharacterMovementComponent* CharacterMovement;

    // Current survival state
    UPROPERTY(BlueprintReadOnly, Category = "Survival State")
    EAnim_SurvivalState CurrentSurvivalState;

    UPROPERTY(BlueprintReadOnly, Category = "Survival State")
    EAnim_SurvivalState PreviousSurvivalState;

    // Movement data
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FAnim_MovementData MovementData;

    // Foot IK system
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKInterpSpeed;

    // Survival stats influence on animation
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HungerPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace1D* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* HuntingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* ClimbingMontage;

    // State transition parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Transitions")
    float IdleToWalkThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Transitions")
    float WalkToRunThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Transitions")
    float StateTransitionTime;

    // Environmental awareness
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bIsNearPredator;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bIsNearWater;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bIsOnSteepSlope;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float SlopeAngle;

private:
    // Internal update functions
    void UpdateMovementData(float DeltaTime);
    void UpdateSurvivalState(float DeltaTime);
    void UpdateFootIK(float DeltaTime);
    void UpdateSurvivalStats(float DeltaTime);
    void UpdateEnvironmentalAwareness(float DeltaTime);

    // Foot IK helper functions
    FVector PerformFootTrace(const FVector& FootLocation, const FName& SocketName);
    void CalculateFootIKValues(float DeltaTime);

    // State transition logic
    EAnim_SurvivalState DetermineSurvivalState();
    bool CanTransitionToState(EAnim_SurvivalState NewState);

    // Cached values for smooth transitions
    float CachedSpeed;
    float CachedDirection;
    float StateTransitionTimer;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void PlaySurvivalMontage(EAnim_SurvivalState SurvivalAction);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void SetSurvivalState(EAnim_SurvivalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    bool IsInSurvivalState(EAnim_SurvivalState StateToCheck) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void SetEnvironmentalState(bool bNearPredator, bool bNearWater, bool bOnSteepSlope);

    // Getters for animation blueprint
    UFUNCTION(BlueprintPure, Category = "Movement")
    float GetMovementSpeed() const { return MovementData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Movement")
    float GetMovementDirection() const { return MovementData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Movement")
    bool IsMoving() const { return MovementData.Speed > IdleToWalkThreshold; }

    UFUNCTION(BlueprintPure, Category = "Movement")
    bool IsRunning() const { return MovementData.Speed > WalkToRunThreshold; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    EAnim_SurvivalState GetCurrentSurvivalState() const { return CurrentSurvivalState; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetFootIKData() const { return FootIKData; }
};