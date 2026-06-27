#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Flee        UMETA(DisplayName = "Flee"),
    Alert       UMETA(DisplayName = "Alert"),
    Interact    UMETA(DisplayName = "Interact"),
    Seek        UMETA(DisplayName = "Seek")
};

UENUM(BlueprintType)
enum class ENPC_DailyRoutine : uint8
{
    Sleep       UMETA(DisplayName = "Sleep"),
    Forage      UMETA(DisplayName = "Forage"),
    Socialize   UMETA(DisplayName = "Socialize"),
    Guard       UMETA(DisplayName = "Guard"),
    Rest        UMETA(DisplayName = "Rest")
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float Timestamp;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FString EventTag;

    FNPC_MemoryEntry()
        : Location(FVector::ZeroVector)
        , Timestamp(0.f)
        , ThreatLevel(0.f)
        , EventTag(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    FVector WorldLocation;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration;

    FNPC_PatrolPoint()
        : WorldLocation(FVector::ZeroVector)
        , WaitDuration(2.f)
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_DailyRoutine CurrentRoutine;

    // --- Patrol ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolRadius;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentPatrolIndex;

    // --- Detection ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float FleeThreshold;

    // --- Memory ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayTime;

    // --- Methods ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetDailyRoutine(ENPC_DailyRoutine NewRoutine);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordMemoryEvent(FVector EventLocation, float ThreatLevel, const FString& EventTag);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void PurgeOldMemories(float CurrentTime);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Detection")
    bool IsPlayerInSightRange() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Detection")
    bool IsThreatNearby(float& OutThreatLevel) const;

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

private:
    float StateTimer;
    float PatrolWaitTimer;
    bool bWaitingAtPatrolPoint;
    FVector FleeDestination;

    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickFlee(float DeltaTime);
    void TickAlert(float DeltaTime);
    void GeneratePatrolPoints();
};
