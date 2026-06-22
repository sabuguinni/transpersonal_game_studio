// DinosaurCombatAIController.h
// Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260622_006
// Dinosaur combat AI controller: reads NPCBehaviorComponent state,
// manages attack/chase/flee sequences for T-Rex and Raptor species.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "DinosaurCombatAIController.generated.h"

// Forward declarations
class UNPCBehaviorComponent;
class UCharacterMovementComponent;

UENUM(BlueprintType)
enum class ECombat_DinoAIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Roar        UMETA(DisplayName = "Roar"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Generic             UMETA(DisplayName = "Generic")
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Damage = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsAreaAttack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AreaAttackRadius = 150.0f;
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
    float WalkSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ChaseSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FleeSpeed = 1100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ChaseRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SightRange = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float HearingRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RoarCooldown = 15.0f;
};

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── State Machine ──────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_DinoAIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_DinoAIState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatAI(float DeltaTime);

    // ── Attack Logic ───────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeDamage_Combat(float DamageAmount, AActor* DamageSource);

    // ── Perception ─────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* ScanForPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetDistanceToTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPatrolPoints(const TArray<AActor*>& Points);

    // ── Species Config ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ApplySpeciesPreset(ECombat_DinoSpecies Species);

    // ── Properties ─────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    ECombat_DinoSpecies DinoSpecies = ECombat_DinoSpecies::TyrannosaurusRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    FCombat_DinoStats DinoStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    FCombat_AttackData AttackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    bool bPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float PackAlertRadius = 1500.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State",
        meta = (AllowPrivateAccess = "true"))
    ECombat_DinoAIState CurrentCombatState = ECombat_DinoAIState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float TimeSinceLastAttack = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float TimeSinceLastRoar = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    bool bIsRoaring = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Patrol")
    TArray<AActor*> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Patrol")
    int32 CurrentPatrolIndex = 0;

private:
    void State_Idle(float DeltaTime);
    void State_Patrol(float DeltaTime);
    void State_Alert(float DeltaTime);
    void State_Chase(float DeltaTime);
    void State_Attack(float DeltaTime);
    void State_Flee(float DeltaTime);
    void State_Roar(float DeltaTime);

    void AlertNearbyPackMembers();
    void SetMovementSpeed(float Speed);
    void MoveToNextPatrolPoint();

    float RoarTimer = 0.0f;
    float PatrolWaitTimer = 0.0f;
    bool bWaitingAtPatrolPoint = false;
};
