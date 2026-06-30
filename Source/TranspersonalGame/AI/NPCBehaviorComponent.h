#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// Alert level enum — threat-driven escalation
UENUM(BlueprintType)
enum class ENPC_AlertLevel : uint8
{
    Unaware     UMETA(DisplayName = "Unaware"),
    Curious     UMETA(DisplayName = "Curious"),
    Suspicious  UMETA(DisplayName = "Suspicious"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Panicked    UMETA(DisplayName = "Panicked")
};

// Daily routine phase
UENUM(BlueprintType)
enum class ENPC_RoutinePhase : uint8
{
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Waking          UMETA(DisplayName = "Waking"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Resting         UMETA(DisplayName = "Resting"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Patrolling      UMETA(DisplayName = "Patrolling")
};

// Memory entry for perceived threats/events
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = false;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Alert System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void RaiseThreat(float ThreatAmount, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void DecayThreat(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "NPC|Alert")
    ENPC_AlertLevel GetAlertLevel() const { return CurrentAlertLevel; }

    // --- Routine System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void SetRoutinePhase(ENPC_RoutinePhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "NPC|Routine")
    ENPC_RoutinePhase GetRoutinePhase() const { return CurrentRoutinePhase; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void UpdateDailyRoutine(float GameTimeHours);

    // --- Memory System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordMemory(FVector Location, float ThreatLevel);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasActiveMemory() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FNPC_MemoryEntry GetMostRecentMemory() const;

    // --- Patrol System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolRadius(float Radius) { PatrolRadius = Radius; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint() const;

    // --- Properties ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ThreatDecayRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertThresholdCurious = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertThresholdSuspicious = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertThresholdAlarmed = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertThresholdPanicked = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    int32 MaxMemoryEntries = 8;

private:
    UPROPERTY()
    ENPC_AlertLevel CurrentAlertLevel = ENPC_AlertLevel::Unaware;

    UPROPERTY()
    ENPC_RoutinePhase CurrentRoutinePhase = ENPC_RoutinePhase::Foraging;

    UPROPERTY()
    float CurrentThreatScore = 0.0f;

    UPROPERTY()
    TArray<FNPC_MemoryEntry> MemoryEntries;

    UPROPERTY()
    FVector HomeLocation = FVector::ZeroVector;

    void UpdateAlertLevelFromScore();
};
