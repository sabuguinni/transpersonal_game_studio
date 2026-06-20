#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoAnimInstance.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinoLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Trot        UMETA(DisplayName = "Trot"),
    Run         UMETA(DisplayName = "Run"),
    Attack      UMETA(DisplayName = "Attack"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoAlertLevel : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Curious     UMETA(DisplayName = "Curious"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    float LeanAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    ENPC_DinoLocomotionState LocomotionState;

    // ── Predator States ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Behavior")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Behavior")
    bool bIsAlert;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Behavior")
    bool bIsFeeding;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Behavior")
    bool bIsSleeping;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Behavior")
    bool bIsFleeing;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Behavior")
    ENPC_DinoAlertLevel AlertLevel;

    // ── Pack Dynamics ────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Pack")
    float PackThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Pack")
    bool bIsPackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Pack")
    int32 NearbyPackMemberCount;

    // ── Foot IK ──────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|FootIK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|FootIK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|FootIK")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|FootIK")
    FRotator RightFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|FootIK")
    float FootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|FootIK")
    float PelvisOffset;

    // ── Aim / Head Tracking ──────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|AimOffset")
    float HeadPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|AimOffset")
    float HeadYaw;

    // ── Damage / Health ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Health")
    float HealthNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Health")
    bool bIsInjured;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Health")
    bool bIsDead;

    // ── Attack ───────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Attack")
    float AttackPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Attack")
    bool bBiteActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Attack")
    bool bTailSwipeActive;

    // ── Tail Physics ─────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Tail")
    float TailSwingAmount;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Tail")
    float TailSwingSpeed;

private:
    UPROPERTY()
    class APawn* OwnerPawn;

    float PreviousSpeed;

    bool TraceFootIK(FName SocketName, FVector& OutLocation, FRotator& OutRotation);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateLocomotionState();
    void UpdateTailPhysics(float DeltaSeconds);
};
