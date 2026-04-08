#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameplayTags.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class EDinosaurCombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Attacking,
    Retreating,
    Territorial,
    Feeding,
    Investigating
};

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    ApexPredator,      // T-Rex, Giganotosaurus
    PackHunter,        // Velociraptor, Deinonychus
    AmbushPredator,    // Carnotaurus, Baryonyx
    TerritorialHerbivore, // Triceratops, Ankylosaurus
    FlockHerbivore,    // Parasaurolophus, Gallimimus
    SolitaryHerbivore  // Brachiosaurus, Stegosaurus
};

USTRUCT(BlueprintType)
struct FDinosaurCombatProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HuntingRange = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StalkingDuration = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanCallForHelp = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanFlankPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> CombatAbilities;
};

USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadWrite)
    float LastSeenPlayerTime;

    UPROPERTY(BlueprintReadWrite)
    EDinosaurThreatLevel PerceivedThreatLevel;

    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadWrite)
    FVector TerritoryCenter;

    UPROPERTY(BlueprintReadWrite)
    bool bHasBeenAttackedByPlayer;

    UPROPERTY(BlueprintReadWrite)
    float LastDamageTime;
};

/**
 * Advanced AI Controller for Dinosaur Combat
 * Implements tactical behavior, memory system, and adaptive difficulty
 */
UCLASS()
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FDinosaurCombatProfile CombatProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    class UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    class UBlackboard* CombatBlackboard;

    // Memory System
    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FDinosaurMemory DinosaurMemory;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EDinosaurCombatState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastStateChangeTime;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(EDinosaurCombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    EDinosaurCombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, struct FAIStimulus Stimulus);

    // Combat Behavior
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankingPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CallForHelp();

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdatePlayerMemory(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FVector GetLastKnownPlayerLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void SetThreatLevel(EDinosaurThreatLevel ThreatLevel);

    // Adaptive Difficulty
    UFUNCTION(BlueprintCallable, Category = "Difficulty")
    void AdjustDifficultyBasedOnPlayerPerformance();

private:
    void InitializePerception();
    void UpdateBlackboardValues();
    void ProcessPerceptionData();
    
    float CalculateThreatLevel(AActor* Target) const;
    bool IsPlayerInTerritory() const;
    void UpdateCombatState();
    
    // Tactical AI
    FVector CalculateOptimalPosition() const;
    bool ShouldUseSpecialAbility() const;
    void PlanNextMove();

    // Performance tracking for adaptive difficulty
    int32 PlayerDeathCount = 0;
    int32 PlayerEscapeCount = 0;
    float AverageEngagementTime = 0.0f;
    float LastEngagementStartTime = 0.0f;
};