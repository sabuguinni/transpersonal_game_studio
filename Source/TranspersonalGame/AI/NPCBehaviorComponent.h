#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Flee        UMETA(DisplayName = "Flee"),
    Alert       UMETA(DisplayName = "Alert"),
    Socialise   UMETA(DisplayName = "Socialise"),
    Sleep       UMETA(DisplayName = "Sleep"),
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
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutineSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float TimeOfDayStart = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float TimeOfDayEnd = 8.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_BehaviorState DesiredState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    FVector TargetLocation = FVector::ZeroVector;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // State management
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

    // Memory system
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RegisterThreat(AActor* ThreatActor, FVector Location, ENPC_ThreatLevel Level);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ClearExpiredMemories();

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasActiveThreat() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    ENPC_ThreatLevel GetHighestThreatLevel() const;

    // Patrol
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AddPatrolPoint(FVector Point);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint();

    // Daily routine
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void AddRoutineSlot(FNPC_DailyRoutineSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void EvaluateRoutine(float CurrentTimeOfDay);

    // Perception
    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    void OnSightStimulus(AActor* SeenActor, float Strength);

    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    void OnHearingStimulus(FVector SoundLocation, float Loudness);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState PreviousState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 8;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory",
        meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_MemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Patrol",
        meta = (AllowPrivateAccess = "true"))
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_DailyRoutineSlot> DailyRoutine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float FleeThreshold = 0.4f;

private:
    void UpdateBehavior(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandlePatrolState(float DeltaTime);
    void HandleFleeState(float DeltaTime);
    void HandleAlertState(float DeltaTime);

    float StateTimer = 0.0f;
    float MemoryCleanupTimer = 0.0f;
};
