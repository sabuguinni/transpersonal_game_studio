#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "TRexBehaviorTree.generated.h"

/**
 * T-Rex AI Controller — drives patrol, chase, and attack behavior
 * Patrol radius: 5000 units | Chase range: 3000 units | Attack range: 300 units
 */
UCLASS(ClassGroup = "DinosaurAI", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ATRexAIController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Behavior tree asset reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|BehaviorTree")
    UBehaviorTree* TRexBehaviorTreeAsset;

    // Blackboard keys
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Blackboard")
    FName BlackboardKey_TargetActor = FName("TargetActor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Blackboard")
    FName BlackboardKey_PatrolOrigin = FName("PatrolOrigin");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Blackboard")
    FName BlackboardKey_PatrolRadius = FName("PatrolRadius");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Blackboard")
    FName BlackboardKey_AIState = FName("AIState");

    // Perception ranges
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float SightRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float SightLoseSightRadius = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    float PatrolRadius = 5000.0f;

    // State machine
    UFUNCTION(BlueprintCallable, Category = "AI|State")
    void SetAIState(int32 NewState);

    UFUNCTION(BlueprintCallable, Category = "AI|State")
    int32 GetAIState() const { return CurrentAIState; }

    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    bool IsPlayerInChaseRange() const;

protected:
    UPROPERTY()
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY()
    UBlackboardComponent* BlackboardComp;

    int32 CurrentAIState; // 0=Patrol, 1=Alert, 2=Chase, 3=Attack

    FVector PatrolOrigin;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    void UpdateBlackboardFromPerception(AActor* PlayerActor);
};
