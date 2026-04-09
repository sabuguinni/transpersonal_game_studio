#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameplayTags.h"
#include "CombatAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class UCombatTacticsComponent;
class UGroupCombatComponent;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Neutral,
    Alert,
    Stalking,
    Engaging,
    Attacking,
    Defending,
    Retreating,
    Flanking,
    Ambushing,
    Regrouping
};

UENUM(BlueprintType)
enum class ECombatRole : uint8
{
    Aggressor,
    Defender,
    Flanker,
    Support,
    Scout,
    Alpha,
    Beta,
    Omega
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EThreatLevel ThreatLevel = EThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DistanceToTarget = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bHasLineOfSight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DamageDealt = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DamageReceived = 0.0f;

    FCombatTarget()
    {
        TargetActor = nullptr;
        ThreatLevel = EThreatLevel::None;
        LastSeenTime = 0.0f;
        LastKnownPosition = FVector::ZeroVector;
        DistanceToTarget = 0.0f;
        bHasLineOfSight = false;
        DamageDealt = 0.0f;
        DamageReceived = 0.0f;
    }
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
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Armor = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CriticalHitChance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DodgeChance = 0.2f;

    FCombatStats()
    {
        AttackDamage = 50.0f;
        AttackRange = 200.0f;
        AttackCooldown = 2.0f;
        MovementSpeed = 600.0f;
        Health = 100.0f;
        MaxHealth = 100.0f;
        Armor = 10.0f;
        CriticalHitChance = 0.1f;
        DodgeChance = 0.2f;
    }
};

/**
 * Advanced Combat AI Controller that implements tactical combat behavior
 * for dinosaur enemies with sophisticated threat assessment, group coordination,
 * and adaptive combat strategies
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
    class UCombatTacticsComponent* TacticsComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UGroupCombatComponent* GroupComponent;

    // Behavior Tree Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardData* CombatBlackboard;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombatRole CombatRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TacticalIntelligence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float GroupCoordination = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AdaptabilityLevel = 0.4f;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombatState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FCombatTarget PrimaryTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<FCombatTarget> KnownTargets;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FVector CombatPosition;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float LastAttackTime;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float CombatTimer;

    // Tactical Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float OptimalCombatRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float FlankingDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float RegroupDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanCallForHelp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanFlank = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanAmbush = true;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AcquireTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void LoseTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTargetInfo(AActor* Target);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetPrimaryTarget() const { return PrimaryTarget.TargetActor; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    EThreatLevel AssessThreatLevel(AActor* Target) const;

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteDefensiveAction();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteRetreat();

    // Tactical Positioning
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateOptimalPosition();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateAmbushPosition();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsPositionSafe(FVector Position) const;

    // Group Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallForHelp();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RespondToHelpCall(ACombatAIController* Caller);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateGroupAttack();

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    TArray<ACombatAIController*> GetNearbyAllies(float Radius = 1000.0f) const;

    // Adaptive Behavior
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AdaptToPlayerBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void LearnFromCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTactics();

    // Perception Events
    UFUNCTION()
    void OnCombatPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnDamageReceived(float Damage, AActor* DamageSource);

    // Debugging
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawCombatInfo() const;

protected:
    // Internal combat logic
    void InitializeCombatPerception();
    void InitializeCombatBehaviorTree();
    void UpdateCombatBlackboard();
    void ProcessCombatDecisions();
    void UpdateCombatState(float DeltaTime);
    void AnalyzeCombatSituation();
    
    // Combat calculations
    float CalculateDamageOutput() const;
    float CalculateHitChance(AActor* Target) const;
    bool ShouldRetreat() const;
    bool ShouldCallForHelp() const;
    bool CanExecuteFlankingManeuver() const;

    // Timers and cooldowns
    float StateChangeTimer = 0.0f;
    float TacticsUpdateTimer = 0.0f;
    float PerceptionUpdateTimer = 0.0f;
    float AdaptationTimer = 0.0f;

    // Combat memory
    TMap<AActor*, float> DamageHistory;
    TMap<AActor*, int32> EncounterCount;
    TArray<FVector> SuccessfulPositions;
    TArray<FVector> FailedPositions;

    // Cached references
    class ADinosaurCharacter* CombatPawn;
    class UWorld* CachedWorld;
    class APlayerController* PlayerController;
};