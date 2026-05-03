#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "CombatAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class APawn;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Possess(APawn* InPawn) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    UBlackboardData* BlackboardAsset;

    // Combat Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FlankingDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_AIState CurrentAIState = ECombat_AIState::Patrolling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_TacticalRole TacticalRole = ECombat_TacticalRole::Flanker;

public:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetAIState() const { return CurrentAIState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTacticalRole(ECombat_TacticalRole NewRole);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_TacticalRole GetTacticalRole() const { return TacticalRole; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const;

    // Tactical Decisions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackTarget() const;

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateWithPack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<ACombatAIController*> GetNearbyPackMembers(float SearchRadius = 1000.0f);

private:
    // Internal state tracking
    AActor* CurrentTarget;
    FVector LastKnownTargetLocation;
    float LastTargetSightTime;
    bool bIsInCombat;

    // Pack coordination
    TArray<TWeakObjectPtr<ACombatAIController>> PackMembers;
    float LastPackCoordinationTime;

    // Helper functions
    void UpdateBlackboardValues();
    void HandleTargetLost();
    void HandleTargetAcquired(AActor* NewTarget);
    bool IsTargetInRange(AActor* Target, float Range) const;
    FVector GetRandomFlankingPosition(AActor* Target) const;
};