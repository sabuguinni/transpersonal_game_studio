#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "SharedTypes.h"
#include "Combat_DinosaurCombatAI.generated.h"

class APawn;
class UBehaviorTree;

UENUM(BlueprintType)
enum class ECombat_AggressionLevel : uint8
{
    Passive = 0,
    Defensive = 1,
    Aggressive = 2,
    Enraged = 3
};

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol = 0,
    Investigate = 1,
    Engage = 2,
    Retreat = 3,
    Ambush = 4
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Stamina = 100.0f;

    FCombat_DinosaurStats()
    {
        AttackDamage = 50.0f;
        AttackRange = 300.0f;
        DetectionRadius = 1500.0f;
        MovementSpeed = 400.0f;
        Health = 100.0f;
        Stamina = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIHearingConfig* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBlackboardComponent* BlackboardComponent;

    // Combat Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_DinosaurStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AggressionLevel AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_TacticalState TacticalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatCooldown = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat;

    // Pack Behavior (for Velociraptors)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackHunter;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    TArray<ACombat_DinosaurCombatAI*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    ACombat_DinosaurCombatAI* PackLeader;

public:
    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EnterCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExitCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetAggressionLevel(ECombat_AggressionLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTacticalState(ECombat_TacticalState NewState);

    // Pack Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(ACombat_DinosaurCombatAI* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackAttack(AActor* Target);

    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal Combat Logic
    void UpdateCombatBehavior(float DeltaTime);
    void UpdatePackBehavior(float DeltaTime);
    void SelectBestTarget();
    void ExecuteTacticalManeuver();
    
    // Utility Functions
    bool IsTargetValid() const;
    bool IsTargetInRange() const;
    bool HasLineOfSight() const;
    void UpdateBlackboardValues();
};