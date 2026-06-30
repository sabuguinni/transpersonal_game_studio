#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// NPC daily routine phase
UENUM(BlueprintType)
enum class ENPC_RoutinePhase : uint8
{
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Waking      UMETA(DisplayName = "Waking"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Resting     UMETA(DisplayName = "Resting"),
    Socializing UMETA(DisplayName = "Socializing"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Investigating UMETA(DisplayName = "Investigating"),
    Patrolling  UMETA(DisplayName = "Patrolling")
};

// NPC alert level
UENUM(BlueprintType)
enum class ENPC_AlertLevel : uint8
{
    Unaware     UMETA(DisplayName = "Unaware"),
    Curious     UMETA(DisplayName = "Curious"),
    Suspicious  UMETA(DisplayName = "Suspicious"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Panicked    UMETA(DisplayName = "Panicked")
};

// Memory entry — what this NPC remembers about a stimulus
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float DecayRate = 30.0f;  // seconds until memory fades

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = false;
};

// Patrol waypoint
USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLookAround = true;
};

/**
 * UNPCBehaviorComponent
 * Drives NPC daily routines, memory, alert states, and patrol logic.
 * Attach to any ACharacter or APawn to give it autonomous behavior.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // --- Alert System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void ReceiveThreatStimulus(FVector StimulusLocation, float ThreatMagnitude);

    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void ClearAlert();

    UFUNCTION(BlueprintPure, Category = "NPC|Alert")
    ENPC_AlertLevel GetCurrentAlertLevel() const { return CurrentAlertLevel; }

    // --- Memory System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordMemory(FVector Location, float Threat);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasActiveMemory() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FNPC_MemoryEntry GetMostRecentMemory() const;

    // --- Routine System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void SetRoutinePhase(ENPC_RoutinePhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "NPC|Routine")
    ENPC_RoutinePhase GetCurrentRoutinePhase() const { return CurrentRoutinePhase; }

    // --- Patrol System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AddPatrolPoint(FVector Location, float WaitTime = 2.0f, bool bLook = true);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolDestination();

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertDecayRate = 5.0f;  // alert level drops per second when no stimulus

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ThreatThresholdCurious = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ThreatThresholdSuspicious = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ThreatThresholdAlarmed = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ThreatThresholdPanicked = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryCapacity = 5.0f;  // max simultaneous memories

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius = 2000.0f;

private:
    UPROPERTY()
    ENPC_AlertLevel CurrentAlertLevel = ENPC_AlertLevel::Unaware;

    UPROPERTY()
    ENPC_RoutinePhase CurrentRoutinePhase = ENPC_RoutinePhase::Foraging;

    UPROPERTY()
    float CurrentThreatAccumulator = 0.0f;

    UPROPERTY()
    TArray<FNPC_MemoryEntry> MemoryBank;

    UPROPERTY()
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY()
    int32 CurrentPatrolIndex = 0;

    UPROPERTY()
    float PatrolWaitTimer = 0.0f;

    void UpdateAlertLevel();
    void DecayMemories(float DeltaTime);
    void DecayThreat(float DeltaTime);
    void UpdateRoutineFromAlert();
};
