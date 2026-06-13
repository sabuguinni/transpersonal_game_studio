#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Socializing UMETA(DisplayName = "Socializing"),
    Working UMETA(DisplayName = "Working"),
    Resting UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class ENPC_PersonalityType : uint8
{
    Cautious UMETA(DisplayName = "Cautious"),
    Aggressive UMETA(DisplayName = "Aggressive"),
    Curious UMETA(DisplayName = "Curious"),
    Social UMETA(DisplayName = "Social"),
    Solitary UMETA(DisplayName = "Solitary")
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Importance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventType;

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        Importance = 0.0f;
        EventType = TEXT("Unknown");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Current behavior state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentState;

    // Personality traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Personality")
    ENPC_PersonalityType PersonalityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability;

    // Memory system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> ShortTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> LongTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxShortTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxLongTermMemories;

    // Patrol behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Patrol")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Patrol")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Patrol")
    float PatrolSpeed;

    // Detection and awareness
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Detection")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Detection")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Detection")
    float AlertnessLevel;

    // Daily routine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    float DailyRoutineTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    float RoutineChangeInterval;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(FVector Location, FString EventType, float Importance);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ProcessMemories(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Patrol")
    void InitializePatrolRoute();

    UFUNCTION(BlueprintCallable, Category = "NPC Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC Detection")
    bool DetectPlayer();

    UFUNCTION(BlueprintCallable, Category = "NPC Routine")
    void UpdateDailyRoutine(float DeltaTime);

private:
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle MemoryProcessTimer;

    void UpdateBehavior();
    void ConsolidateMemories();
    float CalculateMemoryImportance(const FNPC_MemoryEntry& Memory, float CurrentTime);
};