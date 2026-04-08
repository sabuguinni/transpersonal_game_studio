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
    Feeding,
    Territorial,
    Fleeing
};

UENUM(BlueprintType)
enum class EDinosaurAggressionLevel : uint8
{
    Passive,
    Defensive,
    Territorial,
    Aggressive,
    Enraged
};

USTRUCT(BlueprintType)
struct FCombatPersonality
{
    GENERATED_BODY()

    // Base aggression multiplier (0.5 = passive, 2.0 = very aggressive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float AggressionMultiplier = 1.0f;

    // How quickly the dinosaur escalates combat (0.1 = slow, 2.0 = instant)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float EscalationRate = 1.0f;

    // Preferred engagement distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100", ClampMax = "2000"))
    float PreferredCombatRange = 500.0f;

    // How long the dinosaur remembers threats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "5.0", ClampMax = "300.0"))
    float ThreatMemoryDuration = 60.0f;

    // Chance to use special attacks (0.0 = never, 1.0 = always when available)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpecialAttackChance = 0.3f;

    // How much the dinosaur values its own safety (0.0 = fearless, 1.0 = very cautious)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SelfPreservation = 0.5f;
};

USTRUCT(BlueprintType)
struct FCombatMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float CurrentHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    float CurrentStamina = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    float MaxStamina = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float LastDamageTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ConsecutiveHits = 0;

    UPROPERTY(BlueprintReadOnly)
    bool bIsWounded = false;

    UPROPERTY(BlueprintReadOnly)
    bool bIsExhausted = false;
};

/**
 * Advanced Combat AI Controller for Dinosaurs
 * Implements tactical combat behavior with personality-driven decision making
 */
UCLASS(BlueprintType)
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
    class UAIPerceptionComponent* PerceptionComponent;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    class UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatPersonality CombatPersonality;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FCombatMetrics CombatMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EDinosaurCombatState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EDinosaurAggressionLevel CurrentAggressionLevel;

    // Perception Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float FieldOfView = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRadius = 1200.0f;

    // Combat Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float SpecialAttackCooldown = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float StateChangeDelay = 0.5f;

    // Blackboard Keys
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName TargetActorKey = TEXT("TargetActor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName CombatStateKey = TEXT("CombatState");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName AggressionLevelKey = TEXT("AggressionLevel");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName LastKnownLocationKey = TEXT("LastKnownLocation");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName OptimalAttackPositionKey = TEXT("OptimalAttackPosition");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName CanAttackKey = TEXT("CanAttack");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName IsWoundedKey = TEXT("IsWounded");

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(EDinosaurCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetAggressionLevel(EDinosaurAggressionLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnTakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnAttackHit(AActor* HitTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnAttackMissed();

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector CalculateOptimalAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldUseSpecialAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateThreatLevel(AActor* Target);

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal Combat Logic
    void UpdateCombatMetrics(float DeltaTime);
    void EvaluateTacticalSituation();
    void UpdateBlackboardValues();
    void ProcessCombatStateTransitions();
    
    // Timers
    float LastAttackTime = 0.0f;
    float LastSpecialAttackTime = 0.0f;
    float LastStateChangeTime = 0.0f;
    
    // Current Target Tracking
    UPROPERTY()
    AActor* CurrentTarget = nullptr;
    
    FVector LastKnownTargetLocation = FVector::ZeroVector;
    float LastTargetSightTime = 0.0f;
    
    // Combat Analytics
    int32 TotalAttacksMade = 0;
    int32 SuccessfulHits = 0;
    float CombatStartTime = 0.0f;
};