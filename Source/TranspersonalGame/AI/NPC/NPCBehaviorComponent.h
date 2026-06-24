#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Interacting UMETA(DisplayName = "Interacting"),
};

UENUM(BlueprintType)
enum class ENPC_PersonalityType : uint8
{
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Sentinel    UMETA(DisplayName = "Sentinel"),
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceObserved;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsThreat;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FString ThreatTag;

    FNPC_MemoryEntry()
        : LastKnownLocation(FVector::ZeroVector)
        , TimeSinceObserved(0.f)
        , bIsThreat(false)
        , ThreatTag(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNPC_PatrolRoute
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> Waypoints;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    int32 CurrentWaypointIndex;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaypointAcceptanceRadius;

    FNPC_PatrolRoute()
        : CurrentWaypointIndex(0)
        , WaypointAcceptanceRadius(150.f)
    {}
};

/**
 * NPCBehaviorComponent — drives NPC daily routines, memory, and state machine.
 * Attached to any ACharacter-derived NPC. Works alongside UE5 BehaviorTree via
 * a companion AIController that reads CurrentState and MemoryEntries each tick.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    ENPC_PersonalityType PersonalityType;

    // --- Patrol ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FNPC_PatrolRoute PatrolRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float RunSpeed;

    // --- Perception ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float ThreatFleeRadius;

    // --- Memory ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(FVector ThreatLocation, FString ThreatTag);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AdvancePatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    FVector GetCurrentPatrolTarget() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool HasActiveThreatMemory() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    FVector GetLastKnownThreatLocation() const;

private:
    void TickMemoryDecay(float DeltaTime);
    void EvaluateStateTransitions();
};
