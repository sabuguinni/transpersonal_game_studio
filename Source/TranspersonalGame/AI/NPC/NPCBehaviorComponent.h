#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Rest        UMETA(DisplayName = "Rest"),
    Alert       UMETA(DisplayName = "Alert"),
    Flee        UMETA(DisplayName = "Flee"),
    Attack      UMETA(DisplayName = "Attack"),
    Socialize   UMETA(DisplayName = "Socialize"),
    Seek        UMETA(DisplayName = "Seek")
};

UENUM(BlueprintType)
enum class ENPC_DangerLevel : uint8
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
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_DangerLevel DangerLevel = ENPC_DangerLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FString ThreatTag = TEXT("");

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutineSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float TimeOfDay = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_BehaviorState TargetState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float Duration = 60.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
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
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

    // --- Threat Detection ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(FVector ThreatLocation, ENPC_DangerLevel Danger, const FString& ThreatTag);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearThreat();

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool HasActiveThreat() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    // --- Memory System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void AddMemoryEntry(FVector Location, ENPC_DangerLevel Danger, const FString& Tag);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> GetRecentMemories(float WithinSeconds) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void PurgeOldMemories(float OlderThanSeconds);

    // --- Daily Routine ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void AddRoutineSlot(float TimeOfDay, ENPC_BehaviorState State, FVector Location, float Duration);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void EvaluateRoutine(float CurrentTimeOfDay);

    // --- Patrol ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolCenter(FVector Center, float Radius);

    UFUNCTION(BlueprintPure, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint() const;

    // --- Survival Stats ---
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Stats")
    float Hunger = 100.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Stats")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Stats")
    float Energy = 100.0f;

    // --- Detection Ranges ---
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Detection")
    float ThreatDetectionRadius = 3000.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Detection")
    float FleeRadius = 1500.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Detection")
    float AttackRadius = 200.0f;

    // --- Camp Reference ---
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Patrol")
    FVector CampLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|Patrol")
    float PatrolRadius = 800.0f;

private:
    UPROPERTY()
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY()
    ENPC_DangerLevel CurrentDangerLevel = ENPC_DangerLevel::None;

    UPROPERTY()
    TArray<FNPC_MemoryEntry> MemoryLog;

    UPROPERTY()
    TArray<FNPC_DailyRoutineSlot> DailyRoutine;

    UPROPERTY()
    FVector PatrolCenter = FVector::ZeroVector;

    UPROPERTY()
    int32 CurrentPatrolIndex = 0;

    float StateTimer = 0.0f;
    float MemoryDecayTimer = 0.0f;

    void TickSurvivalStats(float DeltaTime);
    void TickStateLogic(float DeltaTime);
    void TickMemoryDecay(float DeltaTime);
};
