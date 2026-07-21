#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Combat_DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurCombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Defending   UMETA(DisplayName = "Defending")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AggressionLevel;

    FCombat_DinosaurStats()
    {
        Health = 100.0f;
        AttackDamage = 25.0f;
        AttackRange = 200.0f;
        MovementSpeed = 300.0f;
        DetectionRadius = 1000.0f;
        AggressionLevel = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurCombatAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_DinosaurCombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_DinosaurCombatState GetCombatState() const { return CurrentCombatState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFleeingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartPatrolling();

    // Detection System
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool DetectNearbyThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* FindClosestPlayer();

    // Species Configuration
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ConfigureForSpecies(ECombat_DinosaurSpecies Species);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    ECombat_DinosaurCombatState CurrentCombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    ECombat_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    FCombat_DinosaurStats CombatStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Target")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float AttackCooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    float StateChangeInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Behavior")
    float LastStateChangeTime;

private:
    void UpdateCombatBehavior(float DeltaTime);
    void ProcessIdleState();
    void ProcessPatrollingState();
    void ProcessHuntingState();
    void ProcessAttackingState();
    void ProcessFleeingState();
    void ProcessDefendingState();
    
    bool IsTargetInRange() const;
    bool CanAttack() const;
    void MoveTowardsTarget();
    void MoveAwayFromTarget();
};