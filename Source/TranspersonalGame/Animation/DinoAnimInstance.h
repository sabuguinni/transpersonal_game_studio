#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoAnimInstance.generated.h"

/**
 * FAnim_DinoLocomotionState
 * Locomotion state data for dinosaur animation blending.
 */
USTRUCT(BlueprintType)
struct FAnim_DinoLocomotionState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Dinosaur")
    float Speed = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Dinosaur")
    float Direction = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Dinosaur")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Dinosaur")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Dinosaur")
    bool bIsRoaring = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Dinosaur")
    bool bIsDead = false;
};

/**
 * EAnim_DinoGait
 * Locomotion gait enum for dinosaur movement states.
 */
UENUM(BlueprintType)
enum class EAnim_DinoGait : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Trot        UMETA(DisplayName = "Trot"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Attack      UMETA(DisplayName = "Attack"),
    Roar        UMETA(DisplayName = "Roar"),
    Dead        UMETA(DisplayName = "Dead"),
};

/**
 * UDinoAnimInstance
 * Animation instance for all dinosaur characters.
 * Drives locomotion blend spaces, attack montages, and IK foot placement.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion State ─────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Dinosaur",
              meta = (AllowPrivateAccess = "true"))
    FAnim_DinoLocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Dinosaur",
              meta = (AllowPrivateAccess = "true"))
    EAnim_DinoGait CurrentGait = EAnim_DinoGait::Idle;

    // ── Speed thresholds (tunable per species via Blueprint) ─────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float WalkSpeedThreshold = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float TrotSpeedThreshold = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float RunSpeedThreshold = 450.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float SprintSpeedThreshold = 700.f;

    // ── IK Foot Placement ────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK",
              meta = (AllowPrivateAccess = "true"))
    FVector LeftFootIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK",
              meta = (AllowPrivateAccess = "true"))
    FVector RightFootIKLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    float FootIKTraceDistance = 100.f;

    // ── Functions ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Animation|Dinosaur")
    void SetAttacking(bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Animation|Dinosaur")
    void SetRoaring(bool bRoaring);

    UFUNCTION(BlueprintCallable, Category = "Animation|Dinosaur")
    void SetDead(bool bDead);

    UFUNCTION(BlueprintPure, Category = "Animation|Dinosaur")
    EAnim_DinoGait GetCurrentGait() const { return CurrentGait; }

private:
    void UpdateLocomotionState(float DeltaSeconds);
    void UpdateGait();
    void UpdateFootIK();
    void TraceFootIK(const FName& FootSocketName, FVector& OutIKLocation);

    UPROPERTY()
    class APawn* OwnerPawn = nullptr;
};
