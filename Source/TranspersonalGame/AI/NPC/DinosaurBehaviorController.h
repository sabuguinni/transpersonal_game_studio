#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "SharedTypes.h"
#include "DinosaurBehaviorController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UBehaviorTree;

UENUM(BlueprintType)
enum class ENPC_DinosaurBehaviorState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Chasing,
    Attacking,
    Fleeing,
    Grazing,
    Sleeping,
    Hunting
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float ChaseDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float FleeDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    bool bIsHerbivore = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    ENPC_DinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "AI Targets")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI Targets")
    FVector PatrolCenter;

    UPROPERTY(BlueprintReadOnly, Category = "AI Targets")
    FVector LastKnownPlayerLocation;

public:
    // Behavior control functions
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetCurrentTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void UpdatePatrolCenter(FVector NewCenter);

    UFUNCTION(BlueprintPure, Category = "AI Behavior")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "AI Behavior")
    bool IsPlayerInChaseRange() const;

    UFUNCTION(BlueprintPure, Category = "AI Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "AI Behavior")
    bool ShouldFleeFromPlayer() const;

    // Pack behavior functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(ADinosaurBehaviorController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    bool IsInPack() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CallPackToHunt(AActor* Target);

protected:
    // AI Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Pack behavior data
    UPROPERTY()
    ADinosaurBehaviorController* PackLeader;

    UPROPERTY()
    TArray<ADinosaurBehaviorController*> PackMembers;

    // Behavior timers
    FTimerHandle PatrolTimerHandle;
    FTimerHandle StateUpdateTimerHandle;

    void UpdateBehaviorState();
    void SelectNewPatrolPoint();
    void HandlePlayerDetection(AActor* Player);
    void HandlePackCoordination();

private:
    void SetupAIPerception();
    void InitializeBehaviorTree();
    void ConfigureDinosaurFromTags();
};