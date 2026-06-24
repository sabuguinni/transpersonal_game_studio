#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Flee        UMETA(DisplayName = "Flee"),
    Seek        UMETA(DisplayName = "Seek Shelter"),
    Social      UMETA(DisplayName = "Social"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector ThreatLocation;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatTimestamp;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FString ThreatActorName;

    FNPC_MemoryEntry()
        : ThreatLocation(FVector::ZeroVector)
        , ThreatTimestamp(0.f)
        , ThreatLevel(ENPC_ThreatLevel::None)
        , ThreatActorName(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutineSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float TimeOfDayHour;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_BehaviorState TargetState;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    FVector TargetLocation;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    FString ActivityDescription;

    FNPC_DailyRoutineSlot()
        : TimeOfDayHour(0.f)
        , TargetState(ENPC_BehaviorState::Idle)
        , TargetLocation(FVector::ZeroVector)
        , ActivityDescription(TEXT(""))
    {}
};

/**
 * NPCBehaviorComponent — drives NPC daily routines, memory, and threat response.
 * Attached to any NPC actor. Integrates with UE5 Behavior Tree via blackboard.
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent), DisplayName="NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    // --- Threat Detection ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(AActor* ThreatActor, ENPC_ThreatLevel Level);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearThreatMemory();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    // --- Memory ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> GetMemoryEntries() const { return MemoryLog; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void PruneOldMemories(float MaxAgeSeconds);

    // --- Daily Routine ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void AddRoutineSlot(FNPC_DailyRoutineSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void EvaluateDailyRoutine(float CurrentHour);

    // --- Flee ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    FVector GetNearestShelterLocation() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AlertNearbyNPCs(float AlertRadius);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float AlertBroadcastRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float MemoryRetentionSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PatrolRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_DailyRoutineSlot> DailyRoutine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    TArray<FVector> ShelterLocations;

private:
    float StateTimer;
    FVector PatrolOrigin;

    void UpdateThreatFromMemory();
    void TickPatrol(float DeltaTime);
    void TickFlee(float DeltaTime);
    void TickAlert(float DeltaTime);
};
