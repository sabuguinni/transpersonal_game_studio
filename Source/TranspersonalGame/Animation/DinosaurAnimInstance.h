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
    Roar        UMETA(DisplayName = "Roar"),
    Death       UMETA(DisplayName = "Death"),
};

USTRUCT(BlueprintType)
struct FAnim_DinoLocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsRoaring = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsDead = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    EAnim_DinoLocomotionState LocomotionState = EAnim_DinoLocomotionState::Idle;
};

/**
 * Animation Instance for dinosaur characters.
 * Drives locomotion blend space (Idle/Walk/Run) and action states (Attack, Roar, Death).
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    /** Current locomotion data updated every frame */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Dinosaur", meta = (AllowPrivateAccess = "true"))
    FAnim_DinoLocomotionData LocomotionData;

    /** Current movement speed (0 = idle, >0 = moving) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Dinosaur", meta = (AllowPrivateAccess = "true"))
    float CurrentSpeed = 0.0f;

    /** Current locomotion state for state machine */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Dinosaur", meta = (AllowPrivateAccess = "true"))
    EAnim_DinoLocomotionState CurrentState = EAnim_DinoLocomotionState::Idle;

    /** Walk speed threshold — below this is idle */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float WalkSpeedThreshold = 50.0f;

    /** Run speed threshold — above this is running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float RunSpeedThreshold = 300.0f;

    /** Trigger an attack montage */
    UFUNCTION(BlueprintCallable, Category = "Animation|Dinosaur")
    void TriggerAttack();

    /** Trigger a roar montage */
    UFUNCTION(BlueprintCallable, Category = "Animation|Dinosaur")
    void TriggerRoar();

    /** Set the dinosaur as dead */
    UFUNCTION(BlueprintCallable, Category = "Animation|Dinosaur")
    void SetDead(bool bDead);

private:
    /** Cached owner pawn */
    UPROPERTY()
    class APawn* OwnerPawn = nullptr;
};
