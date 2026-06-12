#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "SharedTypes.h"
#include "Combat_CombatAIController.generated.h"

class APawn;
class UBehaviorTree;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_CombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_CombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Possess(APawn* InPawn) override;

    // Behavior Tree Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    // AI Perception Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Sight Configuration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISightConfig* SightConfig;

    // Hearing Configuration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIHearingConfig* HearingConfig;

    // Combat Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* CombatBehaviorTree;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombat_AIState CurrentCombatState;

    // Combat Target
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* CombatTarget;

    // Combat Range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 300.0f;

    // Detection Range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float DetectionRange = 1500.0f;

    // Aggression Level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AggressionLevel = 0.5f;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsPackHunter = false;

    // Territorial Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsTerritorial = true;

    // Territory Center
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector TerritoryCenter;

    // Territory Radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TerritoryRadius = 2000.0f;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIState GetCombatState() const { return CurrentCombatState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetCombatTarget() const { return CombatTarget; }

    // Combat Queries
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInCombat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInTerritory() const;

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteDefense();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteRetreat();

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateWithPack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallForBackup();

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTerritory(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void DefendTerritory();

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat Decision Making
    void UpdateCombatDecision();
    void EvaluateThreatLevel();
    void SelectBestTarget();

    // Blackboard Keys
    void InitializeBlackboard();
    void UpdateBlackboardValues();

private:
    // Internal state tracking
    float LastCombatTime;
    float LastPerceptionUpdate;
    TArray<AActor*> NearbyThreats;
    TArray<AActor*> PackMembers;
};