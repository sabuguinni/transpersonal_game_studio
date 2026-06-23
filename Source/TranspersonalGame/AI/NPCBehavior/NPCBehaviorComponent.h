// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Modular NPC behavior: patrol, awareness, daily routines, memory
// Attaches to any AActor to give it NPC behavior capabilities

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// NPC behavior states — drives the behavior tree
UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Interacting     UMETA(DisplayName = "Interacting"),
    Resting         UMETA(DisplayName = "Resting"),
    Dead            UMETA(DisplayName = "Dead")
};

// NPC archetype — determines default behavior profile
UENUM(BlueprintType)
enum class ENPC_Archetype : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Guard           UMETA(DisplayName = "Guard"),
    Child           UMETA(DisplayName = "Child"),
    Wounded         UMETA(DisplayName = "Wounded")
};

// Memory entry — what this NPC remembers about an event or entity
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;   // 0=neutral, 1=max threat

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FString EventTag;           // "PlayerSeen", "DinoHeard", "AllyDied"

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = true;
};

// Patrol waypoint data
USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration = 2.0f;  // seconds to wait at this point

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLookAround = true;    // rotate to scan area when waiting
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === ARCHETYPE & STATE ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    ENPC_Archetype Archetype = ENPC_Archetype::Gatherer;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    // === PATROL SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLoopPatrol = true;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentPatrolIndex = 0;

    // === AWARENESS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Awareness")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Awareness")
    float HearingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Awareness")
    float SightAngleDegrees = 90.0f;    // cone half-angle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Awareness")
    float ThreatResponseThreshold = 0.5f;   // flee if threat >= this

    // === MEMORY ===

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayTime = 30.0f;  // seconds before memory fades

    // === DAILY ROUTINE ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float DawnActivityHour = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float DuskRestHour = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    bool bRestAtNight = true;

    // === SOCIAL ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Social")
    float AlertAlliesRadius = 500.0f;   // radius to warn nearby NPCs

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Social")
    bool bCanAlertAllies = true;

    // === PUBLIC API ===

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AddMemoryEntry(FVector Location, float ThreatLevel, const FString& EventTag);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearExpiredMemories();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    float GetHighestThreatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool IsPlayerVisible() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvancePatrolIndex();

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetCurrentPatrolTarget() const;

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

private:
    float StateTimer = 0.0f;
    float MemoryTickAccumulator = 0.0f;
    float WaitAtWaypointTimer = 0.0f;
    bool bWaitingAtWaypoint = false;

    void TickPatrol(float DeltaTime);
    void TickAwareness(float DeltaTime);
    void TickMemoryDecay(float DeltaTime);
    void ApplyArchetypeDefaults();
};
