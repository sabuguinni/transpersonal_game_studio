#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "NPC_DinosaurAIController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_DinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_DinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIHearingConfig* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBlackboardComponent* BlackboardComponent;

    // AI Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float ChaseDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float AttackDistance;

    // Dinosaur Behavior State
    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    ENPC_DinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    FVector PatrolDestination;

public:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Behavior Control
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetPatrolDestination(FVector NewDestination);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ReturnToHome();

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "AI Utility")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "AI Utility")
    float GetDistanceToHome() const;

    UFUNCTION(BlueprintPure, Category = "AI Utility")
    bool IsPlayerInSight() const;

    UFUNCTION(BlueprintPure, Category = "AI Utility")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "AI Utility")
    FVector GetRandomPatrolPoint() const;

protected:
    // Internal Functions
    void InitializePerception();
    void InitializeBehaviorTree();
    void UpdateBlackboardValues();
    bool CanSeeActor(AActor* Actor) const;
    void HandlePlayerDetection(AActor* Player);
    void HandlePlayerLost();
};