#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// NPC daily routine schedule slot
UENUM(BlueprintType)
enum class ENPC_RoutineActivity : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Forage          UMETA(DisplayName = "Forage"),
    Rest            UMETA(DisplayName = "Rest"),
    Flee            UMETA(DisplayName = "Flee"),
    Investigate     UMETA(DisplayName = "Investigate"),
    SocialInteract  UMETA(DisplayName = "SocialInteract"),
    SeekShelter     UMETA(DisplayName = "SeekShelter"),
    Eat             UMETA(DisplayName = "Eat"),
    Alert           UMETA(DisplayName = "Alert")
};

// NPC emotional/threat state
UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Wary        UMETA(DisplayName = "Wary"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Panicked    UMETA(DisplayName = "Panicked"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

// Memory entry — what this NPC remembers about a stimulus
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsPlayerRelated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FString StimulusTag;
};

// Daily schedule entry
USTRUCT(BlueprintType)
struct FNPC_ScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    float StartHour = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    float EndHour = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    ENPC_RoutineActivity Activity = ENPC_RoutineActivity::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    FVector TargetLocation = FVector::ZeroVector;
};

/**
 * UNPCBehaviorComponent
 * Drives NPC daily routines, memory system, and threat response.
 * Attach to any ACharacter or APawn to give it autonomous survival behavior.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Routine System ──────────────────────────────────────────────────────

    /** Current activity this NPC is performing */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Routine")
    ENPC_RoutineActivity CurrentActivity = ENPC_RoutineActivity::Idle;

    /** Daily schedule — ordered list of time-bound activities */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_ScheduleEntry> DailySchedule;

    /** Current in-game hour (0-24, driven externally by day/night system) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float CurrentGameHour = 8.0f;

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void UpdateSchedule(float GameHour);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    ENPC_RoutineActivity GetScheduledActivityForHour(float Hour) const;

    // ── Threat & Emotion System ──────────────────────────────────────────────

    /** Current threat perception level */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Threat")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::Calm;

    /** Radius within which this NPC detects threats */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Threat")
    float ThreatDetectionRadius = 1500.0f;

    /** How quickly threat level decays back to Calm (units/second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Threat")
    float ThreatDecayRate = 5.0f;

    /** Current accumulated threat score */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Threat")
    float CurrentThreatScore = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void RegisterThreat(FVector ThreatLocation, float ThreatMagnitude, bool bIsPlayer, FString StimulusTag);

    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void ClearThreat();

    UFUNCTION(BlueprintPure, Category = "NPC|Threat")
    bool IsUnderThreat() const;

    // ── Memory System ────────────────────────────────────────────────────────

    /** Short-term memory — recent stimuli (max 8 entries) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> ShortTermMemory;

    /** Long-term memory — persistent high-threat events (max 4 entries) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> LongTermMemory;

    /** How long (seconds) short-term memories persist */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float ShortTermMemoryDuration = 60.0f;

    /** Threat score threshold to commit to long-term memory */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float LongTermMemoryThreshold = 75.0f;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void AddMemoryEntry(FNPC_MemoryEntry Entry);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void PruneExpiredMemories(float CurrentTime);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasMemoryOfLocation(FVector Location, float Radius) const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FNPC_MemoryEntry GetMostRecentThreatMemory() const;

    // ── Social Behavior ──────────────────────────────────────────────────────

    /** Whether this NPC will alert nearby NPCs when threatened */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Social")
    bool bCanAlertOthers = true;

    /** Radius within which this NPC broadcasts alerts to others */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Social")
    float AlertBroadcastRadius = 2000.0f;

    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    void BroadcastAlert(FVector ThreatLocation, float ThreatMagnitude);

    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    void ReceiveAlert(FVector ThreatLocation, float ThreatMagnitude);

    // ── Patrol System ────────────────────────────────────────────────────────

    /** Patrol waypoints for this NPC */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> PatrolWaypoints;

    /** Current patrol waypoint index */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentPatrolIndex = 0;

    /** Acceptance radius to consider a waypoint reached */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaypointAcceptanceRadius = 150.0f;

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvancePatrolIndex();

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void GeneratePatrolWaypointsAround(FVector Center, float Radius, int32 Count);

private:
    void UpdateThreatDecay(float DeltaTime);
    void EvaluateThreatLevel();
    void SyncActivityWithThreat();

    float TimeSinceLastMemoryPrune = 0.0f;
    static constexpr float MemoryPruneInterval = 10.0f;
};
