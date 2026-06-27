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
    Eating      UMETA(DisplayName = "Eating"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct FAnim_DinoLocomotionData
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
 * Animation Instance for all dinosaur characters.
 * Drives locomotion blend space, attack montages, and procedural IK.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    /** Current locomotion data updated every frame from owning pawn */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    FAnim_DinoLocomotionData LocomotionData;

    /** Ground speed used for blend space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    float GroundSpeed = 0.f;

    /** Direction (-180 to 180) for strafe blend */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    float MovementDirection = 0.f;

    /** Whether the dino is currently in the air */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir = false;

    /** Whether the dino is performing an attack */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking = false;

    /** Whether the dino is roaring */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    bool bIsRoaring = false;

    /** Whether the dino is dead */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    bool bIsDead = false;

    /** Whether the dino is eating */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    bool bIsEating = false;

    /** Current locomotion state enum */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Dino", meta = (AllowPrivateAccess = "true"))
    EAnim_DinoLocomotionState LocomotionState = EAnim_DinoLocomotionState::Idle;

    /** Trigger attack montage — called by Combat AI */
    UFUNCTION(BlueprintCallable, Category = "Anim|Dino")
    void TriggerAttack();

    /** Trigger roar montage — called by NPC Behavior */
    UFUNCTION(BlueprintCallable, Category = "Anim|Dino")
    void TriggerRoar();

    /** Trigger death montage — called by health system */
    UFUNCTION(BlueprintCallable, Category = "Anim|Dino")
    void TriggerDeath();

    /** Set eating state */
    UFUNCTION(BlueprintCallable, Category = "Anim|Dino")
    void SetEating(bool bEating);

protected:
    /** Cached owning pawn */
    UPROPERTY()
    class APawn* OwnerPawn = nullptr;

    /** Cached character movement component */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComp = nullptr;

    /** Determine locomotion state from current speed and flags */
    EAnim_DinoLocomotionState DetermineLocomotionState() const;

    /** Walk speed threshold */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Dino|Thresholds")
    float WalkSpeedThreshold = 50.f;

    /** Run speed threshold */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Dino|Thresholds")
    float RunSpeedThreshold = 300.f;
};
