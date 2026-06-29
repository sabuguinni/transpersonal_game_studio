#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
//  Enums — MUST be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Gather      UMETA(DisplayName = "Gather Resources"),
    Flee        UMETA(DisplayName = "Flee from Threat"),
    Seek        UMETA(DisplayName = "Seek Target"),
    Investigate UMETA(DisplayName = "Investigate"),
    Interact    UMETA(DisplayName = "Interact with Object"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None     UMETA(DisplayName = "No Threat"),
    Low      UMETA(DisplayName = "Low Threat"),
    Medium   UMETA(DisplayName = "Medium Threat"),
    High     UMETA(DisplayName = "High Threat"),
    Critical UMETA(DisplayName = "Critical Threat")
};

// ============================================================
//  Structs — MUST be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    AActor* SourceActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FString ContextNote;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool bIsActive = false;
};

// ============================================================
//  Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPC_BehaviorStateDelegate,
    ENPC_BehaviorState, OldState, ENPC_BehaviorState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNPC_ThreatLevelDelegate,
    ENPC_ThreatLevel, NewThreatLevel);

// ============================================================
//  UNPCBehaviorComponent
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent),
    DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_BehaviorStateDelegate OnBehaviorStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_ThreatLevelDelegate OnThreatLevelChanged;

    // ---- State Queries ----
    UFUNCTION(BlueprintPure, Category = "NPC|State")
    ENPC_BehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    ENPC_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    float GetFearLevel() const { return CurrentFear; }

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    float GetHunger() const { return CurrentHunger; }

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsAlive() const { return bIsAlive; }

    // ---- External Control (called by BT Tasks / Animation Agent) ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Control")
    void SetFearLevel(float NewFear);

    UFUNCTION(BlueprintCallable, Category = "NPC|Control")
    void SetHunger(float NewHunger);

    UFUNCTION(BlueprintCallable, Category = "NPC|Control")
    void SetFatigue(float NewFatigue);

    UFUNCTION(BlueprintCallable, Category = "NPC|Control")
    void NotifyDeath();

    UFUNCTION(BlueprintCallable, Category = "NPC|Control")
    void ForceState(ENPC_BehaviorState ForcedState);

    // ---- Navigation Helpers ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Navigation")
    FVector GetNextPatrolPoint() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Navigation")
    FVector GetFleeDirection() const;

    // ---- Memory System ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void AddMemoryEntry(AActor* SourceActor, FVector Location,
        ENPC_ThreatLevel Threat, const FString& Context);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasMemoryOf(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FVector GetLastKnownLocationOf(AActor* Actor) const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    int32 GetMemoryCount() const { return MemoryEntries.Num(); }

    // ---- Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AttackRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    int32 MaxMemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    bool bCanFlee;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    bool bCanAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    bool bHasDailyRoutine;

    // ---- Daily Routine Hours ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|DailyRoutine",
        meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float WakeHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|DailyRoutine",
        meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float SleepHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|DailyRoutine",
        meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float GatherHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|DailyRoutine",
        meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float RestHour;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    // ---- Runtime State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    ENPC_ThreatLevel CurrentThreatLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    float CurrentFear;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    float CurrentHunger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    float CurrentFatigue;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    bool bIsAlive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    FVector HomeLocation;

    bool bHomeLocationSet;

    // ---- Memory ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory",
        meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_MemoryEntry> MemoryEntries;

    // ---- Timers ----
    FTimerHandle MemoryCleanupTimer;
    FTimerHandle DailyRoutineTimer;

    // ---- Internal Methods ----
    void EvaluateThreat();
    void UpdateBehaviorState();
    void SetBehaviorState(ENPC_BehaviorState NewState);
    void CleanupExpiredMemories();
    void EvaluateDailyRoutine();
};
