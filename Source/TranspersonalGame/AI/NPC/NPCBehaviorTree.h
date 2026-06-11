#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/Core/SharedTypes.h"
#include "NPCBehaviorTree.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_NPCState CurrentState = ENPC_NPCState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AlertLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    int32 CurrentPatrolIndex = 0;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_BehaviorTreeManager : public UObject
{
    GENERATED_BODY()

public:
    UNPC_BehaviorTreeManager();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeBehaviorTree(class ANPC_BaseCharacter* NPCOwner);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateBehaviorState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_NPCState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_NPCState GetCurrentState() const { return BehaviorState.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddPatrolPoint(const FVector& Point);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetTargetActor(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void OnPlayerDetected(AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void OnPlayerLost();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void OnCombatStart(AActor* Enemy);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void OnCombatEnd();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior")
    FNPC_BehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior")
    class ANPC_BaseCharacter* OwnerNPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float PatrolSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float ChaseSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float LostPlayerTime = 5.0f;

private:
    void ProcessIdleState(float DeltaTime);
    void ProcessPatrolState(float DeltaTime);
    void ProcessChaseState(float DeltaTime);
    void ProcessCombatState(float DeltaTime);
    void ProcessFleeState(float DeltaTime);

    FVector GetNextPatrolPoint();
    bool IsPlayerInRange() const;
    void UpdateAlertLevel(float DeltaTime);
};