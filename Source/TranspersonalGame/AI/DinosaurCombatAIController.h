// DinosaurCombatAIController.h
// Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260622_003
// Tactical combat AI controller for dinosaur enemies.
// Reads UNPCBehaviorComponent state and drives Behavior Tree.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DinosaurCombatAIController.generated.h"

// Combat phase enum — global scope per UBT rules, Combat_ prefix for uniqueness
UENUM(BlueprintType)
enum class ECombat_Phase : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Recovering  UMETA(DisplayName = "Recovering")
};

// Dinosaur species type — affects combat parameters
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

// Combat stats struct — per-species tuning
USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggroRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChargeSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackCallRadius = 3000;
};

// Blackboard key names (string constants for BT integration)
namespace Combat_BBKeys
{
    static const FName TargetActor     = TEXT("TargetActor");
    static const FName TargetLocation  = TEXT("TargetLocation");
    static const FName CombatPhase     = TEXT("CombatPhase");
    static const FName bCanAttack      = TEXT("bCanAttack");
    static const FName bIsFleeing      = TEXT("bIsFleeing");
    static const FName bTargetVisible  = TEXT("bTargetVisible");
    static const FName PatrolIndex     = TEXT("PatrolIndex");
    static const FName HomeLocation    = TEXT("HomeLocation");
}

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // --- Species Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FCombat_DinoStats CombatStats;

    // --- Behavior Tree ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|AI")
    UBehaviorTree* BehaviorTree;

    // --- Combat State ---
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_Phase CurrentPhase = ECombat_Phase::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float CurrentHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float AttackCooldownRemaining = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bPackCallSent = false;

    // --- Target ---
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Target")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Target")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Target")
    bool bTargetVisible = false;

    // --- Home / Patrol ---
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Patrol")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Patrol")
    int32 CurrentPatrolIndex = 0;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatPhase(ECombat_Phase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeCombatDamage(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInAggroRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void BroadcastPackCall();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AdvancePatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombat_DinoStats GetDefaultStatsForSpecies(ECombat_DinoSpecies InSpecies) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

protected:
    // Perception component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception", meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent_Combat;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception", meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception", meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig;

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

private:
    void UpdateCombatStateMachine(float DeltaTime);
    void UpdateBlackboard();
    void InitializePatrolWaypoints();
    void ApplySpeciesStats();

    float TimeSinceLastSeen = 0.0f;
    static constexpr float LoseTargetTime = 8.0f;
};
