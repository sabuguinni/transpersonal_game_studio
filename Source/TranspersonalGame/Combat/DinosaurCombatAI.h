#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurCombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Attacking,
    Retreating,
    Defending,
    Circling,
    PackCoordination
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FearThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 MaxPackSize = 3;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_DinosaurCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_DinosaurCombatState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<ADinosaurCombatAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinosaurCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void JoinPack(ADinosaurCombatAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInPack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackAttack();

protected:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    void UpdateCombatBehavior(float DeltaTime);
    void ProcessTargetSelection();
    void HandlePackCoordination();
    float CalculateThreatLevel(AActor* Actor) const;
    bool ShouldRetreat() const;
    void ExecuteAttackPattern();
};