#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "Combat_DinosaurCombatSystem.generated.h"

class ATranspersonalCharacter;
class APawn;

UENUM(BlueprintType)
enum class ECombat_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 600.0f;

    FCombat_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        AttackDamage = 25.0f;
        DetectionRadius = 2000.0f;
        AttackRange = 300.0f;
        MovementSpeed = 600.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurCombatSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurCombatSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat state management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombat_DinosaurState CurrentState = ECombat_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    FCombat_DinosaurStats DinosaurStats;

    // Target tracking
    UPROPERTY(BlueprintReadOnly, Category = "Combat Target")
    ATranspersonalCharacter* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    bool bIsAggressive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    float StateUpdateInterval = 0.5f;

    // Combat functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool DetectNearbyTargets();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartHunting(ATranspersonalCharacter* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StopHunting();

private:
    float LastStateUpdateTime = 0.0f;
    float LastAttackTime = 0.0f;
    float AttackCooldown = 2.0f;

    void UpdateCombatBehavior();
    void HandleIdleState();
    void HandleHuntingState();
    void HandleAttackingState();
};