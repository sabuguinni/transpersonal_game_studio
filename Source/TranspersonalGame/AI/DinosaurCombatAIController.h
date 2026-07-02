#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DinosaurCombatAIController.generated.h"

// ============================================================
// Combat AI Types — Agent #12
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Disengage   UMETA(DisplayName = "Disengage"),
    Roar        UMETA(DisplayName = "Roar"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FCombat_DinoSpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float DetectionRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AttackRange = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AttackDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float DisengageDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float PackAlertRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float RoarCooldown = 15.0f;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float TimeLastSeen = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    bool bIsCurrentlyVisible = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_DinoStateChanged, ECombat_DinoState, OldState, ECombat_DinoState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_PackAlertBroadcast, FVector, ThreatLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_DamageDealt, AActor*, Target, float, DamageAmount);

/**
 * DinosaurCombatAIController
 * Agent #12 — Combat & Enemy AI
 * 
 * Tactical combat AI for dinosaurs. Handles:
 * - Perception (sight + hearing via AIPerceptionComponent)
 * - State machine: Idle→Patrol→Investigate→Chase→Attack→Disengage
 * - Species-specific traits (T-Rex vs Raptor vs Triceratops)
 * - Pack coordination: raptors share threat memory
 * - Damage application to TranspersonalCharacter survival stats
 * - Roar mechanic: alerts nearby pack members
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dinosaur Combat AI Controller"))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── State Machine ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|State")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void RegisterThreat(AActor* ThreatActor, FVector Location, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void ClearThreat(AActor* ThreatActor);

    // ── Species Configuration ──────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TyrannosaurusRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FCombat_DinoSpeciesTraits SpeciesTraits;

    // ── Pack Coordination ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void BroadcastPackAlert(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void ReceivePackAlert(FVector ThreatLocation);

    // ── Delegates ─────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_DinoStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_PackAlertBroadcast OnPackAlertBroadcast;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_DamageDealt OnDamageDealt;

    // ── Debug ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Debug")
    bool bShowDebugVisuals = true;

protected:
    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // State tick handlers
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickInvestigate(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickDisengage(float DeltaTime);

    // Combat actions
    void ExecuteAttack(AActor* Target);
    void PerformRoar();
    void UpdateThreatMemory(float DeltaTime);
    AActor* SelectHighestThreat() const;
    bool IsTargetInAttackRange(AActor* Target) const;
    void DrawDebugState() const;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig;

    // State
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;
    ECombat_DinoState PreviousState = ECombat_DinoState::Idle;

    TArray<FCombat_ThreatEntry> ThreatMemory;
    AActor* CurrentTarget = nullptr;

    float AttackCooldownTimer = 0.0f;
    float RoarCooldownTimer = 0.0f;
    float StateTimer = 0.0f;
    float PatrolWaitTimer = 0.0f;

    FVector PatrolOrigin = FVector::ZeroVector;
    FVector InvestigateTarget = FVector::ZeroVector;

    static const float ThreatDecayRate;
    static const float ThreatMemoryTimeout;
    static const float PatrolRadius;
};
