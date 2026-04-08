#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "CombatAIController.generated.h"

class UBehaviorTree;
class UBlackboardAsset;
class ACombatDinosaur;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Hunting,
    Attacking,
    Fleeing,
    Feeding,
    Resting,
    Territorial
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
struct FCombatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    AActor* LastKnownTarget = nullptr;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    EThreatLevel ThreatLevel = EThreatLevel::None;

    UPROPERTY(BlueprintReadWrite)
    bool bHasBeenAttacked = false;

    UPROPERTY(BlueprintReadWrite)
    float AggressionLevel = 0.0f;
};

/**
 * Advanced AI Controller for dinosaur combat behavior
 * Implements tactical AI with memory, threat assessment, and dynamic behavior switching
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    // AI Assets
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBlackboardAsset* BlackboardAsset;

    // Combat State
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombatState CurrentCombatState = ECombatState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FCombatMemory CombatMemory;

    // Tactical Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float OptimalAttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float FleeHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float AggressionDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float TerritorialRadius = 1500.0f;

    // Gameplay Tags for behavior identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tags")
    FGameplayTagContainer DinosaurTypeTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tags")
    FGameplayTagContainer BehaviorTags;

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

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat")
    EThreatLevel AssessThreat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatLevel(AActor* Target, EThreatLevel NewThreat);

    // Tactical Decisions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector FindOptimalAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector FindFleePosition(AActor* Threat);

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RememberTarget(AActor* Target, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ForgetTarget(AActor* Target);

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool HasMemoryOf(AActor* Target) const;

    // Blackboard Helpers
    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardCombatData();

private:
    // Internal state tracking
    TMap<AActor*, FCombatMemory> TargetMemories;
    float LastCombatStateChange = 0.0f;
    float StateChangeDelay = 1.0f; // Prevent rapid state switching

    // Tactical analysis
    void AnalyzeTacticalSituation();
    void UpdateAggressionLevel(float DeltaTime);
    bool IsInTerritory(const FVector& Location) const;
    
    // Perception configuration
    void ConfigurePerception();
    void SetupSightSense();
    void SetupHearingSense();
};