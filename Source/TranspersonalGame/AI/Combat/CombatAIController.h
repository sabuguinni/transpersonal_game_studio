// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameplayTags.h"
#include "CombatAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class UCombatMemoryComponent;
class UThreatAssessmentComponent;
class UCombatFormationComponent;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Patrol,
    Investigate,
    Engage,
    Flank,
    Retreat,
    Regroup,
    Ambush,
    Pursue,
    Defend,
    Hunt,
    Stalk,
    Pounce
};

UENUM(BlueprintType)
enum class ECombatRole : uint8
{
    Aggressor,      // Direct attacker
    Flanker,        // Side/rear attacker
    Defender,       // Protects territory/pack
    Scout,          // Information gatherer
    Ambusher,       // Stealth attacker
    Supporter,      // Assists others
    Alpha,          // Pack leader
    Lone            // Solo hunter
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
struct FCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Armor = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Aggression = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Courage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Intelligence = 0.6f;

    FCombatStats()
    {
        AttackDamage = 50.0f;
        AttackRange = 200.0f;
        AttackSpeed = 1.0f;
        MovementSpeed = 600.0f;
        Health = 100.0f;
        Armor = 0.0f;
        Stamina = 100.0f;
        Aggression = 0.7f;
        Courage = 0.5f;
        Intelligence = 0.6f;
    }
};

USTRUCT(BlueprintType)
struct FCombatTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    EThreatLevel ThreatLevel = EThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FVector LastKnownPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float DistanceToTarget = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bHasLineOfSight = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float DamageDealt = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float DamageReceived = 0.0f;

    FCombatTarget()
    {
        TargetActor = nullptr;
        ThreatLevel = EThreatLevel::None;
        LastKnownPosition = FVector::ZeroVector;
        LastSeenTime = 0.0f;
        DistanceToTarget = 0.0f;
        bHasLineOfSight = false;
        DamageDealt = 0.0f;
        DamageReceived = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCombatFormation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    TArray<TWeakObjectPtr<AActor>> FormationMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    TWeakObjectPtr<AActor> FormationLeader;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    FVector FormationCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    float FormationRadius = 500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    FGameplayTag FormationType;

    FCombatFormation()
    {
        FormationLeader = nullptr;
        FormationCenter = FVector::ZeroVector;
        FormationRadius = 500.0f;
    }
};

/**
 * Advanced Combat AI Controller for tactical combat behaviors
 * Handles threat assessment, combat formations, flanking maneuvers, and adaptive tactics
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

    // Core Combat Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UCombatMemoryComponent* CombatMemory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UThreatAssessmentComponent* ThreatAssessment;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UCombatFormationComponent* FormationComponent;

    // Behavior Tree Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardData* CombatBlackboard;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombatRole CombatRole = ECombatRole::Aggressor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float EngagementRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RegroupDistance = 1500.0f;

    // Tactical Behavior Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanFlank = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanAmbush = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanRetreat = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanCallForHelp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float TacticalThinkingInterval = 2.0f;

    // Current Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombatState CurrentCombatState = ECombatState::Patrol;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FCombatTarget PrimaryTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<FCombatTarget> SecondaryTargets;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FCombatFormation CurrentFormation;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float CombatTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FVector LastKnownPlayerPosition = FVector::ZeroVector;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPrimaryTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddSecondaryTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveTarget(AActor* Target);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetBestTarget() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool HasValidTarget() const;

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EvaluateTacticalOptions();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFlank() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldAmbush() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetAmbushPosition(AActor* Target) const;

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallForReinforcements();

    // Formation Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void JoinFormation(ACombatAIController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void LeaveFormation();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateFormationPosition();

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsInFormation() const;

    // Perception Events
    UFUNCTION()
    void OnCombatPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnDamageReceived(AActor* DamageSource, float DamageAmount);

    UFUNCTION()
    void OnAllyDamaged(AActor* Ally, AActor* Attacker, float DamageAmount);

    // Memory and Learning
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RecordCombatExperience(AActor* Opponent, bool bVictorious);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetExperienceAgainst(AActor* Opponent) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AdaptTactics(AActor* Opponent);

    // Debugging
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawCombatInfo() const;

protected:
    // Internal Combat Logic
    void InitializeCombatPerception();
    void InitializeCombatBehaviorTree();
    void UpdateCombatBlackboard();
    void ProcessTacticalThinking();
    void UpdateTargetTracking(float DeltaTime);
    void EvaluateThreats();
    void UpdateCombatStats(float DeltaTime);

    // Tactical Calculations
    float CalculateThreatLevel(AActor* Target) const;
    float CalculateFlankingScore(FVector Position, AActor* Target) const;
    float CalculateAmbushScore(FVector Position, AActor* Target) const;
    bool HasLineOfSight(FVector FromPosition, AActor* Target) const;
    bool IsPositionSafe(FVector Position) const;

    // Combat Utilities
    void BroadcastCombatAlert(AActor* Threat);
    void RequestFormationSupport();
    void CoordinateGroupAttack();

    // Timers and Intervals
    float TacticalUpdateTimer = 0.0f;
    float TargetUpdateTimer = 0.0f;
    float StaminaRegenTimer = 0.0f;
    float CombatExperienceTimer = 0.0f;

    // Cached References
    class APawn* CombatPawn;
    class UWorld* CachedWorld;
    TArray<ACombatAIController*> NearbyAllies;
    TArray<AActor*> KnownThreats;

    // Combat Learning Data
    TMap<AActor*, float> OpponentExperience;
    TMap<FGameplayTag, float> TacticSuccessRates;
    int32 CombatVictories = 0;
    int32 CombatDefeats = 0;
};