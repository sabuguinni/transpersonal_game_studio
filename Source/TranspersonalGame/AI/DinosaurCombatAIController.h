#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NPCBehaviorComponent.h"
#include "DinosaurCombatAIController.generated.h"

// Forward declarations
class UNavigationSystemV1;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

/** Combat phase for this encounter */
UENUM(BlueprintType)
enum class ECombat_Phase : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Alerted         UMETA(DisplayName = "Alerted"),
    Chasing         UMETA(DisplayName = "Chasing"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Recovering      UMETA(DisplayName = "Recovering"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Dead            UMETA(DisplayName = "Dead")
};

/** Per-species combat configuration */
USTRUCT(BlueprintType)
struct FCombat_SpeciesConfig
{
    GENERATED_BODY()

    /** Detection range in UU (Unreal Units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float SightRadius = 2000.0f;

    /** Hearing range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float HearingRadius = 1500.0f;

    /** Attack melee range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackRange = 250.0f;

    /** Base damage per hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackDamage = 40.0f;

    /** Seconds between attacks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackCooldown = 2.5f;

    /** Max movement speed when chasing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Movement")
    float ChaseSpeed = 600.0f;

    /** Max movement speed when patrolling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Movement")
    float PatrolSpeed = 200.0f;

    /** Health threshold (0-1) below which dino flees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Survival")
    float FleeHealthThreshold = 0.2f;

    /** Does this species flee at all? (T-Rex never flees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Survival")
    bool bCanFlee = true;

    /** Does this species call pack members when alerted? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackHunter = false;

    /** Radius to alert nearby pack members */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackAlertRadius = 1200.0f;
};

/**
 * ADinosaurCombatAIController
 *
 * AI Controller for all dinosaur combat encounters.
 * Reads UNPCBehaviorComponent state and drives movement + attack decisions.
 * Integrates with UE5 AIPerception for sight/hearing detection.
 *
 * Species presets:
 *   TRex         — 2000 sight, 40 dmg, never flees, solo hunter
 *   Velociraptor — 1500 sight, 20 dmg, pack hunter (alerts 3 raptors)
 *   Triceratops  — 1500 sight, 50 dmg, charges when player enters 1500u, flees at 20% HP
 *   Brachiosaurus — passive, only attacks if struck directly
 *   Ankylosaurus — 800 sight, 60 dmg tail sweep, slow but armoured
 */
UCLASS(ClassGroup = "TranspersonalGame|AI", meta = (DisplayName = "Dinosaur Combat AI Controller"))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    // === AAIController overrides ===
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    // === Species configuration ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_SpeciesConfig SpeciesConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    ENPC_DinoPersonality DinoPersonality = ENPC_DinoPersonality::TerritorialPatrol;

    // === State ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_Phase CurrentPhase = ECombat_Phase::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|State")
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    float AttackCooldownRemaining = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget = nullptr;

    // === Patrol waypoints ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    TArray<AActor*> PatrolWaypoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Patrol")
    int32 CurrentWaypointIndex = 0;

    // === Public interface ===
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnPlayerDetected(AActor* Player, FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnTakeDamage(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetSpeciesPreset(ENPC_DinoPersonality Personality);

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_Phase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsHostile() const;

    // === Static species presets ===
    UFUNCTION(BlueprintCallable, Category = "Combat|Presets")
    static FCombat_SpeciesConfig GetTRexPreset();

    UFUNCTION(BlueprintCallable, Category = "Combat|Presets")
    static FCombat_SpeciesConfig GetVelociraptorPreset();

    UFUNCTION(BlueprintCallable, Category = "Combat|Presets")
    static FCombat_SpeciesConfig GetTriceratopsPreset();

    UFUNCTION(BlueprintCallable, Category = "Combat|Presets")
    static FCombat_SpeciesConfig GetBrachiosaurusPreset();

    UFUNCTION(BlueprintCallable, Category = "Combat|Presets")
    static FCombat_SpeciesConfig GetAnkylosaurusPreset();

protected:
    // === Perception ===
    UPROPERTY(VisibleAnywhere, Category = "Combat|Perception")
    UAIPerceptionComponent* PerceptionComponent_Combat = nullptr;

    // === Internal state machine ===
    void TickIdle(float DeltaTime);
    void TickPatrolling(float DeltaTime);
    void TickAlerted(float DeltaTime);
    void TickChasing(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickRecovering(float DeltaTime);
    void TickFleeing(float DeltaTime);

    void TransitionToPhase(ECombat_Phase NewPhase);
    void AlertPackMembers();
    void ExecuteAttack();
    bool IsPlayerInAttackRange() const;
    bool IsPlayerInSightRange() const;
    FVector GetFleeDestination() const;
    FVector GetNextPatrolPoint();

    // === Cached refs ===
    UPROPERTY()
    UNPCBehaviorComponent* BehaviorComponent = nullptr;

    float AlertedTimer = 0.0f;
    float RecoveryTimer = 0.0f;
    float FleeTimer = 0.0f;
    bool bHasAlertedPack = false;
};
