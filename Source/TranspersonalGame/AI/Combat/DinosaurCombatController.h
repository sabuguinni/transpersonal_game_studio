#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatAITypes.h"
#include "DinosaurCombatController.generated.h"

class UBehaviorTree;
class UBlackboardAsset;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, ECombatState, OldState, ECombatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDetected, AActor*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLost, AActor*, Player);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    UAISenseConfig_Damage* DamageConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    UBlackboardAsset* DefaultBlackboard;

    // Combat State Management
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombatState CurrentCombatState = ECombatState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* LastKnownPlayerLocation = nullptr;

    // Archetype Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FDinosaurArchetype DinosaurArchetype;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Config")
    FDinosaurVariation DinosaurVariation;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float AlertnessLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float CurrentStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float LastPlayerSightingTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float InvestigationTimeout = 10.0f;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnPlayerDetected OnPlayerDetected;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnPlayerLost OnPlayerLost;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat State")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat State")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // Archetype Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Config")
    void InitializeFromArchetype(const FDinosaurArchetype& Archetype);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Config")
    void GenerateRandomVariation();

    UFUNCTION(BlueprintPure, Category = "Dinosaur Config")
    FDinosaurVariation GetDinosaurVariation() const { return DinosaurVariation; }

    // Combat Queries
    UFUNCTION(BlueprintPure, Category = "Combat Queries")
    bool IsPlayerInSight() const;

    UFUNCTION(BlueprintPure, Category = "Combat Queries")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "Combat Queries")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Combat Queries")
    bool CanAttackTarget() const;

    UFUNCTION(BlueprintPure, Category = "Combat Queries")
    bool ShouldFlee() const;

    // Stamina Management
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void RegenerateStamina(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Stamina")
    float GetStaminaPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Stamina")
    bool HasEnoughStamina(float RequiredAmount) const;

    // Tactical AI
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector FindOptimalAttackPosition();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector FindFleePosition();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateAlertness(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ReactToPlayerAction(const FString& ActionType, float Intensity);

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal State Management
    void UpdateCombatLogic(float DeltaTime);
    void HandleStateTransitions();
    void UpdateBlackboardValues();

    // Utility Functions
    bool IsPlayerCharacter(AActor* Actor) const;
    float CalculateThreatLevel(AActor* ThreatActor) const;
    void PlayCombatAudio(const FString& AudioCue);
};