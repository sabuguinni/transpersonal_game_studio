#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinoLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Attack      UMETA(DisplayName = "Attack"),
    Roar        UMETA(DisplayName = "Roar"),
    Death       UMETA(DisplayName = "Death"),
    Eat         UMETA(DisplayName = "Eat"),
    Sleep       UMETA(DisplayName = "Sleep")
};

USTRUCT(BlueprintType)
struct FAnim_DinoAnimData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Dino")
    float Speed = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Dino")
    float Direction = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Dino")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Dino")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Dino")
    bool bIsRoaring = false;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Dino")
    bool bIsDead = false;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Dino")
    bool bIsEating = false;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Dino")
    EAnim_DinoLocomotionState LocomotionState = EAnim_DinoLocomotionState::Idle;
};

/**
 * UDinoAnimInstance
 * Animation instance for all dinosaur pawns.
 * Drives locomotion blend space, attack montages, and IK foot placement.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    /** Current locomotion state driving the state machine */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    EAnim_DinoLocomotionState LocomotionState;

    /** Packed animation data updated every frame */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    FAnim_DinoAnimData AnimData;

    /** Ground speed (cm/s) used by blend space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    float GroundSpeed;

    /** Movement direction (-180 to 180) for strafe blend */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    float MovementDirection;

    /** True when the pawn is airborne */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    /** True when attack montage is playing */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking;

    /** True when roar montage is playing */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    bool bIsRoaring;

    /** True when death state is active */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    bool bIsDead;

    /** Foot IK alpha — 1.0 = full IK, 0.0 = disabled */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK", meta = (AllowPrivateAccess = "true"))
    float FootIKAlpha;

    /** Left foot IK target location in world space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK", meta = (AllowPrivateAccess = "true"))
    FVector LeftFootIKLocation;

    /** Right foot IK target location in world space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK", meta = (AllowPrivateAccess = "true"))
    FVector RightFootIKLocation;

    /** Play an attack montage — called by AI behavior tree */
    UFUNCTION(BlueprintCallable, Category = "Anim|Dino")
    void TriggerAttack();

    /** Play a roar montage */
    UFUNCTION(BlueprintCallable, Category = "Anim|Dino")
    void TriggerRoar();

    /** Transition to death state */
    UFUNCTION(BlueprintCallable, Category = "Anim|Dino")
    void TriggerDeath();

    /** Update foot IK for uneven terrain */
    UFUNCTION(BlueprintCallable, Category = "Anim|IK")
    void UpdateFootIK(float DeltaSeconds);

protected:
    /** Cached owning pawn */
    UPROPERTY()
    APawn* OwnerPawn;

    /** Resolve locomotion state from speed and flags */
    EAnim_DinoLocomotionState ResolveLocomotionState() const;

    /** Trace foot to ground and return adjusted location */
    FVector TraceFootToGround(const FName& FootSocketName) const;

    /** Walk speed threshold (cm/s) */
    static constexpr float WalkSpeedThreshold = 150.f;

    /** Run speed threshold (cm/s) */
    static constexpr float RunSpeedThreshold = 400.f;
};
