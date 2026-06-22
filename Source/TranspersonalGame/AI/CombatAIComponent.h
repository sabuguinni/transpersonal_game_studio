// CombatAIComponent.h
// Combat & Enemy AI Agent #12 — Transpersonal Game Studio
// Tactical combat AI for dinosaurs: threat detection, flanking, pack coordination
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "CombatAIComponent.generated.h"

// Combat state enum — global scope (RULE 1)
UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    PackHunt    UMETA(DisplayName = "PackHunt")
};

// Threat level enum
UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None    UMETA(DisplayName = "None"),
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Lethal  UMETA(DisplayName = "Lethal")
};

// Combat species type
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "TRex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Pterodactyl UMETA(DisplayName = "Pterodactyl")
};

// Patrol waypoint struct
USTRUCT(BlueprintType)
struct FCombat_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float WaitTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsAlertPoint = false;
};

// Pack coordination data
USTRUCT(BlueprintType)
struct FCombat_PackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    AActor* PackLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsCoordinating = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CoordinationRadius = 1500.0f;
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

    // ── Species & Stats ──────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float DetectionRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float PatrolSpeed = 200.0f;

    // ── State ────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_AIState CurrentState = ECombat_AIState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float TimeSinceLastAttack = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|State")
    float AttackCooldown = 1.5f;

    // ── Patrol ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Patrol")
    TArray<FCombat_PatrolPoint> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Patrol")
    int32 CurrentPatrolIndex = 0;

    // ── Pack Behavior ────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    FCombat_PackData PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    float FlankingAngle = 90.0f;

    // ── Tactical Behavior ────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Tactics")
    bool bCanAmbush = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Tactics")
    float AmbushTriggerDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Tactics")
    bool bFleesWhenLowHealth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Tactics")
    float FleeHealthThreshold = 20.0f;

    // ── Public Interface ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeDamage_Combat(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AlertPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatLevel EvaluateThreat(AActor* PotentialThreat) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition(int32 FlankIndex, int32 TotalFlankers) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetCurrentState() const { return CurrentState; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void UpdateCombatState(float DeltaTime);
    void ProcessAttack(float DeltaTime);
    void ScanForThreats();
    bool bIsDead = false;
};
