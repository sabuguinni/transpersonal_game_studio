#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatAIComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ECombat_AIState — tactical state machine for dinosaur/enemy combat AI
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Stalk           UMETA(DisplayName = "Stalk"),
    Charge          UMETA(DisplayName = "Charge"),
    Flank           UMETA(DisplayName = "Flank"),
    Ambush          UMETA(DisplayName = "Ambush"),
    Attack          UMETA(DisplayName = "Attack"),
    Retreat         UMETA(DisplayName = "Retreat"),
    Stunned         UMETA(DisplayName = "Stunned"),
    Dead            UMETA(DisplayName = "Dead")
};

// ─────────────────────────────────────────────────────────────────────────────
// ECombat_AttackType — type of attack being executed
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    None            UMETA(DisplayName = "None"),
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    TailSwipe       UMETA(DisplayName = "TailSwipe"),
    Charge          UMETA(DisplayName = "Charge"),
    Pounce          UMETA(DisplayName = "Pounce"),
    Stomp           UMETA(DisplayName = "Stomp")
};

// ─────────────────────────────────────────────────────────────────────────────
// ECombat_DinoSpecies — species determines base AI behavior profile
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    Generic         UMETA(DisplayName = "Generic"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus")
};

// ─────────────────────────────────────────────────────────────────────────────
// FCombat_AttackRecord — log of recent attacks for cooldown tracking
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCombat_AttackRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombat_AttackType AttackType = ECombat_AttackType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float TimeExecuted = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float DamageDealt = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    bool bHitTarget = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// FCombat_ThreatTarget — information about the current threat/target
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCombat_ThreatTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* TargetActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    bool bIsVisible = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// UCombatAIComponent — core combat AI component for dinosaurs and enemies
// Attach to any APawn/ACharacter to give it tactical combat intelligence.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Combat AI Component")
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Species Configuration ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Generic;

    // ── State ──────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_AIState CurrentState = ECombat_AIState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_AIState PreviousState = ECombat_AIState::Idle;

    // ── Threat Tracking ────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Threat")
    FCombat_ThreatTarget PrimaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Threat")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Threat")
    float AttackRadius = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Threat")
    float FlankRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Threat")
    float RetreatHealthThreshold = 0.25f;

    // ── Attack Configuration ───────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float BaseDamage = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float ChargeCooldown = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float PounceRange = 400.0f;

    // ── Pack Behavior ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    float PackCoordinationRadius = 1200.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Pack")
    int32 PackMembersNearby = 0;

    // ── Health ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Health")
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Health")
    float CurrentHealth = 100.0f;

    // ── Attack History ─────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|History")
    TArray<FCombat_AttackRecord> AttackHistory;

    // ── Public API ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeCombatDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ForceState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanCharge() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector ComputeFlankPosition(bool bLeftFlank) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AttackType SelectBestAttack() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool HasTarget() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetDistanceToTarget() const;

private:
    // Internal tick rate control
    float TickAccumulator = 0.0f;
    static constexpr float TickInterval = 0.1f; // 10Hz

    // Cooldown timers
    float AttackCooldownRemaining = 0.0f;
    float ChargeCooldownRemaining = 0.0f;
    float StunDurationRemaining = 0.0f;

    // Internal methods
    void ApplySpeciesProfile();
    void TickCombatAI(float DeltaTime);
    void UpdateTargetVisibility();
    void EvaluateCombatState();
    void TickStateLogic(float DeltaTime);
    void ExecuteAttack(ECombat_AttackType AttackType);
    void CountPackMembers();

    // State tick handlers
    void TickStalk(float DeltaTime);
    void TickCharge(float DeltaTime);
    void TickFlank(float DeltaTime);
    void TickAmbush(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickRetreat(float DeltaTime);
};
