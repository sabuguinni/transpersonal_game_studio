// CombatAIComponent.h
// Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260629_002
// Tactical combat AI for dinosaurs and hostile NPCs.
// Binds to NPCBehaviorComponent::OnThreatDetected delegate.
// All types prefixed Combat_ per RULE 2 (unique names across project).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "CombatAIComponent.generated.h"

// ─────────────────────────────────────────────
// UENUM — Combat attack phase (global scope, RULE 1)
// ─────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_AttackPhase : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Charging        UMETA(DisplayName = "Charging"),
    Striking        UMETA(DisplayName = "Striking"),
    Recovering      UMETA(DisplayName = "Recovering"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Circling        UMETA(DisplayName = "Circling"),
    PackCoordinate  UMETA(DisplayName = "PackCoordinate")
};

// ─────────────────────────────────────────────
// UENUM — Dinosaur combat archetype (global scope, RULE 1)
// ─────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_DinoArchetype : uint8
{
    Ambush          UMETA(DisplayName = "Ambush"),      // Raptors — flank, leap, pin
    Charger         UMETA(DisplayName = "Charger"),     // Carnotaurus — head-on rush
    Crusher         UMETA(DisplayName = "Crusher"),     // T-Rex — power, area denial
    Skirmisher      UMETA(DisplayName = "Skirmisher"),  // Dilophosaurus — spit, retreat
    Herd            UMETA(DisplayName = "Herd"),        // Triceratops — group charge
    Ambusher        UMETA(DisplayName = "Ambusher")     // Spinosaurus — water/shadow
};

// ─────────────────────────────────────────────
// UENUM — Combat threat response (global scope, RULE 1)
// ─────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_ThreatResponse : uint8
{
    Ignore          UMETA(DisplayName = "Ignore"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Warn            UMETA(DisplayName = "Warn"),
    Attack          UMETA(DisplayName = "Attack"),
    Flee            UMETA(DisplayName = "Flee")
};

// ─────────────────────────────────────────────
// USTRUCT — Single combat action record (global scope, RULE 1)
// ─────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCombat_ActionRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombat_AttackPhase Phase = ECombat_AttackPhase::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float Timestamp = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float DamageDealt = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    bool bHitTarget = false;
};

// ─────────────────────────────────────────────
// USTRUCT — Pack coordination slot (global scope, RULE 1)
// ─────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCombat_PackSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* PackMember = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombat_AttackPhase AssignedRole = ECombat_AttackPhase::Circling;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector ApproachVector = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    bool bIsLeader = false;
};

// ─────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnAttackPhaseChanged,
    ECombat_AttackPhase, OldPhase, ECombat_AttackPhase, NewPhase);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnDamageDealt,
    AActor*, Target, float, DamageAmount);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnTargetLost,
    AActor*, LastKnownTarget);

// ─────────────────────────────────────────────
// UCombatAIComponent
// ─────────────────────────────────────────────
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent),
       DisplayName = "Combat AI Component")
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

    // ── Lifecycle ──
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Core combat interface (Blueprint callable) ──

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TransitionToPhase(ECombat_AttackPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateAttackDamage() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetInLineOfSight() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterPackMember(AActor* Member, bool bAsLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatResponse EvaluateThreatResponse(AActor* ThreatActor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition(int32 SlotIndex, int32 TotalSlots) const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AttackPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetDistanceToTarget() const;

    // ── Configuration ──

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    ECombat_DinoArchetype Archetype = ECombat_DinoArchetype::Ambush;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float DetectionRadius = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float AttackRadius = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float BaseDamage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float AttackCooldown = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float StalkDuration = 6.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float ChargeSpeed = 900.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float RetreatHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    int32 MaxPackSize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    bool bCanPackCoordinate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    bool bUsesAmbushTactics = false;

    // ── Runtime state ──

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_AttackPhase CurrentPhase = ECombat_AttackPhase::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float CurrentHealth = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float MaxHealth = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float TimeSinceLastAttack = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float PhaseTimer = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    TArray<FCombat_PackSlot> PackSlots;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    TArray<FCombat_ActionRecord> ActionHistory;

    // ── Delegates ──

    UPROPERTY(BlueprintAssignable, Category = "Combat AI|Events")
    FCombat_OnAttackPhaseChanged OnAttackPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat AI|Events")
    FCombat_OnDamageDealt OnDamageDealt;

    UPROPERTY(BlueprintAssignable, Category = "Combat AI|Events")
    FCombat_OnTargetLost OnTargetLost;

private:
    // Per-phase tick helpers
    void TickIdle(float DeltaTime);
    void TickStalking(float DeltaTime);
    void TickCharging(float DeltaTime);
    void TickStriking(float DeltaTime);
    void TickRecovering(float DeltaTime);
    void TickRetreating(float DeltaTime);
    void TickCircling(float DeltaTime);
    void TickPackCoordinate(float DeltaTime);

    // Internal helpers
    void ScanForTargets();
    void RecordAction(ECombat_AttackPhase Phase, float Damage, bool bHit);
    void ApplyArchetypeBehavior();
    bool ShouldRetreat() const;

    FTimerHandle ScanTimerHandle;
    FTimerHandle AttackCooldownHandle;
    bool bAttackOnCooldown = false;
};
