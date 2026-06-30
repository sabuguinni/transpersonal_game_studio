#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// NPC Behavior Component — Agent #11
// Drives daily routines, threat response, memory, and social
// relations for all non-player characters in the world.
// ============================================================

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle          UMETA(DisplayName = "Idle"),
    Patrol        UMETA(DisplayName = "Patrol"),
    Gather        UMETA(DisplayName = "Gather"),
    Flee          UMETA(DisplayName = "Flee"),
    Seek          UMETA(DisplayName = "Seek"),
    Investigate   UMETA(DisplayName = "Investigate"),
    Interact      UMETA(DisplayName = "Interact"),
    Dead          UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None     UMETA(DisplayName = "None"),
    Low      UMETA(DisplayName = "Low"),
    Medium   UMETA(DisplayName = "Medium"),
    High     UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ENPC_TimeOfDay : uint8
{
    Dawn    UMETA(DisplayName = "Dawn"),
    Morning UMETA(DisplayName = "Morning"),
    Midday  UMETA(DisplayName = "Midday"),
    Dusk    UMETA(DisplayName = "Dusk"),
    Night   UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ENPC_SocialRelation : uint8
{
    Stranger  UMETA(DisplayName = "Stranger"),
    Neutral   UMETA(DisplayName = "Neutral"),
    Friendly  UMETA(DisplayName = "Friendly"),
    Hostile   UMETA(DisplayName = "Hostile"),
    Ally      UMETA(DisplayName = "Ally")
};

// A single memory entry — what this NPC remembers about an event
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
    bool bIsDangerous = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;
};

// Daily schedule slot — what this NPC does at a given time of day
USTRUCT(BlueprintType)
struct FNPC_ScheduleSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    ENPC_TimeOfDay TimeOfDay = ENPC_TimeOfDay::Morning;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    ENPC_BehaviorState DesiredState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    float Duration = 60.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    // ---- Lifecycle ----
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- State ----
    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    void EvaluateThreat(AActor* ThreatSource, float ThreatScore);

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    ENPC_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    // ---- Memory ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void AddMemory(AActor* Source, FVector Location, float Threat, bool bDangerous);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetOldMemories(float MaxAge);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasMemoryOf(AActor* Source) const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FNPC_MemoryEntry GetMostDangerousMemory() const;

    // ---- Schedule ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Schedule")
    void AddScheduleSlot(FNPC_ScheduleSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "NPC|Schedule")
    void EvaluateSchedule(ENPC_TimeOfDay CurrentTimeOfDay);

    // ---- Social ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    void SetRelationToPlayer(ENPC_SocialRelation Relation);

    UFUNCTION(BlueprintPure, Category = "NPC|Social")
    ENPC_SocialRelation GetRelationToPlayer() const { return RelationToPlayer; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    void ModifyFear(float Delta);

    UFUNCTION(BlueprintPure, Category = "NPC|Social")
    float GetFearLevel() const { return FearLevel; }

    // ---- Patrol ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintPure, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvancePatrolIndex();

protected:
    // ---- Internal helpers ----
    void UpdateThreatDecay(float DeltaTime);
    void UpdateFearDecay(float DeltaTime);
    ENPC_ThreatLevel ScoreToThreatLevel(float Score) const;

public:
    // ---- Exposed properties ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ThreatDecayRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FearDecayRate = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryMaxAge = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolAcceptRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeSpeedMultiplier = 1.5f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Social", meta = (AllowPrivateAccess = "true"))
    ENPC_SocialRelation RelationToPlayer = ENPC_SocialRelation::Neutral;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Social", meta = (AllowPrivateAccess = "true"))
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    float CurrentThreatScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryEntries;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Schedule")
    TArray<FNPC_ScheduleSlot> DailySchedule;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentPatrolIndex = 0;
};
