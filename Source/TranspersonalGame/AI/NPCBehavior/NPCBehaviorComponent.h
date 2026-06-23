// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Manages NPC daily routines, memory, and social reactions for prehistoric survival NPCs

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// NPC daily routine states — what the NPC is currently doing
UENUM(BlueprintType)
enum class ENPC_RoutineState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Resting         UMETA(DisplayName = "Resting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Alerted         UMETA(DisplayName = "Alerted"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Investigating   UMETA(DisplayName = "Investigating")
};

// NPC social role within the tribe
UENUM(BlueprintType)
enum class ENPC_TribeRole : uint8
{
    Scout       UMETA(DisplayName = "Scout"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Elder       UMETA(DisplayName = "Elder"),
    Guard       UMETA(DisplayName = "Guard"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Child       UMETA(DisplayName = "Child")
};

// Memory entry: what the NPC remembers about a specific event or entity
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector EventLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float EventTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f; // 0=neutral, 1=low threat, 10=extreme danger

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FString EventTag; // "Raptor", "TRex", "Player", "Fire", "Food"

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = true;
};

// NPC survival needs — drives behavior decisions
USTRUCT(BlueprintType)
struct FNPC_SurvivalNeeds
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Needs")
    float Hunger = 100.0f; // 0=starving, 100=full

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Needs")
    float Thirst = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Needs")
    float Fear = 0.0f; // 0=calm, 100=panicking

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Needs")
    float Energy = 100.0f; // 0=exhausted, 100=rested

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Needs")
    float Safety = 100.0f; // 0=in danger, 100=fully safe
};

UCLASS(ClassGroup = (NPC), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    // --- Core State ---
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Identity")
    ENPC_TribeRole TribeRole = ENPC_TribeRole::Gatherer;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Identity")
    FString NPCName = "Unnamed";

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_RoutineState CurrentRoutine = ENPC_RoutineState::Idle;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|State")
    FNPC_SurvivalNeeds Needs;

    // --- Memory System ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryLog;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 20;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Memory")
    float MemoryDecayRate = 0.1f; // Threat level decays per second

    // --- Patrol & Movement ---
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Patrol")
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Patrol")
    float PatrolRadius = 2000.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Patrol")
    float WaypointAcceptanceRadius = 150.0f;

    // --- Threat Detection ---
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Threat")
    float DinosaurDetectionRadius = 3000.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Threat")
    float PlayerDetectionRadius = 1500.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Threat")
    float FleeThreshold = 60.0f; // Fear level at which NPC flees

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Threat")
    float AlertThreshold = 30.0f; // Fear level at which NPC becomes alerted

    // --- Social Behavior ---
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Social")
    bool bWillShareInformation = true; // Will warn nearby NPCs of threats

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Social")
    float InformationShareRadius = 1200.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Social")
    float PlayerRelationship = 50.0f; // 0=hostile, 50=neutral, 100=friendly

    // --- Blueprint-callable functions ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void AddMemoryEntry(FVector Location, float ThreatLevel, const FString& Tag);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    float GetHighestThreatInMemory() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void PruneOldMemories();

    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    void SetRoutineState(ENPC_RoutineState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    ENPC_RoutineState EvaluatePriorityRoutine() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Needs")
    void TickNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    void AlertNearbyNPCs(FVector ThreatLocation, float ThreatLevel, const FString& ThreatTag);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvanceWaypoint();

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsFleeing() const { return CurrentRoutine == ENPC_RoutineState::Fleeing; }

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsAlerted() const { return CurrentRoutine == ENPC_RoutineState::Alerted; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float TimeSinceLastNeedsTick = 0.0f;
    float NeedsTickInterval = 5.0f; // Update needs every 5 seconds
};
