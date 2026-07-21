#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoAnimInstance.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinoLocoState : uint8
{
    Resting     UMETA(DisplayName = "Resting"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Feed        UMETA(DisplayName = "Feed"),
    Roar        UMETA(DisplayName = "Roar"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoGait : uint8
{
    Stationary  UMETA(DisplayName = "Stationary"),
    Walk        UMETA(DisplayName = "Walk"),
    Trot        UMETA(DisplayName = "Trot"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint")
};

USTRUCT(BlueprintType)
struct FNPC_DinoFootIK
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    bool bGroundContact = false;
};

USTRUCT(BlueprintType)
struct FNPC_DinoLeanData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Lean")
    float LeanForwardBack = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Lean")
    float LeanLeftRight = 0.0f;
};

/**
 * UDinoAnimInstance
 * Animation instance for all dinosaur pawns.
 * Drives locomotion state machine, foot IK terrain adaptation,
 * directional lean, and attack root motion.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    ENPC_DinoLocoState LocoState = ENPC_DinoLocoState::Resting;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    ENPC_DinoGait CurrentGait = ENPC_DinoGait::Stationary;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    float VerticalSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    bool bIsRoaring = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    bool bIsDead = false;

    // ── Speed thresholds (tunable in editor) ────────────────────
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dino|Thresholds")
    float WalkSpeed = 150.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dino|Thresholds")
    float TrotSpeed = 400.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dino|Thresholds")
    float RunSpeed = 800.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dino|Thresholds")
    float SprintSpeed = 1400.0f;

    // ── Foot IK ─────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    FNPC_DinoFootIK FrontLeftFoot;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    FNPC_DinoFootIK FrontRightFoot;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    FNPC_DinoFootIK RearLeftFoot;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    FNPC_DinoFootIK RearRightFoot;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    float PelvisOffset = 0.0f;

    // ── Lean ────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Lean")
    FNPC_DinoLeanData LeanData;

    // ── Head tracking ───────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Head")
    FRotator HeadAimRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Head")
    float HeadTrackAlpha = 0.0f;

    // ── State setters (called by AI controller) ──────────────────
    UFUNCTION(BlueprintCallable, Category = "Dino|State")
    void SetLocoState(ENPC_DinoLocoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dino|State")
    void SetAttacking(bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Dino|State")
    void SetRoaring(bool bRoaring);

    UFUNCTION(BlueprintCallable, Category = "Dino|State")
    void SetDead(bool bDead);

    UFUNCTION(BlueprintCallable, Category = "Dino|Head")
    void SetHeadTarget(FVector WorldTarget, float Alpha);

private:
    void UpdateGait();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateLean(float DeltaSeconds);
    void UpdateHeadTracking(float DeltaSeconds);
    FNPC_DinoFootIK TraceFootIK(FName SocketName, float TraceLength = 80.0f);

    FVector PreviousVelocity = FVector::ZeroVector;
    FVector HeadTargetWorld = FVector::ZeroVector;
    float LeanInterpSpeed = 6.0f;
    float HeadInterpSpeed = 4.0f;
};
