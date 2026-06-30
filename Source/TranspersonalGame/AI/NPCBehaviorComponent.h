#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// ENUMS — all prefixed NPC_ to avoid global name collisions
// ============================================================

UENUM(BlueprintType)
enum class ENPC_AlertLevel : uint8
{
    Unaware      UMETA(DisplayName = "Unaware"),
    Curious      UMETA(DisplayName = "Curious"),
    Suspicious   UMETA(DisplayName = "Suspicious"),
    Alarmed      UMETA(DisplayName = "Alarmed"),
    Panicked     UMETA(DisplayName = "Panicked")
};

UENUM(BlueprintType)
enum class ENPC_RoutinePhase : uint8
{
    Sleeping      UMETA(DisplayName = "Sleeping"),
    Waking        UMETA(DisplayName = "Waking"),
    Foraging      UMETA(DisplayName = "Foraging"),
    Resting       UMETA(DisplayName = "Resting"),
    Socializing   UMETA(DisplayName = "Socializing"),
    Fleeing       UMETA(DisplayName = "Fleeing"),
    Investigating UMETA(DisplayName = "Investigating"),
    Patrolling    UMETA(DisplayName = "Patrolling")
};

UENUM(BlueprintType)
enum class ENPC_Relationship : uint8
{
    Unknown    UMETA(DisplayName = "Unknown"),
    Neutral    UMETA(DisplayName = "Neutral"),
    Friendly   UMETA(DisplayName = "Friendly"),
    Hostile    UMETA(DisplayName = "Hostile"),
    Feared     UMETA(DisplayName = "Feared"),
    Trusted    UMETA(DisplayName = "Trusted")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC Memory")
    FVector EventLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC Memory")
    bool bWasPlayerInvolved = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC Memory")
    FString EventDescription;
};

USTRUCT(BlueprintType)
struct FNPC_DailyScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC Schedule")
    ENPC_RoutinePhase Phase = ENPC_RoutinePhase::Foraging;

    UPROPERTY(BlueprintReadWrite, Category = "NPC Schedule")
    float StartHour = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC Schedule")
    float EndHour = 6.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC Schedule")
    FVector TargetLocation = FVector::ZeroVector;
};

// ============================================================
// COMPONENT
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Alert System ----
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetAlertLevel(ENPC_AlertLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RegisterThreatEvent(FVector ThreatLocation, float ThreatMagnitude, bool bPlayerCaused);

    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    ENPC_AlertLevel GetAlertLevel() const { return CurrentAlertLevel; }

    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    float GetAlertDecayRate() const { return AlertDecayRate; }

    // ---- Routine System ----
    UFUNCTION(BlueprintCallable, Category = "NPC Routine")
    void AdvanceRoutinePhase();

    UFUNCTION(BlueprintCallable, Category = "NPC Routine")
    void SetRoutinePhase(ENPC_RoutinePhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "NPC Routine")
    ENPC_RoutinePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "NPC Routine")
    FVector GetCurrentTargetLocation() const { return CurrentTargetLocation; }

    // ---- Memory System ----
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemoryEntry(FVector Location, float ThreatLevel, bool bPlayerInvolved, const FString& Description);

    UFUNCTION(BlueprintPure, Category = "NPC Memory")
    bool HasRecentThreatMemory(float WithinSeconds) const;

    UFUNCTION(BlueprintPure, Category = "NPC Memory")
    FNPC_MemoryEntry GetMostRecentMemory() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void PurgeOldMemories(float OlderThanSeconds);

    // ---- Relationship System ----
    UFUNCTION(BlueprintCallable, Category = "NPC Relationship")
    void SetRelationshipToPlayer(ENPC_Relationship NewRelationship);

    UFUNCTION(BlueprintPure, Category = "NPC Relationship")
    ENPC_Relationship GetRelationshipToPlayer() const { return PlayerRelationship; }

    UFUNCTION(BlueprintCallable, Category = "NPC Relationship")
    void ModifyRelationshipScore(float Delta);

    UFUNCTION(BlueprintPure, Category = "NPC Relationship")
    float GetRelationshipScore() const { return RelationshipScore; }

    // ---- Schedule ----
    UFUNCTION(BlueprintCallable, Category = "NPC Schedule")
    void SetDailySchedule(const TArray<FNPC_DailyScheduleEntry>& NewSchedule);

    UFUNCTION(BlueprintCallable, Category = "NPC Schedule")
    void EvaluateScheduleForHour(float CurrentHour);

    // ---- Fear & Threat ----
    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    float GetFearLevel() const { return FearLevel; }

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void DecayFear(float DeltaTime);

protected:
    // Alert state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior", meta = (AllowPrivateAccess = "true"))
    ENPC_AlertLevel CurrentAlertLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AlertDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AlertDecayDelay = 10.0f;

    float TimeSinceLastThreat = 0.0f;

    // Routine
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Routine", meta = (AllowPrivateAccess = "true"))
    ENPC_RoutinePhase CurrentPhase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Routine", meta = (AllowPrivateAccess = "true"))
    FVector CurrentTargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    TArray<FNPC_DailyScheduleEntry> DailySchedule;

    // Memory
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Memory", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_MemoryEntry> MemoryLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxMemoryEntries = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float MemoryRetentionSeconds = 300.0f;

    // Relationship
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Relationship", meta = (AllowPrivateAccess = "true"))
    ENPC_Relationship PlayerRelationship;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Relationship", meta = (AllowPrivateAccess = "true"))
    float RelationshipScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Relationship")
    float RelationshipScoreMin = -100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Relationship")
    float RelationshipScoreMax = 100.0f;

    // Fear
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior", meta = (AllowPrivateAccess = "true"))
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float FearDecayRate = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float FearPanicThreshold = 80.0f;

private:
    void UpdateAlertDecay(float DeltaTime);
    ENPC_Relationship ScoreToRelationship(float Score) const;
};
