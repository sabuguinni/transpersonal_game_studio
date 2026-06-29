#pragma once
// DinosaurAnimInstance.h
// Agent #10 — Animation Agent | PROD_CYCLE_AUTO_20260629_004
// UAnimInstance subclass driving all dinosaur species locomotion and combat.
// Provides blend space inputs, attack montage triggers, and procedural IK.

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DinosaurAnimInstance.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EAnim_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Trot        UMETA(DisplayName = "Trot"),
    Run         UMETA(DisplayName = "Run"),
    Charge      UMETA(DisplayName = "Charge"),
    Swim        UMETA(DisplayName = "Swim")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurAttackType : uint8
{
    None        UMETA(DisplayName = "None"),
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    TailSwipe   UMETA(DisplayName = "Tail Swipe"),
    Charge      UMETA(DisplayName = "Charge"),
    Stomp       UMETA(DisplayName = "Stomp")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurBehaviorState : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ---- Species ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Identity")
    EAnim_DinosaurSpecies Species;

    // ---- Locomotion ----
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    EAnim_DinosaurLocomotionState LocomotionState;

    // ---- Combat ----
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Combat")
    EAnim_DinosaurAttackType CurrentAttack;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Combat")
    float AttackBlendWeight;

    // ---- Behavior ----
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Behavior")
    EAnim_DinosaurBehaviorState BehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Behavior")
    float AlertLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Behavior")
    bool bIsRoaring;

    // ---- Procedural ----
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Procedural")
    float TailSwayAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Procedural")
    float HeadBobAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Procedural")
    float BreathingAlpha;

    // ---- IK ----
    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    FVector LeftFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    FVector RightFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    float FootIKAlpha;

    // ---- Blend Weights ----
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Blend")
    float SpeedNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Blend")
    float AggressionBlend;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Dino|Combat")
    void TriggerAttack(EAnim_DinosaurAttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Dino|Combat")
    void EndAttack();

    UFUNCTION(BlueprintCallable, Category = "Dino|Behavior")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Dino|Behavior")
    void SetBehaviorState(EAnim_DinosaurBehaviorState NewState);

private:
    UPROPERTY()
    APawn* CachedPawn;

    UPROPERTY()
    UCharacterMovementComponent* CachedMovement;

    float ProceduralTime;

    void UpdateLocomotionState();
    void UpdateProceduralAnimation(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
};
