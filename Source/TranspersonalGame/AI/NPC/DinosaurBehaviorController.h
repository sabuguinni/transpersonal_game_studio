#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../../SharedTypes.h"
#include "DinosaurBehaviorController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class APawn;

/**
 * Advanced AI Controller for dinosaur NPCs with sophisticated behavior patterns
 * Handles pack dynamics, territorial behavior, hunting, and social interactions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Trees for different dinosaur types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    UBehaviorTree* PredatorBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    UBehaviorTree* HerbivoreeBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    UBehaviorTree* PackHunterBehaviorTree;

    // Blackboard Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    UBlackboardData* DinosaurBlackboard;

    // Perception Configuration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
    class UAISenseConfig_Hearing* HearingConfig;

    // Behavior State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENPC_DinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENPC_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TArray<ADinosaurBehaviorController*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    ADinosaurBehaviorController* PackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    bool bIsPackLeader;

    // Territory and Patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Patrol")
    int32 CurrentPatrolIndex;

    // Combat and Hunting
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChaseRange;

    // Social Behavior
    UPROPERTY(BlueprintReadOnly, Category = "Social")
    float LastSocialInteraction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialInteractionCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "Social")
    TMap<ADinosaurBehaviorController*, float> SocialRelationships;

public:
    // Behavior State Management
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "Behavior")
    ENPC_DinosaurBehaviorState GetBehaviorState() const { return CurrentBehaviorState; }

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ADinosaurBehaviorController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void AddPackMember(ADinosaurBehaviorController* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void RemovePackMember(ADinosaurBehaviorController* Member);

    UFUNCTION(BlueprintPure, Category = "Pack")
    bool IsInPack() const { return PackLeader != nullptr || bIsPackLeader; }

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritory(FVector Center, float Radius);

    UFUNCTION(BlueprintPure, Category = "Territory")
    bool IsInTerritory(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void GeneratePatrolPoints();

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack();

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal behavior functions
    void UpdateBehaviorState();
    void UpdatePackBehavior();
    void UpdateTerritorialBehavior();
    void UpdateHuntingBehavior();
    void UpdateSocialBehavior();

    // Utility functions
    FVector GetRandomPatrolPoint() const;
    ADinosaurBehaviorController* FindNearestPackMember() const;
    AActor* FindBestTarget() const;
    bool ShouldFleeFromThreat(AActor* Threat) const;

private:
    // Timers and intervals
    float BehaviorUpdateInterval;
    float LastBehaviorUpdate;
    float PackUpdateInterval;
    float LastPackUpdate;
};