#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// === NPC Alert Level — threat-driven escalation ===
UENUM(BlueprintType)
enum class ENPC_AlertLevel : uint8
{
    Unaware      UMETA(DisplayName = "Unaware"),
    Curious      UMETA(DisplayName = "Curious"),
    Suspicious   UMETA(DisplayName = "Suspicious"),
    Alarmed      UMETA(DisplayName = "Alarmed"),
    Panicked     UMETA(DisplayName = "Panicked")
};

// === NPC Daily Routine Phase ===
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

// === NPC Role in tribe ===
UENUM(BlueprintType)
enum class ENPC_TribeRole : uint8
{
    Elder    UMETA(DisplayName = "Elder"),
    Hunter   UMETA(DisplayName = "Hunter"),
    Gatherer UMETA(DisplayName = "Gatherer"),
    Scout    UMETA(DisplayName = "Scout"),
    Guard    UMETA(DisplayName = "Guard")
};

// === NPC Memory Entry — what this NPC remembers ===
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsPlayerRelated = false;
};

// === NPC Behavior Component ===
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === Alert System ===
    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void SetAlertLevel(ENPC_AlertLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void RaiseThreat(float ThreatAmount, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void DecayThreat(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "NPC|Alert")
    ENPC_AlertLevel GetAlertLevel() const { return CurrentAlertLevel; }

    // === Routine System ===
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void SetRoutinePhase(ENPC_RoutinePhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "NPC|Routine")
    ENPC_RoutinePhase GetRoutinePhase() const { return CurrentRoutinePhase; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void UpdateDailyRoutine(float WorldTimeHours);

    // === Memory System ===
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordMemory(FVector Location, float ThreatLevel, bool bPlayerRelated);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetOldMemories(float MaxAge);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasMemoryOfThreat() const;

    // === Properties ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_TribeRole TribeRole = ENPC_TribeRole::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ThreatDecayRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertThresholdCurious = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertThresholdSuspicious = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertThresholdAlarmed = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertThresholdPanicked = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FVector HomeLocation = FVector::ZeroVector;

private:
    UPROPERTY(VisibleAnywhere, Category = "NPC|State")
    ENPC_AlertLevel CurrentAlertLevel = ENPC_AlertLevel::Unaware;

    UPROPERTY(VisibleAnywhere, Category = "NPC|State")
    ENPC_RoutinePhase CurrentRoutinePhase = ENPC_RoutinePhase::Foraging;

    UPROPERTY(VisibleAnywhere, Category = "NPC|State")
    float CurrentThreatScore = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryEntries;
};
