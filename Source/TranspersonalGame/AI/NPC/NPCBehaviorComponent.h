#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// NPC Role Enum — defines the primary function of each NPC in the tribe
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ENPC_Role : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Guard       UMETA(DisplayName = "Guard"),
};

// ─────────────────────────────────────────────────────────────────────────────
// NPC Daily Phase — time-of-day driven behavior slot
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ENPC_DailyPhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Midday      UMETA(DisplayName = "Midday"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
};

// ─────────────────────────────────────────────────────────────────────────────
// NPC Activity — what the NPC is doing during a given phase
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ENPC_Activity : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Craft       UMETA(DisplayName = "Craft"),
    Sleep       UMETA(DisplayName = "Sleep"),
    Watch       UMETA(DisplayName = "Watch"),
    Flee        UMETA(DisplayName = "Flee"),
    Alert       UMETA(DisplayName = "Alert"),
    Converse    UMETA(DisplayName = "Converse"),
};

// ─────────────────────────────────────────────────────────────────────────────
// NPC Memory Entry — a single remembered event
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector EventLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float EventTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;   // 0.0 = safe, 1.0 = lethal

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FString EventTag;           // e.g. "TRex_Sighted", "Player_Hostile"

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = true;
};

// ─────────────────────────────────────────────────────────────────────────────
// NPC Daily Routine Slot — one phase → activity mapping
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FNPC_RoutineSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_DailyPhase Phase = ENPC_DailyPhase::Dawn;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_Activity Activity = ENPC_Activity::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float Duration = 300.0f;    // seconds
};

// ─────────────────────────────────────────────────────────────────────────────
// UNPCBehaviorComponent — drives NPC daily routine, memory, and reactions
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Identity ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    FString NPCName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    ENPC_Role Role = ENPC_Role::Gatherer;

    // ── Routine ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_RoutineSlot> DailyRoutine;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Routine")
    ENPC_DailyPhase CurrentPhase = ENPC_DailyPhase::Dawn;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Routine")
    ENPC_Activity CurrentActivity = ENPC_Activity::Idle;

    // ── Memory ────────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayTime = 600.0f;     // seconds until memory fades

    // ── Perception Radii ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float AlertRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float FleeRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float MemoryRadius = 5000.0f;

    // ── Reaction State ────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    bool bIsAlerting = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    bool bIsFleeing = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float FearLevel = 0.0f;     // 0.0 = calm, 1.0 = panic

    // ── Public API ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnThreatDetected(FVector ThreatLocation, float ThreatLevel, const FString& ThreatTag);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetDailyPhase(ENPC_DailyPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_Activity GetActivityForPhase(ENPC_DailyPhase Phase) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RecordMemory(FVector Location, float Time, float Threat, const FString& Tag);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void DecayMemories(float CurrentTime);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool HasMemoryOfThreat(const FString& ThreatTag) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    float GetHighestThreatInMemory() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void BuildDefaultRoutine();

private:
    float AccumulatedTime = 0.0f;
    float LastMemoryDecayTime = 0.0f;
};
