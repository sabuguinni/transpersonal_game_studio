#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatAITypes.h"
#include "DinosaurCombatAI.generated.h"

class UBehaviorTree;
class UBlackboardAsset;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Perception System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    UAISenseConfig_Damage* DamageConfig;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Behavior")
    UBlackboardAsset* BlackboardAsset;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
    EDinosaurArchetype DinosaurArchetype = EDinosaurArchetype::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
    FCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
    FDinosaurVariation DinosaurVariation;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    ECombatState CurrentCombatState = ECombatState::Passive;

    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    EThreatLevel CurrentThreatLevel = EThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    AActor* LastKnownTargetLocation = nullptr;

    // Pack Behavior
    UPROPERTY(BlueprintReadOnly, Category = "AI|Pack")
    TArray<ADinosaurCombatAI*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "AI|Pack")
    ADinosaurCombatAI* PackLeader = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "AI|Pack")
    bool bIsPackLeader = false;

    // Territory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Territory")
    float TerritoryRadius = 1000.0f;

    // Timers and Cooldowns
    UPROPERTY(BlueprintReadOnly, Category = "AI|Timing")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "AI|Timing")
    float LastTargetSeenTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "AI|Timing")
    float StateChangeTime = 0.0f;

public:
    // Events
    UPROPERTY(BlueprintAssignable, Category = "AI|Events")
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "AI|Events")
    FOnThreatLevelChanged OnThreatLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "AI|Events")
    FOnTargetAcquired OnTargetAcquired;

    UPROPERTY(BlueprintAssignable, Category = "AI|Events")
    FOnTargetLost OnTargetLost;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void SetThreatLevel(EThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void ClearTarget();

    UFUNCTION(BlueprintPure, Category = "AI|Combat")
    bool IsInCombat() const;

    UFUNCTION(BlueprintPure, Category = "AI|Combat")
    bool CanSeeTarget() const;

    UFUNCTION(BlueprintPure, Category = "AI|Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "AI|Combat")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "AI|Combat")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void InitializeFromArchetype(const FDinosaurArchetypeData& ArchetypeData);

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "AI|Pack")
    void JoinPack(ADinosaurCombatAI* Leader);

    UFUNCTION(BlueprintCallable, Category = "AI|Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "AI|Pack")
    void BecomePackLeader();

    UFUNCTION(BlueprintCallable, Category = "AI|Pack")
    void AddPackMember(ADinosaurCombatAI* Member);

    UFUNCTION(BlueprintCallable, Category = "AI|Pack")
    void RemovePackMember(ADinosaurCombatAI* Member);

    UFUNCTION(BlueprintPure, Category = "AI|Pack")
    bool IsInPack() const { return PackLeader != nullptr || bIsPackLeader; }

    // Strategy Selection
    UFUNCTION(BlueprintCallable, Category = "AI|Strategy")
    EHuntingStrategy SelectBestStrategy();

    UFUNCTION(BlueprintCallable, Category = "AI|Strategy")
    void ExecuteStrategy(EHuntingStrategy Strategy);

protected:
    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal Methods
    void UpdateCombatLogic(float DeltaTime);
    void UpdateThreatAssessment();
    void UpdatePackBehavior();
    void UpdateTerritorialBehavior();
    
    bool ShouldEngageTarget(AActor* Target) const;
    bool ShouldFleeFromTarget(AActor* Target) const;
    EThreatLevel CalculateThreatLevel(AActor* Target) const;
    
    void InitializePerception();
    void InitializeBehaviorTree();
    void SetupBlackboardKeys();

    // Strategy Implementations
    void ExecuteDirectAssault();
    void ExecuteAmbushStrategy();
    void ExecutePackHuntStrategy();
    void ExecuteStalkingStrategy();
    void ExecuteTerritorialDefense();

    // Utility Functions
    FVector GetRandomPointInTerritory() const;
    FVector GetFlankingPosition(AActor* Target) const;
    FVector GetAmbushPosition(AActor* Target) const;
    bool HasClearLineOfSight(AActor* Target) const;
};