#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

USTRUCT(BlueprintType)
struct FCombat_TacticalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombat_AIBehavior CurrentBehavior = ECombat_AIBehavior::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float HealthPercentage = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float TimeSinceLastPlayerSighting = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    bool bCanSeePlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float DistanceToPlayer = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_TacticalAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Tree Asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBlackboardData* BlackboardAsset;

    // Tactical State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    FCombat_TacticalState TacticalState;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float FleeHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float AggressionDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float FearDecayRate = 0.05f;

public:
    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalState();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIBehavior DetermineBestBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteTacticalBehavior(ECombat_AIBehavior NewBehavior);

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    void InitiateAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    void InitiateFlee();

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    void InitiateHunt();

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    void InitiatePatrol();

    // Perception Events
    UFUNCTION()
    void OnPlayerSighted(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void OnPlayerLost(AActor* Actor, FAIStimulus Stimulus);

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Combat State")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "Combat State")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintPure, Category = "Combat State")
    bool ShouldAttack() const;

    UFUNCTION(BlueprintPure, Category = "Combat State")
    FVector GetOptimalAttackPosition() const;

    UFUNCTION(BlueprintPure, Category = "Combat State")
    FVector GetFleeDirection() const;

protected:
    void SetupAIPerception();
    void UpdateBlackboard();
    void CalculateAggressionAndFear();
    
    // Tactical positioning
    FVector FindFlankingPosition();
    FVector FindAmbushPosition();
    bool HasClearLineOfSight(const FVector& TargetLocation) const;
};