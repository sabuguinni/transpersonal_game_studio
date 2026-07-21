#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinosaurAnimInstance.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinoLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpeciesType : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Generic         UMETA(DisplayName = "Generic")
};

/**
 * UDinosaurAnimInstance
 * AnimInstance for all dinosaur pawns. Reads from Blackboard via the owning
 * AIController and drives locomotion, attack, and survival state blends.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion ───────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Locomotion")
    ENPC_DinoLocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Locomotion")
    ENPC_DinoSpeciesType SpeciesType;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Locomotion")
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Locomotion")
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Locomotion")
    float TurnRate;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Locomotion")
    bool bIsSprinting;

    // ─── AI State (mirrored from Blackboard) ─────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|AIState")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|AIState")
    bool bIsAlerted;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|AIState")
    bool bIsFeeding;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|AIState")
    bool bIsFleeing;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|AIState")
    float ThreatLevel;   // 0.0 = calm, 1.0 = maximum threat

    UPROPERTY(BlueprintReadOnly, Category = "NPC|AIState")
    float DistanceToTarget;

    // ─── Survival State ───────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Survival")
    float HealthNormalized;   // 0.0 = dead, 1.0 = full health

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Survival")
    bool bIsWounded;          // HealthNormalized < 0.35

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Survival")
    bool bIsDead;

    // ─── Foot IK (quadrupeds: Triceratops, Brachiosaurus) ────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|FootIK")
    FVector LeftFrontFootEffector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|FootIK")
    FVector RightFrontFootEffector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|FootIK")
    FVector LeftRearFootEffector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|FootIK")
    FVector RightRearFootEffector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|FootIK")
    float IKAlpha;

    UPROPERTY(EditDefaultsOnly, Category = "NPC|FootIK")
    float IKTraceDistance;

    UPROPERTY(EditDefaultsOnly, Category = "NPC|FootIK")
    bool bEnableFootIK;

    // ─── Head Look-At ─────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|LookAt")
    FRotator HeadLookAtRotation;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|LookAt")
    float HeadLookAtAlpha;

private:
    void UpdateLocomotion(float DeltaSeconds);
    void UpdateAIState(float DeltaSeconds);
    void UpdateSurvivalState(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateHeadLookAt(float DeltaSeconds);

    FVector TraceFootPosition(FName SocketName) const;

    APawn* OwnerPawn;
    float PreviousYaw;
};
