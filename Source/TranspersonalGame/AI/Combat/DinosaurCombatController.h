#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "CombatAITypes.h"
#include "DinosaurCombatController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;
class ADinosaurPawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, ECombatState, OldState, ECombatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatDetected, AActor*, ThreatActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatLost, AActor*, ThreatActor);

UCLASS()
class TRANSPERSONALGAME_API ADinosaurCombatController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Tree Asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    // Combat Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FDinosaurCombatData CombatData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FDinosaurPersonality Personality;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FCombatMemory Memory;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CurrentCombatState = ECombatState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> DetectedThreats;

    // Perception Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    UAISenseConfig_Damage* DamageConfig;

public:
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnThreatDetected OnThreatDetected;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnThreatLost OnThreatLost;

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCurrentTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateThreatLevel(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* FindBestTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldCallForHelp();

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(EAttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartFleeing();

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberThreat(AActor* Actor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberDangerousLocation(FVector Location, float DangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberSafeLocation(FVector Location);

    UFUNCTION(BlueprintPure, Category = "Memory")
    float GetRememberedThreatLevel(AActor* Actor);

    // Personality-based decisions
    UFUNCTION(BlueprintCallable, Category = "Personality")
    bool MakePersonalityBasedDecision(const FString& DecisionType);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    float GetPersonalityModifier(const FString& TraitName);

    // Blackboard Helpers
    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardCombatData();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardValue(const FString& KeyName, const FString& Value);

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal State Management
    void InitializePerception();
    void InitializeBehaviorTree();
    void UpdateCombatLogic(float DeltaTime);
    void UpdateMemory(float DeltaTime);
    void ProcessThreatAssessment();

    // Combat Decision Making
    bool ShouldEngageTarget(AActor* Target);
    bool ShouldSwitchTargets(AActor* NewTarget);
    EAttackType ChooseBestAttack();
    FVector FindBestAttackPosition();
    FVector FindBestFleeDirection();

    // Utility Functions
    float CalculateDistanceToActor(AActor* Actor);
    bool IsInAttackRange(AActor* Target);
    bool HasLineOfSight(AActor* Target);
    bool IsPlayerActor(AActor* Actor);

private:
    // Internal timers
    float LastPerceptionUpdate = 0.0f;
    float LastMemoryUpdate = 0.0f;
    float LastThreatAssessment = 0.0f;
    float LastAttackTime = 0.0f;

    // Combat state timers
    float StateChangeTime = 0.0f;
    float MinStateTime = 1.0f; // Minimum time to stay in a state

    // Reference to controlled dinosaur
    UPROPERTY()
    ADinosaurPawn* ControlledDinosaur = nullptr;
};