#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinosaurAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinoLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Trotting    UMETA(DisplayName = "Trotting"),
    Running     UMETA(DisplayName = "Running"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Roaring     UMETA(DisplayName = "Roaring"),
    Eating      UMETA(DisplayName = "Eating"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_DinoBodySize : uint8
{
    Small       UMETA(DisplayName = "Small"),    // Velociraptor
    Medium      UMETA(DisplayName = "Medium"),   // Dilophosaurus
    Large       UMETA(DisplayName = "Large"),    // T-Rex
    Massive     UMETA(DisplayName = "Massive")   // Brachiosaurus
};

/**
 * UDinosaurAnimInstance
 * AnimInstance for all dinosaur pawns in the prehistoric survival game.
 * Drives locomotion blend spaces, attack montages, and procedural IK.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion State ───────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    EAnim_DinoLocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|Locomotion")
    bool bIsDead;

    // ─── Body & Species ─────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Species")
    EAnim_DinoBodySize BodySize;

    /** Normalised [0-1] aggression level — drives blend between patrol and hunt anims */
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Behaviour")
    float AggressionLevel;

    /** Normalised [0-1] health ratio — drives limping / wounded blend */
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Behaviour")
    float HealthRatio;

    // ─── Procedural IK ──────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    FVector LeftFootIKTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    FVector RightFootIKTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|IK")
    float IKFootBlendWeight;

    // ─── Head Look-At ────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|LookAt")
    FRotator HeadLookAtRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|LookAt")
    float HeadLookAtWeight;

    // ─── Tail Physics ────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Physics")
    float TailSwayAmount;

    // ─── Attack / Montage Triggers ───────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dino|Animation")
    void TriggerAttackMontage();

    UFUNCTION(BlueprintCallable, Category = "Dino|Animation")
    void TriggerRoarMontage();

    UFUNCTION(BlueprintCallable, Category = "Dino|Animation")
    void TriggerDeathMontage();

protected:
    /** Cached owning pawn */
    UPROPERTY()
    APawn* OwnerPawn;

    /** Cached movement component */
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

private:
    void UpdateLocomotionState();
    void UpdateIKTargets(float DeltaSeconds);
    void UpdateHeadLookAt(float DeltaSeconds);
    void UpdateTailSway(float DeltaSeconds);

    float TailSwayTime;
};
