#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Combat_AIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Chasing,
    Attacking,
    Fleeing,
    Circling,
    Coordinating
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Tactical")
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical")
    float TimeSinceLastSighting;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical")
    bool bCanSeePlayer;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical")
    float DistanceToPlayer;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical")
    TArray<AActor*> NearbyAllies;

    FCombat_TacticalData()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSinceLastSighting = 0.0f;
        ThreatLevel = ECombat_ThreatLevel::None;
        bIsInCombat = false;
        bCanSeePlayer = false;
        DistanceToPlayer = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_AIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    UBehaviorTree* BehaviorTree;

    // Combat AI State
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombat_AIState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FCombat_TacticalData TacticalData;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float FleeHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CirclingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CoordinationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    bool bUsePackTactics;

public:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalData();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateWithPack();

    // Blackboard Keys
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Blackboard")
    FName PlayerActorKey;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Blackboard")
    FName LastKnownLocationKey;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Blackboard")
    FName CombatStateKey;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Blackboard")
    FName CanSeePlayerKey;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Blackboard")
    FName ThreatLevelKey;

private:
    void InitializePerception();
    void InitializeBlackboard();
    void UpdateBlackboardData();
    void ProcessPlayerSighting(AActor* PlayerActor);
    void HandleCombatLogic();
    void FindNearbyAllies();

    float LastUpdateTime;
    AActor* CurrentTarget;
};