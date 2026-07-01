#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

// ============================================================
// Enums — declared at global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Death       UMETA(DisplayName = "Death")
};

UENUM(BlueprintType)
enum class EAnim_StanceType : uint8
{
    Standing    UMETA(DisplayName = "Standing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Prone       UMETA(DisplayName = "Prone")
};

// ============================================================
// Structs — declared at global scope (UHT requirement)
// ============================================================

USTRUCT(BlueprintType)
struct FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float LeanAngle = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    FVector Acceleration = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FAnim_FootIKState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float PelvisOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float LeftFootOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float RightFootOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    bool bOnUnevenTerrain = false;
};

USTRUCT(BlueprintType)
struct FAnim_AimingData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Aiming")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Aiming")
    float AimYaw = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Aiming")
    bool bIsAiming = false;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Aiming")
    FVector AimTarget = FVector::ZeroVector;
};

// ============================================================
// Main AnimInstance class
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // ---- UAnimInstance interface ----
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeBeginPlay() override;

    // ---- Locomotion state ----
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    FAnim_LocomotionData LocomotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_StanceType StanceType;

    // ---- Foot IK ----
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FAnim_FootIKState FootIKState;

    // ---- Aiming ----
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Aiming")
    FAnim_AimingData AimingData;

    // ---- Blend weights ----
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float UpperBodyBlendWeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float FootIKBlendWeight = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float LandingBlendWeight = 0.0f;

    // ---- Survival state (drives animation overlays) ----
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float HealthNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsInjured = false;

    // ---- Configuration ----
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float WalkSpeedThreshold = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float RunSpeedThreshold = 250.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float SprintSpeedThreshold = 500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float FootIKTraceDistance = 55.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float FootIKInterpSpeed = 12.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float MaxFootOffset = 15.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    FName LeftFootBoneName = FName("foot_l");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    FName RightFootBoneName = FName("foot_r");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    FName PelvisBoneName = FName("pelvis");

    // ---- Blueprint callable utilities ----
    UFUNCTION(BlueprintCallable, Category = "Anim|Locomotion")
    EAnim_LocomotionState GetLocomotionState() const { return LocomotionState; }

    UFUNCTION(BlueprintCallable, Category = "Anim|FootIK")
    FAnim_FootIKState GetFootIKState() const { return FootIKState; }

    UFUNCTION(BlueprintCallable, Category = "Anim|Locomotion")
    float GetSpeedNormalized() const;

    UFUNCTION(BlueprintCallable, Category = "Anim|Locomotion")
    bool ShouldPlayLandAnimation() const;

private:
    // ---- Internal update methods ----
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimingData();
    void UpdateSurvivalOverlays();

    // ---- Foot IK helpers ----
    bool TraceFootPosition(const FName& BoneName, FVector& OutLocation, FRotator& OutRotation);
    float InterpFootOffset(float Current, float Target, float DeltaSeconds) const;

    // ---- Cached references ----
    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    // ---- Internal state ----
    float TimeSinceLanded = 0.0f;
    float LandingAnimDuration = 0.35f;
    FVector PreviousVelocity = FVector::ZeroVector;
};
