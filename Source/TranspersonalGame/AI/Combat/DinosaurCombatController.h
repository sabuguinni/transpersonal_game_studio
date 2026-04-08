#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatAITypes.h"
#include "DinosaurCombatController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UEnvQueryInstanceBlueprintWrapper;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnThreatDetected, AActor*, ThreatActor, float, ThreatLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChanged, ECombatState, NewState);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* PerceptionComponent;

    // Combat AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombatAIProfile CombatProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    class UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    class UBlackboard* CombatBlackboard;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombatState CurrentCombatState = ECombatState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<FThreatMemory> ThreatMemories;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FThreatMemory PrimaryThreat;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnThreatDetected OnThreatDetected;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatStateChanged OnCombatStateChanged;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    // Threat Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatLocation(AActor* ThreatActor, FVector NewLocation);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    FThreatMemory GetPrimaryThreat() const { return PrimaryThreat; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    TArray<FThreatMemory> GetAllThreats() const { return ThreatMemories; }

    // Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FCombatDecision EvaluateCombatSituation();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFleeFromThreat();

    // EQS Integration
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FindOptimalCombatPosition();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FindAmbushPosition();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FindFleePosition();

    // Personality-Based Behavior
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetPersonalityFactor(const FString& TraitName) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ModifyPersonalityTrait(const FString& TraitName, float Delta);

protected:
    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal Methods
    void UpdateThreatMemories(float DeltaTime);
    void EvaluatePrimaryThreat();
    void UpdateBlackboardValues();
    
    // EQS Callbacks
    UFUNCTION()
    void OnEQSQueryComplete(UEnvQueryInstanceBlueprintWrapper* QueryInstance, int32 ItemIndex);

    // Utility Functions
    float CalculateThreatLevel(AActor* Actor) const;
    bool IsActorInSight(AActor* Actor) const;
    bool IsActorInHearingRange(AActor* Actor) const;
    float GetDistanceToActor(AActor* Actor) const;

private:
    // Internal state tracking
    float LastThreatEvaluationTime = 0.0f;
    float ThreatEvaluationInterval = 0.5f;
    
    // Memory management
    float ThreatMemoryDuration = 30.0f;
    int32 MaxThreatMemories = 5;
};