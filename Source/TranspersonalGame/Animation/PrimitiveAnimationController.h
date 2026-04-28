#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "../SharedTypes.h"
#include "PrimitiveAnimationController.generated.h"

// Animation state enums for primitive character movement
UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Unarmed         UMETA(DisplayName = "Unarmed"),
    MeleeReady      UMETA(DisplayName = "Melee Ready"),
    RangedReady     UMETA(DisplayName = "Ranged Ready"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Dodging         UMETA(DisplayName = "Dodging"),
    Stunned         UMETA(DisplayName = "Stunned")
};

// Data structures for animation parameters
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float GroundDistance = 0.0f;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        GroundDistance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_CombatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsAttacking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsBlocking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bHasWeapon = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 ComboIndex = 0;

    FAnim_CombatData()
    {
        bIsAttacking = false;
        bIsBlocking = false;
        bHasWeapon = false;
        AttackSpeed = 1.0f;
        ComboIndex = 0;
    }
};

/**
 * Core animation controller for primitive characters
 * Manages movement and combat animation states with realistic transitions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPrimitiveAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPrimitiveAnimationController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState(const FAnim_MovementData& MovementData);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCombatState(const FAnim_CombatData& CombatData);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAllMontages();

    // State getters
    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_CombatState GetCurrentCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_CombatData GetCombatData() const { return CombatData; }

protected:
    // Current animation states
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_CombatState CurrentCombatState;

    // Animation data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation Data")
    FAnim_MovementData MovementData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation Data")
    FAnim_CombatData CombatData;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> MovementBlendSpace;

    // State transition parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float WalkSpeedThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float StateTransitionTime = 0.2f;

private:
    // Internal state management
    void DetermineMovementState();
    void DetermineCombatState();
    void HandleStateTransitions();

    // Animation instance reference
    UPROPERTY()
    TObjectPtr<UAnimInstance> AnimInstance;

    // Timing for smooth transitions
    float LastStateChangeTime;
    EAnim_MovementState PreviousMovementState;
    EAnim_CombatState PreviousCombatState;
};