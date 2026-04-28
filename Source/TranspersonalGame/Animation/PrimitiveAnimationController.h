#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "PrimitiveAnimationController.generated.h"

class ACharacter;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    MeleeReady  UMETA(DisplayName = "Melee Ready"),
    RangedReady UMETA(DisplayName = "Ranged Ready"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Blocking    UMETA(DisplayName = "Blocking")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_CombatData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EAnim_CombatState CombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float AttackSpeed;

    FAnim_CombatData()
    {
        CombatState = EAnim_CombatState::Unarmed;
        bIsAttacking = false;
        bIsBlocking = false;
        AttackSpeed = 1.0f;
    }
};

/**
 * Primitive Animation Controller - Manages basic character animations
 * Handles movement states, combat states, and animation blending
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPrimitiveAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPrimitiveAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Movement Animation Control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementAnimation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetMovementState() const { return MovementData.MovementState; }

    // Combat Animation Control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatState(EAnim_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAttackAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerBlockAnimation();

    // Animation Data Access
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_CombatData GetCombatData() const { return CombatData; }

    // Animation Utilities
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontage(class UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAllMontages();

protected:
    // Owner character reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    ACharacter* OwnerCharacter;

    // Animation data
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FAnim_MovementData MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FAnim_CombatData CombatData;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float WalkThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float RunThreshold = 300.0f;

    // Animation timing
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float TimeSinceLastAttack = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float AttackCooldown = 1.0f;

private:
    // Internal state management
    void UpdateMovementState();
    void UpdateCombatState();
    void CalculateMovementDirection();
    
    // Cache frequently accessed components
    UCharacterMovementComponent* MovementComponent;
    USkeletalMeshComponent* MeshComponent;
};