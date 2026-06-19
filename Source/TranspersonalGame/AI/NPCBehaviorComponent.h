#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Alert       UMETA(DisplayName = "Alert"),
    Flee        UMETA(DisplayName = "Flee"),
    Attack      UMETA(DisplayName = "Attack"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None    UMETA(DisplayName = "None"),
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Extreme UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceLastSeen;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsPlayerTarget;

    FNPC_MemoryEntry()
        : LastKnownLocation(FVector::ZeroVector)
        , TimeSinceLastSeen(0.0f)
        , ThreatLevel(ENPC_ThreatLevel::None)
        , bIsPlayerTarget(false)
    {}
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutineSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float TimeOfDay;  // 0.0 = midnight, 12.0 = noon, 24.0 = midnight

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    FVector TargetLocation;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_BehaviorState RoutineState;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float Duration;  // seconds to spend at this location

    FNPC_DailyRoutineSlot()
        : TimeOfDay(0.0f)
        , TargetLocation(FVector::ZeroVector)
        , RoutineState(ENPC_BehaviorState::Idle)
        , Duration(30.0f)
    {}
};

/**
 * NPCBehaviorComponent — drives NPC state machine, memory, and daily routines.
 * Attach to any APawn or ACharacter to give it autonomous behavior.
 * Agent #11 — NPC Behavior Agent
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State Machine ---

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnThreatDetected(AActor* ThreatActor, ENPC_ThreatLevel Level);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnThreatLost();

    // --- Memory System ---

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void UpdateMemory(AActor* Target, FVector Location, ENPC_ThreatLevel Threat);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasMemoryOfTarget(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FVector GetLastKnownLocation(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetAllMemories();

    // --- Daily Routine ---

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void AddRoutineSlot(float TimeOfDay, FVector Location, ENPC_BehaviorState State, float Duration);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void EvaluateRoutine(float CurrentTimeOfDay);

    // --- Patrol ---

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolWaypoints(const TArray<FVector>& Waypoints);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintPure, Category = "NPC|Patrol")
    bool HasPatrolRoute() const { return PatrolWaypoints.Num() > 0; }

    // --- Sensing ---

    UFUNCTION(BlueprintCallable, Category = "NPC|Sensing")
    void SetSightRadius(float Radius) { SightRadius = Radius; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Sensing")
    void SetHearingRadius(float Radius) { HearingRadius = Radius; }

    UFUNCTION(BlueprintPure, Category = "NPC|Sensing")
    float GetSightRadius() const { return SightRadius; }

    UFUNCTION(BlueprintPure, Category = "NPC|Sensing")
    float GetHearingRadius() const { return HearingRadius; }

    // --- Properties ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float SightRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDecayRate = 0.1f;  // threat level reduction per second when target not visible

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertDuration = 10.0f;  // seconds to stay alert after losing sight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    bool bUseDailyRoutine = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    bool bIsAggressive = false;  // if true, attacks on sight; if false, flees

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    float TimeInCurrentState;

private:
    TMap<TWeakObjectPtr<AActor>, FNPC_MemoryEntry> MemoryMap;

    TArray<FNPC_DailyRoutineSlot> DailyRoutine;

    TArray<FVector> PatrolWaypoints;
    int32 CurrentPatrolIndex;

    float AlertTimer;

    void TickMemoryDecay(float DeltaTime);
    void TickAlertTimer(float DeltaTime);
    void TransitionToState(ENPC_BehaviorState NewState);
};
