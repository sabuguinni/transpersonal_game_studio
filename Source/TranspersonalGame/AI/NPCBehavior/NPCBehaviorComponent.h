// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260626_004
// Manages NPC daily routines, memory, and behavioral state for prehistoric survival NPCs
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Sheltering  UMETA(DisplayName = "Sheltering"),
    Alerting    UMETA(DisplayName = "Alerting"),
    Resting     UMETA(DisplayName = "Resting"),
    Hunting     UMETA(DisplayName = "Hunting")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None    UMETA(DisplayName = "None"),
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Panic   UMETA(DisplayName = "Panic")
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector ThreatLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FString ThreatType;

    FNPC_MemoryEntry()
        : ThreatLocation(FVector::ZeroVector)
        , ThreatIntensity(0.0f)
        , TimeStamp(0.0f)
        , ThreatType(TEXT("Unknown"))
    {}
};

USTRUCT(BlueprintType)
struct FNPC_WaypointData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FString ActivityAtPoint;

    FNPC_WaypointData()
        : Location(FVector::ZeroVector)
        , WaitDuration(3.0f)
        , ActivityAtPoint(TEXT("Idle"))
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State Management ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    // --- Threat Response ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void RegisterThreat(FVector ThreatLoc, float Intensity, const FString& ThreatType);

    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void ClearThreatMemory();

    UFUNCTION(BlueprintPure, Category = "NPC|Threat")
    ENPC_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    // --- Patrol System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolWaypoints(const TArray<FNPC_WaypointData>& Waypoints);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextWaypointLocation();

    UFUNCTION(BlueprintPure, Category = "NPC|Patrol")
    bool HasReachedCurrentWaypoint() const;

    // --- Memory System ---
    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> GetMemoryEntries() const { return MemoryEntries; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void PurgeOldMemories(float MaxAge);

    // --- Daily Routine ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void UpdateDailyRoutine(float GameTimeOfDay);

    // --- Alert System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void AlertNearbyNPCs(float AlertRadius);

    // --- Properties ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ThreatDetectionRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_ThreatLevel CurrentThreatLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_WaypointData> PatrolWaypoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryEntries;

private:
    float TimeSinceLastStateChange;
    float AccumulatedThreatIntensity;

    void EvaluateThreatLevel();
    void TickPatrolLogic(float DeltaTime);
    void TickFleeLogic(float DeltaTime);
    void DecayMemories(float DeltaTime);
};
