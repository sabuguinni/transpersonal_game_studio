// NPCBehaviorComponent.h
// Agent #11 — NPC Behavior Agent | PROD_CYCLE_AUTO_20260627_009
// Drives NPC daily routines, threat awareness, memory, and social reactions.
// Attached to any AActor that represents a tribal human NPC.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Patrolling   UMETA(DisplayName = "Patrolling"),
    Foraging     UMETA(DisplayName = "Foraging"),
    Resting      UMETA(DisplayName = "Resting"),
    Socializing  UMETA(DisplayName = "Socializing"),
    Alerted      UMETA(DisplayName = "Alerted"),
    Fleeing      UMETA(DisplayName = "Fleeing"),
    Hiding       UMETA(DisplayName = "Hiding"),
    Dead         UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None     UMETA(DisplayName = "None"),
    Low      UMETA(DisplayName = "Low"),
    Medium   UMETA(DisplayName = "Medium"),
    High     UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ENPC_DailyActivity : uint8
{
    Sleep    UMETA(DisplayName = "Sleep"),
    Wake     UMETA(DisplayName = "Wake"),
    Forage   UMETA(DisplayName = "Forage"),
    Work     UMETA(DisplayName = "Work"),
    Socialize UMETA(DisplayName = "Socialize"),
    Guard    UMETA(DisplayName = "Guard"),
    Eat      UMETA(DisplayName = "Eat"),
    Rest     UMETA(DisplayName = "Rest")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_ThreatMemory
{
    GENERATED_BODY()

    /** World location where threat was last seen */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    /** Actor that posed the threat (dinosaur, predator) */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    AActor* ThreatActor = nullptr;

    /** How long ago the threat was seen (seconds) */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceSeen = 0.0f;

    /** Estimated threat severity 0-1 */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatSeverity = 0.0f;

    /** Whether the NPC has alerted others about this threat */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bHasAlertedOthers = false;
};

USTRUCT(BlueprintType)
struct FNPC_DailyScheduleEntry
{
    GENERATED_BODY()

    /** Hour of day (0-23) when this activity begins */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    int32 StartHour = 6;

    /** Activity to perform at this time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    ENPC_DailyActivity Activity = ENPC_DailyActivity::Forage;

    /** Optional target location for this activity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    FVector TargetLocation = FVector::ZeroVector;

    /** Duration in hours */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    float DurationHours = 2.0f;
};

// ─── Component ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ─── State ───────────────────────────────────────────────────────────

    /** Current high-level behavior state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    /** Current perceived threat level */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    /** Current activity from daily schedule */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_DailyActivity CurrentActivity = ENPC_DailyActivity::Wake;

    /** Fear value 0-1 — drives animation posture (feeds AnimInstance) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    float FearLevel = 0.0f;

    /** Alertness 0-1 — how aware the NPC is of surroundings */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    float AlertnessLevel = 0.0f;

    // ─── Configuration ───────────────────────────────────────────────────

    /** Radius within which this NPC detects threats (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ThreatDetectionRadius = 2000.0f;

    /** Radius within which NPC hears sounds (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float HearingRadius = 1500.0f;

    /** Distance at which NPC will flee from a threat (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeDistance = 3000.0f;

    /** How quickly fear decays when no threat is present (per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FearDecayRate = 0.05f;

    /** How quickly alertness decays when no threat is present (per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertnessDecayRate = 0.1f;

    /** Daily schedule — list of time-based activities */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    TArray<FNPC_DailyScheduleEntry> DailySchedule;

    /** Patrol waypoints this NPC walks between */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<AActor*> PatrolWaypoints;

    /** Index of current patrol waypoint */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentWaypointIndex = 0;

    // ─── Memory ──────────────────────────────────────────────────────────

    /** Short-term threat memory — what the NPC has recently seen */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_ThreatMemory> ThreatMemories;

    /** Maximum number of threat memories to retain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxThreatMemories = 5;

    /** How long (seconds) a threat memory persists before fading */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDuration = 60.0f;

    // ─── Public API ──────────────────────────────────────────────────────

    /** Called when a threat enters detection range */
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnThreatDetected(AActor* ThreatActor, float ThreatSeverity, FVector ThreatLocation);

    /** Called when the threat is no longer visible */
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnThreatLost(AActor* ThreatActor);

    /** Alert nearby NPCs about a threat (social propagation) */
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AlertNearbyNPCs(AActor* ThreatActor, float ThreatSeverity);

    /** Force a specific behavior state */
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    /** Get the next patrol waypoint location */
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolWaypointLocation();

    /** Advance to next waypoint in patrol loop */
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvancePatrolWaypoint();

    /** Evaluate what activity the NPC should be doing at the given hour */
    UFUNCTION(BlueprintCallable, Category = "NPC|Schedule")
    ENPC_DailyActivity GetScheduledActivityForHour(int32 Hour) const;

    /** Returns current fear level (0-1) for AnimInstance integration */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NPC|State")
    float GetFearLevel() const { return FearLevel; }

    /** Returns current alertness (0-1) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NPC|State")
    float GetAlertnessLevel() const { return AlertnessLevel; }

    /** Returns true if NPC is in a threat-reactive state */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NPC|State")
    bool IsInThreatState() const;

private:
    void UpdateFearAndAlertness(float DeltaTime);
    void UpdateThreatMemories(float DeltaTime);
    void EvaluateBehaviorState();
    void BuildDefaultDailySchedule();

    float TimeSinceLastThreatScan = 0.0f;
    static constexpr float ThreatScanInterval = 0.5f; // Scan every 0.5s for performance
};
