#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinosaurAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinoLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Attack      UMETA(DisplayName = "Attack"),
    Death       UMETA(DisplayName = "Death"),
    Roar        UMETA(DisplayName = "Roar"),
    Eat         UMETA(DisplayName = "Eat"),
};

UENUM(BlueprintType)
enum class EAnim_DinoSpeciesType : uint8
{
    Biped_Large     UMETA(DisplayName = "Biped Large (T-Rex)"),
    Biped_Small     UMETA(DisplayName = "Biped Small (Raptor)"),
    Quadruped_Large UMETA(DisplayName = "Quadruped Large (Brachio)"),
    Quadruped_Med   UMETA(DisplayName = "Quadruped Medium (Trike)"),
    Quadruped_Small UMETA(DisplayName = "Quadruped Small (Ankylo)"),
};

/**
 * UDinosaurAnimInstance
 * Animation instance for all dinosaur species in the prehistoric survival game.
 * Drives locomotion blend spaces, attack montages, and foot IK adaptation.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion State ──
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_DinoLocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_DinoSpeciesType SpeciesType;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float MaxWalkSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float SpeedNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsDead;

    // ── Foot IK ──
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float FootIKAlpha;

    // ── Head Look-At ──
    UPROPERTY(BlueprintReadOnly, Category = "Anim|LookAt")
    FRotator HeadLookAtRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|LookAt")
    float HeadLookAtAlpha;

    // ── Breathing ──
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Breathing")
    float BreathingCycle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float RunSpeedThreshold;

    // ── Blueprint callable helpers ──
    UFUNCTION(BlueprintCallable, Category = "Anim|State")
    void SetLocomotionState(EAnim_DinoLocomotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Anim|State")
    void TriggerAttack();

    UFUNCTION(BlueprintCallable, Category = "Anim|State")
    void TriggerDeath();

    UFUNCTION(BlueprintCallable, Category = "Anim|State")
    void TriggerRoar();

    UFUNCTION(BlueprintPure, Category = "Anim|Locomotion")
    float GetSpeedNormalized() const { return SpeedNormalized; }

    UFUNCTION(BlueprintPure, Category = "Anim|Locomotion")
    bool GetIsMoving() const { return bIsMoving; }

private:
    void UpdateLocomotionFromSpeed();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateBreathing(float DeltaSeconds);
    void UpdateHeadLookAt(float DeltaSeconds);

    float BreathingTimer;
    float AttackCooldown;
    APawn* OwnerPawn;
};
