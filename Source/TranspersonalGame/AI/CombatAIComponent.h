#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatAIComponent.generated.h"

// ============================================================
// Combat AI Component — Agent #12
// Tactical combat intelligence for dinosaur enemies.
// Attach to: TRex pawn, Raptor pawn, any hostile creature.
// ============================================================

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Detect          UMETA(DisplayName = "Detect"),       // Heard/saw something
    Chase           UMETA(DisplayName = "Chase"),        // Pursuing target
    Attack          UMETA(DisplayName = "Attack"),       // In attack range
    Reposition      UMETA(DisplayName = "Reposition"),  // Circling/flanking
    Retreat         UMETA(DisplayName = "Retreat"),      // Injured, backing off
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    None            UMETA(DisplayName = "None"),
    Bite            UMETA(DisplayName = "Bite"),         // Close range, high damage
    Charge          UMETA(DisplayName = "Charge"),       // Dash + knockback
    Swipe           UMETA(DisplayName = "Swipe"),        // Claw sweep, medium range
    Stomp           UMETA(DisplayName = "Stomp"),        // AoE ground slam (TRex)
    PackFlank       UMETA(DisplayName = "PackFlank")     // Coordinated raptor attack
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Generic         UMETA(DisplayName = "Generic")
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Damage = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Range = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Cooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanInterrupt = false;
};

USTRUCT(BlueprintType)
struct FCombat_SpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AttackRadius = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    bool bIsAggressive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float RetreatHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    TArray<FCombat_AttackData> AvailableAttacks;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), DisplayName = "Combat AI Component")
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── State Machine ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void TransitionToState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|State")
    ECombat_AIState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat|State")
    AActor* GetTarget() const { return CurrentTarget; }

    // ── Combat Actions ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    bool TryPerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    ECombat_AttackType SelectBestAttack(float DistanceToTarget) const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void ApplyAttackDamage(const FCombat_AttackData& Attack);

    // ── Detection ──────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Detection")
    void OnPlayerDetected(AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Combat|Detection")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Detection")
    bool IsTargetInDetectionRange() const;

    // ── Pack Coordination ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void NotifyPackMembersOfTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void ReceivePackSignal(AActor* Target, FVector FlankPosition);

    UFUNCTION(BlueprintPure, Category = "Combat|Pack")
    FVector GetAssignedFlankPosition() const { return AssignedFlankPosition; }

    // ── Health & Damage ────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Health")
    float TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintPure, Category = "Combat|Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Health")
    bool IsAlive() const { return CurrentHealth > 0.0f; }

    // ── Species Config ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Config")
    void ApplySpeciesPreset(ECombat_DinoSpecies Species);

    // ── Properties ────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_SpeciesTraits SpeciesTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    bool bDebugDrawDetectionRadius = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State",
        meta = (AllowPrivateAccess = "true"))
    ECombat_AIState CurrentState = ECombat_AIState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Health")
    float CurrentHealth = 500.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    FVector AssignedFlankPosition = FVector::ZeroVector;

private:
    UPROPERTY()
    AActor* CurrentTarget = nullptr;

    float AttackCooldownRemaining = 0.0f;
    float StateTimer = 0.0f;
    float AlertTimer = 0.0f;
    bool bPackSignalReceived = false;

    void TickIdle(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickReposition(float DeltaTime);
    void TickRetreat(float DeltaTime);

    void InitTRexPreset();
    void InitRaptorPreset();
    void InitBrachiosaurusPreset();
    void InitTriceratopsPreset();
};
