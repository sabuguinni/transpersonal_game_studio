#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "CombatAIController.generated.h"

class UBehaviorTree;
class UBlackboard;

UENUM(BlueprintType)
enum class ECombatAIState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Chasing,
    Attacking,
    Flanking,
    Retreating,
    GroupCoordination
};

UENUM(BlueprintType)
enum class ECombatAIType : uint8
{
    Predator,      // Aggressive, direct attacks
    Herbivore,     // Defensive, group behavior
    Pack,          // Coordinated group attacks
    Alpha,         // Leader behavior, commands others
    Ambush         // Stealth and surprise attacks
};

/**
 * Advanced Combat AI Controller with tactical behavior
 * Implements intelligent enemy behavior with perception, coordination, and adaptive tactics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* PerceptionComponent;

    // AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombatAIType AIType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombatAIState CurrentState;

    // Perception Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRange;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanCallForHelp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float GroupCoordinationRadius;

    // Tactical Behavior
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FVector LastKnownTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float TimeSinceLastSawTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<ACombatAIController*> NearbyAllies;

public:
    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatAIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallForHelp();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateGroupAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetSeen(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void OnTargetLost(AActor* Actor, FAIStimulus Stimulus);

protected:
    void SetupPerception();
    void UpdateNearbyAllies();
    void UpdateCombatState();
    void HandleStateTransitions();

private:
    float LastAllyUpdateTime;
    float StateTransitionCooldown;
    bool bIsCoordinatingAttack;
};