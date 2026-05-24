#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "CombatAIManager.h"
#include "EnemyAIController.generated.h"

class UBehaviorTree;
class APawn;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_AIState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    ECombat_AIBehavior CurrentBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    APawn* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    FVector LastKnownTargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    float AlertLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    float TimeSinceLastTargetSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    bool bIsInCombat;

    FCombat_AIState()
    {
        CurrentBehavior = ECombat_AIBehavior::Patrol;
        CurrentTarget = nullptr;
        LastKnownTargetLocation = FVector::ZeroVector;
        AlertLevel = 0.0f;
        TimeSinceLastTargetSeen = 0.0f;
        bIsInCombat = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Possess(APawn* InPawn) override;
    virtual void UnPossess() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    FCombat_AIState AIState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Combat")
    ACombatAIManager* CombatManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Combat")
    FCombat_TacticalPosition AssignedTacticalPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Combat")
    float FleeHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Movement")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Movement")
    float CombatMovementSpeed;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "AI Combat")
    void SetCombatManager(ACombatAIManager* NewCombatManager);

    UFUNCTION(BlueprintCallable, Category = "AI Combat")
    void SetTacticalPosition(const FCombat_TacticalPosition& NewPosition);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetAIBehavior(ECombat_AIBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    ECombat_AIBehavior GetCurrentBehavior() const;

    UFUNCTION(BlueprintCallable, Category = "AI Combat")
    void EngageTarget(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "AI Combat")
    void DisengageTarget();

    UFUNCTION(BlueprintCallable, Category = "AI Combat")
    bool CanAttackTarget() const;

    UFUNCTION(BlueprintCallable, Category = "AI Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "AI Movement")
    void MoveToTacticalPosition();

    UFUNCTION(BlueprintCallable, Category = "AI Perception")
    bool HasLineOfSightToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "AI State")
    void UpdateAlertLevel(float DeltaAlert);

protected:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void UpdateBehaviorState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI Combat")
    void UpdateCombatState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI Movement")
    void UpdateMovement(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI Perception")
    void ProcessSightStimulus(AActor* Actor, const FAIStimulus& Stimulus);

    UFUNCTION(BlueprintCallable, Category = "AI Perception")
    void ProcessHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);

    UFUNCTION(BlueprintCallable, Category = "AI Combat")
    float CalculateAttackDamage() const;

    UFUNCTION(BlueprintCallable, Category = "AI Combat")
    bool ShouldFleeFromCombat() const;

    void SetupPerception();
    void SetupBlackboard();
    void FindCombatManager();
};