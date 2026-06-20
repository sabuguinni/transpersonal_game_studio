#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// NPC_RoutinePhase — daily schedule phases for NPC agents
// ============================================================
UENUM(BlueprintType)
enum class ENPC_RoutinePhase : uint8
{
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Waking          UMETA(DisplayName = "Waking"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Resting         UMETA(DisplayName = "Resting"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Hiding          UMETA(DisplayName = "Hiding"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Hunting         UMETA(DisplayName = "Hunting")
};

// ============================================================
// NPC_ThreatLevel — perceived danger from environment/player
// ============================================================
UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Curious     UMETA(DisplayName = "Curious"),
    Wary        UMETA(DisplayName = "Wary"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Panicking   UMETA(DisplayName = "Panicking"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

// ============================================================
// NPC_SocialRelation — NPC's attitude toward another agent
// ============================================================
UENUM(BlueprintType)
enum class ENPC_SocialRelation : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Neutral     UMETA(DisplayName = "Neutral"),
    Friendly    UMETA(DisplayName = "Friendly"),
    Tribal      UMETA(DisplayName = "Tribal"),
    Hostile     UMETA(DisplayName = "Hostile"),
    Feared      UMETA(DisplayName = "Feared")
};

// ============================================================
// NPC_MemoryRecord — a single episodic memory entry
// ============================================================
USTRUCT(BlueprintType)
struct FNPC_MemoryRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector EventLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float EventTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bWasDangerous = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FName EventTag = NAME_None;
};

// ============================================================
// NPC_ScheduleEntry — one time-block in the daily routine
// ============================================================
USTRUCT(BlueprintType)
struct FNPC_ScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    ENPC_RoutinePhase Phase = ENPC_RoutinePhase::Resting;

    // Hour of day (0-24) when this phase begins
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    float StartHour = 0.0f;

    // Duration in game-hours
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    float DurationHours = 2.0f;

    // World-space anchor point for this activity
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    FVector AnchorLocation = FVector::ZeroVector;
};

// ============================================================
// UNPCBehaviorComponent — drives daily routine, memory, threat
// ============================================================
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Daily Schedule ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    TArray<FNPC_ScheduleEntry> DailySchedule;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Schedule")
    ENPC_RoutinePhase CurrentPhase = ENPC_RoutinePhase::Resting;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Schedule")
    float CurrentGameHour = 8.0f;

    // ---- Threat & Awareness ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Threat")
    float ThreatDetectionRadius = 1500.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Threat")
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Threat")
    float ThreatScore = 0.0f;

    // ---- Memory System ----

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryRecord> EpisodicMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryRecords = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayRate = 0.02f;

    // ---- Social ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Social")
    ENPC_SocialRelation RelationToPlayer = ENPC_SocialRelation::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Social")
    FName TribeName = NAME_None;

    // ---- Public API ----

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RecordMemory(FVector Location, float Threat, bool bDangerous, FName Tag);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void UpdateThreatLevel(float NewThreatScore);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_RoutinePhase GetPhaseForHour(float Hour) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetGameHour(float Hour);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsInDanger() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    FVector GetMostDangerousMemoryLocation() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void BuildDefaultHunterSchedule();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void BuildDefaultGathererSchedule();

private:
    void TickSchedule(float DeltaTime);
    void TickMemoryDecay(float DeltaTime);
    void TickThreatDecay(float DeltaTime);

    float GameTimeAccumulator = 0.0f;
    // 1 real second = GameHoursPerSecond game-hours
    float GameHoursPerSecond = 0.00278f; // ~1 game-day per real hour
};
