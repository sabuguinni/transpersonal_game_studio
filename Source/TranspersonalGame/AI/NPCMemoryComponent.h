#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCMemoryComponent.generated.h"

/** Types of danger events an NPC can remember */
UENUM(BlueprintType)
enum class ENPC_MemoryEventType : uint8
{
    None            UMETA(DisplayName = "None"),
    DinosaurSighted UMETA(DisplayName = "Dinosaur Sighted"),
    PlayerSpotted   UMETA(DisplayName = "Player Spotted"),
    AttackReceived  UMETA(DisplayName = "Attack Received"),
    AllyKilled      UMETA(DisplayName = "Ally Killed"),
    SafeZoneFound   UMETA(DisplayName = "Safe Zone Found"),
};

/** A single memory record stored by an NPC */
USTRUCT(BlueprintType)
struct FNPC_MemoryRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_MemoryEventType EventType = ENPC_MemoryEventType::None;

    /** World location where the event occurred */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector EventLocation = FVector::ZeroVector;

    /** Game time (seconds) when the event was recorded */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float RecordedAtTime = 0.f;

    /** 0-1 threat weight — decays over time */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatWeight = 1.f;

    /** Tag of the actor that caused the event (e.g. "TRex", "Raptor") */
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FName SourceTag = NAME_None;
};

/**
 * UNPCMemoryComponent
 * Gives any NPC pawn a persistent, decaying memory of danger events.
 * Behavior Tree tasks query this component to decide patrol routes,
 * flee directions, and alert states.
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Memory Component")
class TRANSPERSONALGAME_API UNPCMemoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCMemoryComponent();

    // ── Configuration ────────────────────────────────────────────────────────

    /** Maximum number of memories retained at once */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemories = 16;

    /** Seconds until a memory's ThreatWeight decays to zero */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayDuration = 120.f;

    /** Radius within which two events at the same location are merged */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MergeRadius = 300.f;

    // ── Runtime state ────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryRecord> Memories;

    /** Overall threat level (0-1) derived from all active memories */
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float CurrentThreatLevel = 0.f;

    // ── Interface ────────────────────────────────────────────────────────────

    /** Record a new danger event. Merges with nearby existing memory if within MergeRadius. */
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordEvent(ENPC_MemoryEventType EventType, FVector Location, FName SourceTag, float ThreatWeight = 1.f);

    /** Returns the most threatening memory location, or ZeroVector if none. */
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    FVector GetMostThreateningLocation() const;

    /** Returns the safest known location (lowest threat weight), or ZeroVector if none. */
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    FVector GetSafestKnownLocation() const;

    /** True if any memory of this event type exists with ThreatWeight > Threshold. */
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    bool HasActiveMemoryOf(ENPC_MemoryEventType EventType, float Threshold = 0.1f) const;

    /** Forget all memories (e.g. after returning to camp safely). */
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ClearAllMemories();

    // ── UActorComponent overrides ─────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void DecayMemories(float DeltaTime);
    void PruneExpiredMemories();
};
