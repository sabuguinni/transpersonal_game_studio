#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// NPC Behavior Component — Agent #11
// Drives daily routines, memory, threat reactions, and social
// state for all non-player characters in the prehistoric world.
// ============================================================

UENUM(BlueprintType)
enum class ENPC_DailyPhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    DeepNight   UMETA(DisplayName = "DeepNight")
};

UENUM(BlueprintType)
enum class ENPC_AlertLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Aware       UMETA(DisplayName = "Aware"),
    Suspicious  UMETA(DisplayName = "Suspicious"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hiding      UMETA(DisplayName = "Hiding")
};

UENUM(BlueprintType)
enum class ENPC_SocialRole : uint8
{
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Guard       UMETA(DisplayName = "Guard"),
    Child       UMETA(DisplayName = "Child")
};

USTRUCT(BlueprintType)
struct FNPC_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceSeen = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct FNPC_DailyTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_DailyPhase Phase = ENPC_DailyPhase::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    FName TaskTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float Duration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    bool bCompleted = false;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Social Identity ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    ENPC_SocialRole SocialRole = ENPC_SocialRole::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    FName NPCName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    int32 TribeID = 0;

    // ---- Alert State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Alert")
    ENPC_AlertLevel AlertLevel = ENPC_AlertLevel::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float AlertDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float ThreatDetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float FleeRadius = 3500.0f;

    // ---- Daily Routine ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_DailyPhase CurrentPhase = ENPC_DailyPhase::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_DailyTask> DailySchedule;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Routine")
    int32 CurrentTaskIndex = 0;

    // ---- Threat Memory ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_ThreatMemory> ThreatMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxThreatMemories = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryFadeTime = 120.0f;

    // ---- Survival Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float Hunger = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float Thirst = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float Fatigue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float HungerDrainRate = 0.002f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float ThirstDrainRate = 0.003f;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void RegisterThreat(AActor* ThreatActor, float ThreatScore);

    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void ClearThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    ENPC_AlertLevel GetAlertLevel() const { return AlertLevel; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void AdvanceDailyPhase(ENPC_DailyPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    FNPC_DailyTask GetCurrentTask() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    bool AdvanceToNextTask();

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    bool HasActiveThreats() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    FVector GetHighestThreatLocation() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Survival")
    bool IsHungry() const { return Hunger < 0.3f; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Survival")
    bool IsThirsty() const { return Thirst < 0.3f; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Survival")
    bool IsFatigued() const { return Fatigue > 0.75f; }

private:
    void TickThreatMemories(float DeltaTime);
    void TickSurvivalStats(float DeltaTime);
    void EvaluateAlertLevel();
    void BuildDefaultSchedule();
};
