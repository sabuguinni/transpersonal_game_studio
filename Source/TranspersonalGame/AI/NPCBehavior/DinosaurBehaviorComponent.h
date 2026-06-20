#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DinosaurBehaviorComponent.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Pterodactyl UMETA(DisplayName = "Pterodactyl")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float WaitTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bLookAround = true;
};

USTRUCT(BlueprintType)
struct FNPC_DinoMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bIsPlayerTarget = false;
};

USTRUCT(BlueprintType)
struct FNPC_DinoPerceptionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SmellRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightAngleDeg = 120.0f;

    /** Sprint multiplier: running player detected at SightRadius * SprintDetectionMultiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SprintDetectionMultiplier = 1.5f;

    /** Crouch multiplier: crouching player detected at SightRadius * CrouchDetectionMultiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float CrouchDetectionMultiplier = 0.4f;
};

// ─── Component ────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

    // ── Species & State ────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::Raptor;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State")
    ENPC_DinoState CurrentState = ENPC_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State")
    ENPC_DinoState PreviousState = ENPC_DinoState::Idle;

    // ── Patrol ─────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    float PatrolSpeed = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Patrol")
    int32 CurrentPatrolIndex = 0;

    // ── Combat Ranges ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float ChaseRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackCooldown = 1.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Combat")
    float LastAttackTime = 0.0f;

    // ── Perception ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Perception")
    FNPC_DinoPerceptionConfig PerceptionConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Perception")
    bool bPlayerDetected = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Perception")
    bool bPlayerInChaseRange = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Perception")
    bool bPlayerInAttackRange = false;

    // ── Memory ─────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Memory")
    FNPC_DinoMemoryEntry PlayerMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Memory")
    float MemoryDuration = 15.0f;

    // ── Survival Stats ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger = 1.0f;  // 0=starving, 1=full

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float HungerDecayRate = 0.005f;

    // ── Flags from AnimInstance ────────────────────────────────────────────

    /** Set by AnimInstance — running player triggers extended detection */
    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur|Perception")
    bool bPlayerIsSprinting = false;

    /** Set by AnimInstance — fleeing player triggers pursuit escalation */
    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur|Perception")
    bool bPlayerIsFleeing = false;

    // ── Public API ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetState(ENPC_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void OnPlayerDetected(AActor* Player, float Distance, bool bIsSprinting);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void OnPlayerLost();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|State")
    bool IsAggressive() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|State")
    bool IsPassive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Patrol")
    FVector GetNextPatrolPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Patrol")
    void AdvancePatrolIndex();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Perception")
    float GetEffectiveSightRadius() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void TickMemory(float DeltaTime);
    void TickHunger(float DeltaTime);
    void UpdateStateFromPerception();

    UPROPERTY()
    AActor* TrackedPlayer = nullptr;
};
