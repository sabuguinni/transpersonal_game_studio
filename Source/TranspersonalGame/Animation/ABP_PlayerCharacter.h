#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ABP_PlayerCharacter.generated.h"

UENUM(BlueprintType)
enum class EPlayerAnimState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"), 
    Running         UMETA(DisplayName = "Running"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Hiding          UMETA(DisplayName = "Hiding"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Falling         UMETA(DisplayName = "Falling"),
    Injured         UMETA(DisplayName = "Injured"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Alert           UMETA(DisplayName = "Alert"),
    Panicked        UMETA(DisplayName = "Panicked")
};

UENUM(BlueprintType)
enum class EPlayerStress : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Nervous         UMETA(DisplayName = "Nervous"),
    Scared          UMETA(DisplayName = "Scared"),
    Terrified       UMETA(DisplayName = "Terrified")
};

/**
 * Animation Blueprint para o protagonista paleontologista
 * Foca em transmitir vulnerabilidade, medo e humanidade através do movimento
 */
UCLASS()
class TRANSPERSONALGAME_API UABP_PlayerCharacter : public UAnimInstance
{
    GENERATED_BODY()

public:
    UABP_PlayerCharacter();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // === CORE MOVEMENT VARIABLES ===
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector2D MovementInput;

    // === EMOTIONAL STATE VARIABLES ===
    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    EPlayerStress StressLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float FearIntensity; // 0.0 = calm, 1.0 = terrified

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float ExhaustionLevel; // 0.0 = fresh, 1.0 = exhausted

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float InjuryLevel; // 0.0 = healthy, 1.0 = severely injured

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    bool bIsBeingHunted; // Predator nearby

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    bool bIsHiding; // Player is trying to hide

    // === ACTIVITY STATE ===
    UPROPERTY(BlueprintReadOnly, Category = "Activity")
    EPlayerAnimState CurrentAnimState;

    UPROPERTY(BlueprintReadOnly, Category = "Activity")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Activity")
    bool bIsGathering;

    UPROPERTY(BlueprintReadOnly, Category = "Activity")
    bool bIsCrafting;

    UPROPERTY(BlueprintReadOnly, Category = "Activity")
    bool bIsClimbing;

    // === ENVIRONMENTAL ADAPTATION ===
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float GroundSlope; // For foot IK adaptation

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bIsOnUnstableGround;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float WeatherIntensity; // Rain, wind affecting posture

    // === MOTION MATCHING VARIABLES ===
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector DesiredVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float TurnRate;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bShouldUseMotionMatching;

    // === IK VARIABLES ===
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    bool bEnableFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootRotation;

    // === BREATHING AND MICRO-MOVEMENTS ===
    UPROPERTY(BlueprintReadOnly, Category = "Breathing")
    float BreathingRate; // Affected by stress and exhaustion

    UPROPERTY(BlueprintReadOnly, Category = "Breathing")
    float HeartRate; // Affects subtle body movements

    UPROPERTY(BlueprintReadOnly, Category = "Micro Movements")
    float NervousTwitchIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Micro Movements")
    float LookAroundFrequency; // How often character looks around when scared

private:
    // Character reference
    class ACharacter* OwningCharacter;
    class UCharacterMovementComponent* MovementComponent;
    
    // Internal calculation methods
    void UpdateMovementVariables();
    void UpdateEmotionalState();
    void UpdateActivityState();
    void UpdateEnvironmentalAdaptation();
    void UpdateFootIK();
    void UpdateBreathingAndMicroMovements();
    
    // Helper methods for emotional state calculation
    float CalculateFearFromNearbyThreats();
    float CalculateExhaustionFromActivity();
    EPlayerStress DetermineStressLevel();
};