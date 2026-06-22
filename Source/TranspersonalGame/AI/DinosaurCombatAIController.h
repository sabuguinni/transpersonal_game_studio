#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NPCBehaviorComponent.h"
#include "DinosaurCombatAIController.generated.h"

// Forward declarations
class UNavigationSystemV1;
class AActor;

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Generic     UMETA(DisplayName = "Generic Dinosaur")
};

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Stalk       UMETA(DisplayName = "Stalk - Observing target"),
    Charge      UMETA(DisplayName = "Charge - Direct attack"),
    Flank       UMETA(DisplayName = "Flank - Circling target"),
    Retreat     UMETA(DisplayName = "Retreat - Low health flee"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Territorial UMETA(DisplayName = "Territorial - Defending zone")
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanKnockdown = false;
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FCombat_AttackData PrimaryAttack;
};

/**
 * ADinosaurCombatAIController
 *
 * Tactical combat AI for dinosaur pawns.
 * Implements species-specific behaviour:
 *   - T-Rex: territorial charge, stomp, roar stagger
 *   - Velociraptor: pack flanking, stalk-then-burst, retreat when outnumbered
 *   - Brachiosaurus: passive unless threatened, area stomp
 *
 * Reads ENPC_BehaviorState from UNPCBehaviorComponent on the pawn.
 * Drives movement via UE5 navigation (MoveToActor / MoveToLocation).
 */
UCLASS(ClassGroup = "TranspersonalGame|AI", meta = (DisplayName = "Dinosaur Combat AI Controller"))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    FCombat_DinoStats DinoStats;

    // --- Current State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_AIState CurrentState = ECombat_AIState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|State")
    AActor* CurrentTarget = nullptr;

    // --- Pack Behaviour (Velociraptor) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    bool bIsPackMember = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    TArray<ADinosaurCombatAIController*> PackMembers;

    // --- Patrol ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Patrol")
    TArray<AActor*> PatrolWaypoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Patrol")
    int32 CurrentWaypointIndex = 0;

    // --- Blueprint-callable functions ---
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ClearCombatTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeDamage_Dino(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void NotifyPackOfThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ForceState(ECombat_AIState NewState);

private:
    // State machine tick functions
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickStalk(float DeltaTime);
    void TickCharge(float DeltaTime);
    void TickFlank(float DeltaTime);
    void TickRetreat(float DeltaTime);

    void TransitionToState(ECombat_AIState NewState);
    void ScanForTargets();
    bool IsTargetInRange(float Range) const;
    void AdvancePatrolWaypoint();
    void ExecuteAttack();

    // Species-specific behaviour modifiers
    void ApplySpeciesDefaults();

    // Timers
    float TimeSinceLastAttack = 0.0f;
    float StalkTimer = 0.0f;
    float StalkDuration = 3.0f;  // Raptors observe for 3s before charging
    float IdleTimer = 0.0f;

    // Cached component reference
    UPROPERTY()
    UNPCBehaviorComponent* BehaviorComp = nullptr;
};
