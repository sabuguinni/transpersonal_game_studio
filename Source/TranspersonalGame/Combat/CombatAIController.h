#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombatAIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombatAIType : uint8
{
    Predator    UMETA(DisplayName = "Predator"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Scavenger   UMETA(DisplayName = "Scavenger"),
    Territorial UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct FCombatAIStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float FearThreshold = 0.3f;
};

/**
 * Advanced AI Controller for combat entities in the Transpersonal Game
 * Handles tactical behavior, pack coordination, and dynamic combat responses
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

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Perception")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Perception")
    class UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    class UBlackboardData* BlackboardAsset;

    // Combat AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombatAIType AIType = ECombatAIType::Predator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombatAIStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bCanFormPacks = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    int32 MaxPackSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PackCoordinationRange = 1000.0f;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI State")
    ECombatAIState CurrentState = ECombatAIState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI State")
    TArray<ACombatAIController*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI State")
    bool bIsPackLeader = false;

    // Timers and Cooldowns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float AttackCooldown = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Timing")
    float LastAttackTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float StateChangeDelay = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Timing")
    float LastStateChangeTime = 0.0f;

public:
    // AI Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatAIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(ACombatAIController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void BecomePackLeader();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void FindNearbyAllies();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackAttack(AActor* Target);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateBlackboardValues();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnCombatStateChanged(ECombatAIState OldState, ECombatAIState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnTargetAcquired(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnTargetLost();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnAttackPerformed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnDamageTaken(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnPackJoined(ACombatAIController* PackLeader);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnPackLeft();

private:
    // Internal state management
    void InitializeAIPerception();
    void InitializeBehaviorTree();
    void UpdateCombatLogic(float DeltaTime);
    void HandleStateTransitions();
    void UpdatePackBehavior();
    
    // Combat calculations
    float CalculateThreatLevel(AActor* PotentialThreat) const;
    FVector CalculateOptimalPosition(AActor* Target) const;
    bool ShouldEngageTarget(AActor* PotentialTarget) const;
    
    // Pack coordination
    void SendPackCommand(const FString& Command, AActor* Target = nullptr);
    void ReceivePackCommand(const FString& Command, AActor* Target = nullptr);
    
    FTimerHandle StateUpdateTimer;
    FTimerHandle PackUpdateTimer;
    FTimerHandle PerceptionUpdateTimer;
};