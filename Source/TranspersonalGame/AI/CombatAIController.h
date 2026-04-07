#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "ConsciousnessAIController.h"
#include "CombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Engaging,
    Retreating,
    Flanking,
    Defending,
    Stunned
};

UENUM(BlueprintType)
enum class ECombatPersonality : uint8
{
    Aggressive,     // High consciousness = more tactical
    Defensive,      // High consciousness = better positioning
    Balanced,       // High consciousness = adaptive strategy
    Berserker,      // Low consciousness = pure aggression
    Coward          // Any consciousness = flee when possible
};

USTRUCT(BlueprintType)
struct FCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlankingRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TacticalIntelligence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroupCoordination = 0.5f;
};

UCLASS()
class TRANSPERSONALGAME_API ACombatAIController : public AConsciousnessAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Combat AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombatPersonality CombatPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    class UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FVector> PatrolPoints;

    // Current Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector LastKnownTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float TimeSinceLastTargetSeen;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> AlliedUnits;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // Consciousness-Enhanced Combat Decisions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldEngageTarget(AActor* PotentialTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldCallForHelp();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetOptimalAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankingPosition(AActor* Target);

    // Group Combat Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterAlliedUnit(AActor* Ally);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnregisterAlliedUnit(AActor* Ally);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<AActor*> GetNearbyAllies(float Range = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinateGroupAttack(AActor* Target);

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal Combat Logic
    void UpdateCombatBehavior(float DeltaTime);
    void UpdateConsciousnessModifiers();
    void ProcessPerceptionData();
    
    // Consciousness-Enhanced Decisions
    float CalculateEngagementPriority(AActor* Target);
    float CalculateThreatLevel(AActor* Target);
    bool CanCoordinateWithAllies();
    
    // Combat Utilities
    bool IsInAttackRange(AActor* Target);
    bool HasLineOfSight(AActor* Target);
    FVector PredictTargetMovement(AActor* Target, float PredictionTime = 1.0f);
    
    // Blackboard Keys
    static const FName TargetActorKey;
    static const FName CombatStateKey;
    static const FName LastKnownLocationKey;
    static const FName ShouldFlankKey;
    static const FName ShouldRetreatKey;
    static const FName AlliedUnitsKey;
    static const FName ConsciousnessLevelKey;
};