#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DinosaurBehaviorComponent.generated.h"

// ============================================================
// Enums — NPC_ prefix to avoid collision with other agents
// ============================================================

UENUM(BlueprintType)
enum class ENPC_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Resting     UMETA(DisplayName = "Resting"),
    Alert       UMETA(DisplayName = "Alert"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Pterodactyl UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Curious     UMETA(DisplayName = "Curious"),
    Wary        UMETA(DisplayName = "Wary"),
    Hostile     UMETA(DisplayName = "Hostile"),
    Enraged     UMETA(DisplayName = "Enraged")
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitTimeSeconds = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLookAround = true;
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool bIsConfirmedThreat = false;
};

USTRUCT(BlueprintType)
struct FNPC_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float ChaseSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float AttackDamage = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float AttackCooldown = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float FieldOfViewDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float SmellRadius = 800.0f;
};

// ============================================================
// Component
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── State ──────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_DinoState CurrentState = ENPC_DinoState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::TRex;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Threat")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;

    // ── Stats ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    FNPC_DinoStats DinoStats;

    // ── Patrol ─────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    FVector HomeLocation = FVector::ZeroVector;

    // ── Memory ─────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FNPC_MemoryEntry PlayerMemory;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float AlertCooldown = 0.0f;

    // ── Pack behavior ──────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    bool bIsPackHunter = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    float PackCoordinationRadius = 3000.0f;

    // ── Timers ─────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Timers")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Timers")
    float StateEnteredTime = 0.0f;

    // ── Public API ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetState(ENPC_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnPlayerDetected(AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnPlayerLost();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void TakeDamage_Dino(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AlertPackMembers(AActor* Threat);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool CanAttack() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void GeneratePatrolPoints(int32 NumPoints = 4);

private:
    void UpdateStateMachine(float DeltaTime);
    void UpdateMemory(float DeltaTime);
    void EvaluateThreat();
    void TickIdle(float DeltaTime);
    void TickPatrolling(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickChasing(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickFleeing(float DeltaTime);

    UPROPERTY()
    AActor* TrackedPlayer = nullptr;

    float TimeInCurrentState = 0.0f;
};
