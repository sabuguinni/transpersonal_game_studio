#pragma once

// NPCBehaviorConfig.h
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260622_013
// Data-driven NPC behavior configuration structs for prehistoric survival NPCs

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NPCBehaviorConfig.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_AlertState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Suspicious  UMETA(DisplayName = "Suspicious"),
    Alert       UMETA(DisplayName = "Alert"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DailyTask : uint8
{
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Resting     UMETA(DisplayName = "Resting"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Socializing UMETA(DisplayName = "Socializing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Guarding    UMETA(DisplayName = "Guarding")
};

UENUM(BlueprintType)
enum class ENPC_Archetype : uint8
{
    Scout       UMETA(DisplayName = "Scout"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Elder       UMETA(DisplayName = "Elder"),
    Lookout     UMETA(DisplayName = "Lookout"),
    Craftsman   UMETA(DisplayName = "Craftsman")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitTimeSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLookAround = true;
};

USTRUCT(BlueprintType)
struct FNPC_DailyScheduleEntry
{
    GENERATED_BODY()

    // Hour of day (0-23)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule", meta = (ClampMin = "0", ClampMax = "23"))
    int32 StartHour = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    ENPC_DailyTask Task = ENPC_DailyTask::Patrolling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    FVector TaskLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    float TaskDurationHours = 2.0f;
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    // What was perceived
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float TimestampSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    // Was this a dinosaur threat?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    bool bWasDinosaurThreat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    bool bWasPlayerSighting = false;
};

USTRUCT(BlueprintType)
struct FNPC_SensoryProfile
{
    GENERATED_BODY()

    // Sight range in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float SightRange = 2000.0f;

    // Peripheral angle (half-angle from forward)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses", meta = (ClampMin = "10.0", ClampMax = "180.0"))
    float SightAngleDegrees = 70.0f;

    // Hearing range in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses", meta = (ClampMin = "100.0", ClampMax = "8000.0"))
    float HearingRange = 1500.0f;

    // Smell range (wind-dependent)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses", meta = (ClampMin = "0.0", ClampMax = "3000.0"))
    float SmellRange = 500.0f;

    // Night penalty multiplier (0.0 = blind at night, 1.0 = no penalty)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NightSightMultiplier = 0.4f;
};

USTRUCT(BlueprintType)
struct FNPC_CombatProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float AttackDamage = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float AttackCooldown = 1.5f;

    // Courage: 0 = always flees, 1 = always fights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CourageLevel = 0.5f;

    // Does this NPC flee from dinosaurs?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    bool bFleesFromDinosaurs = true;

    // Flee trigger: health percentage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FleeTriggerHealthPercent = 0.3f;
};

// ─── Main Config Object ───────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPCBehaviorConfig : public UObject
{
    GENERATED_BODY()

public:
    UNPCBehaviorConfig();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    FName NPCName = "Unknown";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    ENPC_Archetype Archetype = ENPC_Archetype::Scout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|State")
    ENPC_AlertState CurrentAlertState = ENPC_AlertState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    TArray<FNPC_DailyScheduleEntry> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryBank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Senses")
    FNPC_SensoryProfile SensoryProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    FNPC_CombatProfile CombatProfile;

    // Max memory entries before oldest is discarded
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory", meta = (ClampMin = "1", ClampMax = "20"))
    int32 MaxMemoryEntries = 8;

    // Health
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float CurrentHealth = 100.0f;

    // Movement speeds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
    float WalkSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
    float RunSpeed = 450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
    float FleeSpeed = 550.0f;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void AddMemoryEntry(const FNPC_MemoryEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ClearOldMemories(float CurrentTime, float MaxAgeSeconds = 120.0f);

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsHostile() const;

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsFleeing() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    void SetAlertState(ENPC_AlertState NewState);
};
