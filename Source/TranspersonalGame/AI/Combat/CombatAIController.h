#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameplayTagContainer.h"
#include "CombatAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class ADinosaurCharacter;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Attacking,
    Fleeing,
    Investigating,
    Territorial,
    Feeding
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct FCombatTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    AActor* Target = nullptr;

    UPROPERTY(BlueprintReadWrite)
    EThreatLevel ThreatLevel = EThreatLevel::None;

    UPROPERTY(BlueprintReadWrite)
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float DistanceToTarget = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bHasLineOfSight = false;

    FCombatTarget()
    {
        Target = nullptr;
        ThreatLevel = EThreatLevel::None;
        LastSeenTime = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        DistanceToTarget = 0.0f;
        bHasLineOfSight = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Behavior Tree and Blackboard
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBlackboardData* BlackboardAsset;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Damage* DamageConfig;

    // Combat State Management
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombatState CurrentCombatState = ECombatState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TArray<FCombatTarget> KnownTargets;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FCombatTarget PrimaryTarget;

    // Combat Parameters
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float OptimalCombatDistance = 500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float FleeDistance = 1500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float TerritoryRadius = 2000.0f;

    // Gameplay Tags for Combat Behaviors
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    FGameplayTagContainer CombatTags;

    // Memory and Decision Making
    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float MemoryDuration = 30.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TMap<AActor*, float> ActorMemory;

public:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddTarget(AActor* Target, EThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RemoveTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombatTarget GetPrimaryTarget() const { return PrimaryTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdatePrimaryTarget();

    // Combat Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldInvestigate() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetOptimalCombatPosition();

    // Blackboard Helpers
    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardValues();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardLocation(const FVector& Location, const FName& KeyName);

protected:
    // Internal Combat Logic
    void ProcessPerceptionUpdate(AActor* Actor, const FAIStimulus& Stimulus);
    void UpdateTargetMemory(float DeltaTime);
    void EvaluateThreatLevel(FCombatTarget& Target);
    bool IsInTerritory(const FVector& Location) const;
    void HandleCombatStateTransition();

    // Utility Functions
    float CalculateTargetPriority(const FCombatTarget& Target) const;
    bool HasLineOfSight(AActor* Target) const;
    FVector GetTerritoryCenter() const;
};