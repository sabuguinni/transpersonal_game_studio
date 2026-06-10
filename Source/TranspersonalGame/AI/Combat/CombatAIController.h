#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../../SharedTypes.h"
#include "CombatAIController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AIState CurrentState = ECombat_AIState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PatrolRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TimeSinceLastContact = 0.0f;

    FCombat_TacticalState()
    {
        CurrentState = ECombat_AIState::Patrol;
        AggressionLevel = 0.5f;
        PackSize = 1;
        AttackRange = 300.0f;
        PatrolRadius = 500.0f;
        CurrentTarget = nullptr;
        LastKnownTargetLocation = FVector::ZeroVector;
        TimeSinceLastContact = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    // Combat State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_TacticalState TacticalState;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BaseAggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SightRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LoseSightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FieldOfViewAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HearingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold = 0.2f;

    // Pack Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<ACombatAIController*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCoordinationRadius = 1000.0f;

public:
    // AI Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_AIState GetCombatState() const { return TacticalState.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetCurrentTarget() const { return TacticalState.CurrentTarget; }

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ACombatAIController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void BroadcastThreatToPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    FVector GetFlankingPosition(AActor* Target);

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldInvestigate(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateTacticalState(float DeltaTime);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool HasLineOfSightToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetOptimalAttackPosition(AActor* Target);

protected:
    void InitializeAIPerception();
    void InitializeBehaviorTree();
    void UpdateAggressionBasedOnHealth();
    void HandleTargetLost();
    void CoordinateWithPackMembers();
    bool IsTargetThreatening(AActor* Target);
    float CalculateThreatLevel(AActor* Target);
};