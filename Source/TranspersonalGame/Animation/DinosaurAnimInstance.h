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
    Eat         UMETA(DisplayName = "Eat"),
    Sleep       UMETA(DisplayName = "Sleep"),
    Roar        UMETA(DisplayName = "Roar"),
    Death       UMETA(DisplayName = "Death"),
};

UENUM(BlueprintType)
enum class EAnim_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Generic         UMETA(DisplayName = "Generic"),
};

USTRUCT(BlueprintType)
struct FAnim_DinoLocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsEating = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsDead = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    float TurnRate = 0.0f;
};

/**
 * Animation Instance for all dinosaur species.
 * Drives locomotion blend spaces, attack montages, and IK foot placement.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // Current locomotion state
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_DinoLocomotionState LocomotionState;

    // Species type for species-specific anim logic
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Species")
    EAnim_DinoSpecies DinoSpecies;

    // Locomotion data updated every frame
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    FAnim_DinoLocomotionData LocomotionData;

    // Ground speed (magnitude of velocity XY)
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float GroundSpeed = 0.0f;

    // Walk/Run threshold — species-specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float RunSpeedThreshold = 400.0f;

    // Foot IK enabled flag
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    bool bEnableFootIK = true;

    // IK foot trace distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    float FootIKTraceDistance = 100.0f;

    // Left foot IK target
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKTarget = FVector::ZeroVector;

    // Right foot IK target
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKTarget = FVector::ZeroVector;

    // IK alpha (0=off, 1=full)
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float FootIKAlpha = 1.0f;

    // Roar trigger (set by AI, consumed by anim)
    UPROPERTY(BlueprintReadWrite, Category = "Animation|Events")
    bool bTriggerRoar = false;

    // Attack trigger
    UPROPERTY(BlueprintReadWrite, Category = "Animation|Events")
    bool bTriggerAttack = false;

protected:
    // Update locomotion state from pawn velocity
    void UpdateLocomotionState();

    // Update foot IK targets via line traces
    void UpdateFootIK(float DeltaSeconds);

    // Resolve locomotion enum from speed
    EAnim_DinoLocomotionState ResolveLocomotionState(float Speed, bool bAttacking, bool bEating, bool bDead) const;

private:
    // Cached owner pawn
    class APawn* OwnerPawn = nullptr;

    // Smooth foot IK alpha
    float FootIKAlphaTarget = 1.0f;
};
