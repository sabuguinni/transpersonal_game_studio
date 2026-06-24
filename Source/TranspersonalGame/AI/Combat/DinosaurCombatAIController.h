#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SharedTypes.h"
#include "DinosaurCombatAIController.generated.h"

// ============================================================
// Enums — global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Charging        UMETA(DisplayName = "Charging"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Flanking        UMETA(DisplayName = "Flanking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Roaring         UMETA(DisplayName = "Roaring")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    Tail            UMETA(DisplayName = "Tail"),
    Charge          UMETA(DisplayName = "Charge"),
    Stomp           UMETA(DisplayName = "Stomp"),
    Pounce          UMETA(DisplayName = "Pounce")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus")
};

// ============================================================
// Structs — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanInterrupt = true;
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
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AggroRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RoarRadius = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 PackSize = 1;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastSeenTimestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsVisible = false;
};

// ============================================================
// Main Controller Class
// ============================================================

UCLASS(ClassGroup = "Combat", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── State Machine ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|State")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    // ── Threat System ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    void ClearThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintPure, Category = "Combat|Threat")
    AActor* GetHighestThreat() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Threat")
    float GetThreatLevel(AActor* ThreatActor) const;

    // ── Attack System ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    bool TryExecuteAttack(ECombat_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void PerformBiteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void PerformChargeAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void PerformPounceAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void TriggerRoar();

    // ── Pack Coordination ──────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void BroadcastPackAlert(AActor* Target, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void ReceivePackAlert(AActor* Target, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void AssignFlankRole(int32 PackIndex, int32 TotalPackSize);

    // ── Navigation ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Nav")
    void MoveToFlankPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Nav")
    void MoveToPatrolPoint(FVector PatrolPoint);

    UFUNCTION(BlueprintCallable, Category = "Combat|Nav")
    void ChaseTarget(AActor* Target);

    // ── Species Configuration ──────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Config")
    void ConfigureForSpecies(ECombat_DinoSpecies Species);

    // ── Properties ────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    ECombat_DinoSpecies DinoSpecies = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_DinoStats DinoStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    TArray<FCombat_AttackData> AvailableAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    int32 PackIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    TArray<ADinosaurCombatAIController*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Territory")
    TArray<FVector> PatrolWaypoints;

private:
    UPROPERTY()
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY()
    TArray<FCombat_ThreatEntry> ThreatList;

    UPROPERTY()
    AActor* CurrentTarget = nullptr;

    UPROPERTY()
    float AttackCooldownRemaining = 0.0f;

    UPROPERTY()
    float StateTimer = 0.0f;

    UPROPERTY()
    int32 CurrentPatrolIndex = 0;

    UPROPERTY()
    float FlankAngleOffset = 0.0f;

    // Internal helpers
    void TickIdle(float DeltaTime);
    void TickPatrolling(float DeltaTime);
    void TickInvestigating(float DeltaTime);
    void TickStalking(float DeltaTime);
    void TickCharging(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickFlanking(float DeltaTime);
    void TickRetreating(float DeltaTime);

    void ScanForThreats();
    void UpdateThreatList(float DeltaTime);
    bool IsTargetInRange(AActor* Target, float Range) const;
    FVector CalculateFlankPosition(AActor* Target, float AngleOffset, float Distance) const;
    void ApplySpeciesDefaults_TRex();
    void ApplySpeciesDefaults_Raptor();
    void ApplySpeciesDefaults_Brachiosaurus();
};
