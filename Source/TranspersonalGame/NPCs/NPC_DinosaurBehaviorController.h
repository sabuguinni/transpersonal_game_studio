#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "NPC_DinosaurBehaviorController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_DinosaurBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_DinosaurBehaviorController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Behavior Tree Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StartBehaviorTree(class UBehaviorTree* BehaviorTree);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StopBehaviorTree();

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Blackboard Keys
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName TargetActorKey = TEXT("TargetActor");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName PatrolLocationKey = TEXT("PatrolLocation");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName HomeLocationKey = TEXT("HomeLocation");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName CurrentStateKey = TEXT("CurrentState");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName AlertLevelKey = TEXT("AlertLevel");

protected:
    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Perception Configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Perception")
    float SightRadius = 3000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Perception")
    float LoseSightRadius = 3500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Perception")
    float PeripheralVisionAngleDegrees = 90.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Perception")
    float HearingRange = 2000.0f;

    // Behavior Settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    float PatrolRadius = 1500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    float ChaseRange = 3000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    float AttackRange = 200.0f;

    // State Management
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENPC_DinosaurState CurrentDinosaurState = ENPC_DinosaurState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float AlertLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector CurrentPatrolLocation = FVector::ZeroVector;

private:
    // Internal state tracking
    float LastTargetSeenTime = 0.0f;
    float StateChangeTime = 0.0f;
    bool bHasValidTarget = false;

    // Helper functions
    void UpdateBlackboardValues();
    void SetDinosaurState(ENPC_DinosaurState NewState);
    bool IsPlayerInRange(float Range) const;
    FVector GetRandomPatrolLocation() const;
    void HandleTargetLost();
    void HandleTargetFound(AActor* Target);
};