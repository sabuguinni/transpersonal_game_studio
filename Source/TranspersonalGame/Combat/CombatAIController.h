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

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing UMETA(DisplayName = "Chasing"),
    Attacking UMETA(DisplayName = "Attacking"),
    Flanking UMETA(DisplayName = "Flanking"),
    Retreating UMETA(DisplayName = "Retreating"),
    Coordinating UMETA(DisplayName = "Coordinating"),
    Dead UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_TacticalRole : uint8
{
    Lone_Hunter UMETA(DisplayName = "Lone Hunter"),
    Pack_Leader UMETA(DisplayName = "Pack Leader"),
    Pack_Member UMETA(DisplayName = "Pack Member"),
    Ambush_Predator UMETA(DisplayName = "Ambush Predator"),
    Tank UMETA(DisplayName = "Tank"),
    Scout UMETA(DisplayName = "Scout"),
    Support UMETA(DisplayName = "Support")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    ECombat_TacticalRole Role = ECombat_TacticalRole::Lone_Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float CautiousLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float PackCoordination = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float PreferredAttackDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float FlankingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bCanCallForHelp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bCanFlank = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bCanRetreat = true;

    FCombat_TacticalData()
    {
        Role = ECombat_TacticalRole::Lone_Hunter;
        AggressionLevel = 0.5f;
        CautiousLevel = 0.3f;
        PackCoordination = 0.7f;
        PreferredAttackDistance = 300.0f;
        FlankingRadius = 800.0f;
        bCanCallForHelp = true;
        bCanFlank = true;
        bCanRetreat = true;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTree;

    // Tactical Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_TacticalData TacticalData;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_AIState CurrentState;

    // Target Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    FVector LastKnownTargetLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float TimeSinceLastTargetSeen;

    // Pack Coordination
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<ACombatAIController*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ACombatAIController* PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    bool bIsPackLeader;

public:
    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetAIState() const { return CurrentState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void LoseTarget();

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFlank() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldCallForHelp() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetRetreatPosition() const;

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void JoinPack(ACombatAIController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void BecomePackLeader();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallPackToTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack();

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal Methods
    void UpdateBlackboard();
    void EvaluateTacticalSituation();
    void ProcessPackCommunication();
    bool IsInAttackRange() const;
    bool IsTargetVisible() const;
    float GetDistanceToTarget() const;
    void UpdatePackFormation();

private:
    // Timers and Cooldowns
    float LastTacticalEvaluation;
    float TacticalEvaluationInterval;
    float LastPackCommunication;
    float PackCommunicationInterval;
};