#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// -----------------------------------------------------------------------
// Enums — NPC_prefix to avoid collision with other agents
// -----------------------------------------------------------------------

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Flee        UMETA(DisplayName = "Flee"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Rest        UMETA(DisplayName = "Rest"),
    Social      UMETA(DisplayName = "Social"),
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None    UMETA(DisplayName = "None"),
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Extreme UMETA(DisplayName = "Extreme"),
};

UENUM(BlueprintType)
enum class ENPC_DailyActivity : uint8
{
    Sleep   UMETA(DisplayName = "Sleep"),
    Forage  UMETA(DisplayName = "Forage"),
    Hunt    UMETA(DisplayName = "Hunt"),
    Guard   UMETA(DisplayName = "Guard"),
    Craft   UMETA(DisplayName = "Craft"),
    Socialize UMETA(DisplayName = "Socialize"),
    Scout   UMETA(DisplayName = "Scout"),
};

// -----------------------------------------------------------------------
// Memory entry — what this NPC remembers about a threat/event
// -----------------------------------------------------------------------

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    AActor* SourceActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = true;
};

// -----------------------------------------------------------------------
// Daily schedule entry
// -----------------------------------------------------------------------

USTRUCT(BlueprintType)
struct FNPC_ScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    float StartHour = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    float EndHour = 6.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    ENPC_DailyActivity Activity = ENPC_DailyActivity::Sleep;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    FVector TargetLocation = FVector::ZeroVector;
};

// -----------------------------------------------------------------------
// Main component
// -----------------------------------------------------------------------

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    // --- Lifecycle ---
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    float Alertness = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    float Stamina = 100.0f;

    // --- Patrol ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<AActor*> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolAcceptanceRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolWaitTime = 2.0f;

    // --- Perception ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float HearingRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float SightAngleDegrees = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float FleeThresholdFear = 70.0f;

    // --- Memory ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> ThreatMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 8;

    // --- Daily Schedule ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    TArray<FNPC_ScheduleEntry> DailySchedule;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Schedule")
    ENPC_DailyActivity CurrentActivity = ENPC_DailyActivity::Guard;

    // --- Functions ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(AActor* ThreatActor, float ThreatScore, FVector LastKnownLoc);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void UpdateBehaviorState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AdvancePatrolWaypoint();

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    FVector GetNextPatrolLocation() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool HasActiveThreat() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    AActor* GetHighestThreatActor() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Schedule")
    void UpdateDailyActivity(float CurrentGameHour);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnHearNoise(AActor* NoiseSource, float Loudness);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnSeeActor(AActor* SeenActor);

private:
    float PatrolWaitTimer = 0.0f;
    bool bIsWaitingAtWaypoint = false;

    void DecayMemory(float DeltaTime);
    void UpdateFearLevel(float DeltaTime);
    void EvaluateThreatLevel();

    ENPC_BehaviorState DetermineStateFromContext() const;
};
