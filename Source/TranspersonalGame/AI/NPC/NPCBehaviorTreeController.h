#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "NPCBehaviorTreeController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPCBehaviorTreeController : public AAIController
{
    GENERATED_BODY()

public:
    ANPCBehaviorTreeController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // NPC Behavior States
    UPROPERTY(BlueprintReadWrite, Category = "NPC Behavior")
    bool bIsIdle;

    UPROPERTY(BlueprintReadWrite, Category = "NPC Behavior")
    bool bIsPatrolling;

    UPROPERTY(BlueprintReadWrite, Category = "NPC Behavior")
    bool bIsHunting;

    UPROPERTY(BlueprintReadWrite, Category = "NPC Behavior")
    bool bIsFleeing;

    UPROPERTY(BlueprintReadWrite, Category = "NPC Behavior")
    bool bIsGathering;

    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(const FString& StateName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartDailyRoutine();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RespondToThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetPatrolRoute(const TArray<FVector>& PatrolPoints);

private:
    TArray<FVector> PatrolPoints;
    int32 CurrentPatrolIndex;
    float LastRoutineUpdate;
};