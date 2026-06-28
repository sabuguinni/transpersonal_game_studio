#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// ENPC_DailyPhase — 7 phases of the prehistoric day cycle
// ============================================================
UENUM(BlueprintType)
enum class ENPC_DailyPhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    DeepNight   UMETA(DisplayName = "DeepNight")
};

// ============================================================
// ENPC_AlertLevel — threat awareness state
// ============================================================
UENUM(BlueprintType)
enum class ENPC_AlertLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Curious     UMETA(DisplayName = "Curious"),
    Suspicious  UMETA(DisplayName = "Suspicious"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Fighting    UMETA(DisplayName = "Fighting")
};

// ============================================================
// ENPC_BehaviorState — primary NPC behavior state
// ============================================================
UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Rest        UMETA(DisplayName = "Rest"),
    Socialise   UMETA(DisplayName = "Socialise"),
    Flee        UMETA(DisplayName = "Flee"),
    Seek        UMETA(DisplayName = "Seek"),
    Combat      UMETA(DisplayName = "Combat"),
    Dead        UMETA(DisplayName = "Dead")
};

// ============================================================
// FNPC_MemoryEntry — a single remembered event
// ============================================================
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool bIsPlayerRelated = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FString EventDescription;
};

// ============================================================
// FNPC_PatrolRoute — ordered waypoint list
// ============================================================
USTRUCT(BlueprintType)
struct FNPC_PatrolRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLoopRoute = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaypointAcceptanceRadius = 150.0f;
};

// ============================================================
// UNPCBehaviorComponent — core NPC behavior driver
// Attach to any ACharacter or APawn to give it daily routines,
// memory, alert states, and patrol behaviour.
// ============================================================
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    // ---- Lifecycle ----
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ---- State queries ----
    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    ENPC_BehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    ENPC_AlertLevel GetAlertLevel() const { return AlertLevel; }

    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    ENPC_DailyPhase GetDailyPhase() const { return CurrentDailyPhase; }

    // ---- Alert system ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void RaiseAlert(float ThreatLevel, FVector ThreatLocation, bool bIsPlayer);

    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void ClearAlert();

    // ---- Memory system ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordMemory(FVector Location, float ThreatLevel, bool bIsPlayer, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    bool HasMemoryOfPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    float GetPlayerThreatMemory() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetOldMemories(float MaxAge);

    // ---- Patrol ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolRoute(const FNPC_PatrolRoute& NewRoute);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvancePatrolWaypoint();

    // ---- Daily phase ----
    UFUNCTION(BlueprintCallable, Category = "NPC|DailyCycle")
    void SetDailyPhase(ENPC_DailyPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "NPC|DailyCycle")
    ENPC_BehaviorState GetPreferredStateForPhase(ENPC_DailyPhase Phase) const;

    // ---- Social ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    void PropagateAlertToNearbyNPCs(float Radius);

    // ---- Exposed config ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryRetentionTime = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FightThreshold = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FNPC_PatrolRoute PatrolRoute;

private:
    UPROPERTY()
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY()
    ENPC_AlertLevel AlertLevel;

    UPROPERTY()
    ENPC_DailyPhase CurrentDailyPhase;

    UPROPERTY()
    float CurrentAlertValue;

    UPROPERTY()
    FVector LastKnownThreatLocation;

    UPROPERTY()
    TArray<FNPC_MemoryEntry> MemoryLog;

    UPROPERTY()
    float WorldTimeCache;

    void UpdateAlertDecay(float DeltaTime);
    void UpdateBehaviorStateFromAlert();
    void UpdateDailyPhaseFromTime();
    ENPC_AlertLevel AlertValueToLevel(float Value) const;
};
