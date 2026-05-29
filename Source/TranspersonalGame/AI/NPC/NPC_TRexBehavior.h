#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NPC_DinosaurBehaviorTree.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "NPC_TRexBehavior.generated.h"

USTRUCT(BlueprintType)
struct FNPC_TRexStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritorialInstinct;

    FNPC_TRexStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        Hunger = 50.0f;
        Stamina = 100.0f;
        Aggression = 75.0f;
        TerritorialInstinct = 90.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TRexBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TRexBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    FNPC_TRexStats TRexStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    UNPC_DinosaurBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float ChaseDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float AttackDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float RoarCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float LastRoarTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    bool bIsHunting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    bool bIsFeeding;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    class APawn* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    FVector HomeTerritory;

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void StartHunting(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void StopHunting();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void StartFeeding();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void StopFeeding();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void PerformRoar();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    bool CanAttack();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    bool IsInAttackRange(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    bool IsInChaseRange(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void UpdateHunger(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void UpdateStamina(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void TakeDamage(float Damage);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    FVector GetRandomPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void SetHomeTerritory(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    bool IsInHomeTerritory();

protected:
    UPROPERTY()
    float HungerDecayRate;

    UPROPERTY()
    float StaminaRecoveryRate;

    UPROPERTY()
    float StaminaDrainRate;

    UPROPERTY()
    float AggressionModifier;

    UFUNCTION()
    void OnTargetLost();

    UFUNCTION()
    void OnTerritoryViolated();

    UFUNCTION()
    APawn* FindNearestPlayer();

    UFUNCTION()
    void UpdateBehaviorState();
};