#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Combat_DinosaurCombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBehaviorTree;

UENUM(BlueprintType)
enum class ECombat_DinosaurCombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurCombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISenseConfig_Sight* SightConfig;

    // Blackboard and Behavior Tree
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    // Combat State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    ECombat_DinosaurCombatState CombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    // Player tracking
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector LastPlayerLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float TimeSinceLastPlayerSeen;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsPlayerInSight;

public:
    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat behavior
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinosaurCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool DetectPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetAggressionLevel(float NewAggression);

protected:
    // State handlers
    void HandleIdleState(float DeltaTime);
    void HandleHuntingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);
    void HandleRetreatingState(float DeltaTime);

    // Combat actions
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void PerformAttack();

    void PerformAttack();
};