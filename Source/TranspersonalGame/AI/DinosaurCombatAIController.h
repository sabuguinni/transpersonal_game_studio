#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "DinosaurCombatAIController.generated.h"

// Forward declarations
class UNPCBehaviorComponent;
class ACharacter;

UENUM(BlueprintType)
enum class ECombat_TacticalPhase : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Alert           UMETA(DisplayName = "Alert"),
    Stalk           UMETA(DisplayName = "Stalk"),
    Charge          UMETA(DisplayName = "Charge"),
    Attack          UMETA(DisplayName = "Attack"),
    Recover         UMETA(DisplayName = "Recover"),
    Flee            UMETA(DisplayName = "Flee"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FCombat_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    float WaitTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    bool bAlertOnArrival = false;
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float BaseDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float ChargeSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float KnockbackForce = 1200.0f;
};

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

    // ── Species & Phase ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Phase")
    ECombat_TacticalPhase CurrentPhase = ECombat_TacticalPhase::Idle;

    // ── Attack Configuration ─────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    FCombat_AttackData AttackData;

    // ── Detection ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float DetectionRange = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float LoseTargetRange = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float AlertRange = 1500.0f;

    // ── Patrol ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    TArray<FCombat_PatrolPoint> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    float PatrolRadius = 2000.0f;

    // ── Pack Coordination ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackCoordinationRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    int32 PackSize = 1;

    // ── Health ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health")
    float MaxHealth = 1000.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Health")
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health")
    float FleeHealthThreshold = 0.25f;

    // ── Public Interface ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTacticalPhase(ECombat_TacticalPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeCombatDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AlertPackMembers(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetPatrolPoints(const TArray<FCombat_PatrolPoint>& NewPoints);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_TacticalPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsHostile() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

private:
    // ── Internal State ───────────────────────────────────────────────────────
    UPROPERTY()
    AActor* CurrentTarget = nullptr;

    UPROPERTY()
    APawn* ControlledPawn = nullptr;

    float AttackCooldownRemaining = 0.0f;
    float PatrolWaitRemaining = 0.0f;
    int32 CurrentPatrolIndex = 0;
    float PhaseTimer = 0.0f;
    float ScanTimer = 0.0f;
    bool bIsCharging = false;

    // ── Phase Tick Functions ─────────────────────────────────────────────────
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickStalk(float DeltaTime);
    void TickCharge(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickRecover(float DeltaTime);
    void TickFlee(float DeltaTime);

    // ── Helpers ──────────────────────────────────────────────────────────────
    AActor* ScanForPlayer();
    float GetDistanceToTarget() const;
    void ExecuteAttack();
    void MoveToTarget(float AcceptanceRadius = 100.0f);
    void MoveToPatrolPoint();
    void AdvancePatrolIndex();
    FVector GetFleeDirection() const;
    void ApplySpeciesDefaults();
};
