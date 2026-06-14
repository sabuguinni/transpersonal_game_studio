#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Fleeing,
    Hunting,
    Feeding,
    Socializing,
    Resting,
    Territorial
};

UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm,
    Alert,
    Aggressive,
    Fearful,
    Curious,
    Hungry,
    Tired,
    Territorial
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector Location;

    UPROPERTY(BlueprintReadOnly)
    AActor* Actor;

    UPROPERTY(BlueprintReadOnly)
    float Timestamp;

    UPROPERTY(BlueprintReadOnly)
    float Importance;

    UPROPERTY(BlueprintReadOnly)
    FString EventType;

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        Actor = nullptr;
        Timestamp = 0.0f;
        Importance = 0.0f;
        EventType = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_BehaviorState Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority;

    FNPC_DailyRoutine()
    {
        StartHour = 0.0f;
        EndHour = 24.0f;
        Activity = ENPC_BehaviorState::Idle;
        TargetLocation = FVector::ZeroVector;
        Priority = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior State Management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetEmotionalState(ENPC_EmotionalState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    ENPC_EmotionalState GetEmotionalState() const { return CurrentEmotionalState; }

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(const FVector& Location, AActor* Actor, const FString& EventType, float Importance = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ForgetOldMemories(float MaxAge = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesOfType(const FString& EventType) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FNPC_MemoryEntry GetMostImportantMemory() const;

    // Daily Routine System
    UFUNCTION(BlueprintCallable, Category = "NPC Routine")
    void AddRoutineActivity(float StartHour, float EndHour, ENPC_BehaviorState Activity, const FVector& Location, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC Routine")
    FNPC_DailyRoutine GetCurrentRoutineActivity() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Routine")
    void UpdateRoutine();

    // Reaction System
    UFUNCTION(BlueprintCallable, Category = "NPC Reactions")
    void ReactToPlayer(AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "NPC Reactions")
    void ReactToThreat(AActor* Threat, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC Reactions")
    void ReactToNoise(const FVector& NoiseLocation, float NoiseLevel);

    // Social Behavior
    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void InteractWithNPC(UNPC_BehaviorComponent* OtherNPC);

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    float GetRelationshipValue(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void ModifyRelationship(AActor* OtherActor, float Delta);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    float GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    bool IsPlayerNearby(float Radius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    AActor* FindNearestActorOfClass(TSubclassOf<AActor> ActorClass, float MaxDistance = 2000.0f) const;

protected:
    // Core State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    ENPC_EmotionalState CurrentEmotionalState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    float StateChangeTime;

    // Memory System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float MemoryDecayRate;

    // Daily Routine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    TArray<FNPC_DailyRoutine> DailyRoutines;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Routine")
    int32 CurrentRoutineIndex;

    // Social Relationships
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Social")
    TMap<AActor*, float> Relationships;

    // Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Parameters")
    float PersonalSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Parameters")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Parameters")
    float FleeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Parameters")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Parameters")
    float CuriosityLevel;

    // Timers
    FTimerHandle RoutineUpdateTimer;
    FTimerHandle MemoryCleanupTimer;
    FTimerHandle BehaviorUpdateTimer;

private:
    void UpdateBehavior();
    void CleanupMemories();
    bool ShouldChangeState(ENPC_BehaviorState NewState) const;
    float CalculateStatePriority(ENPC_BehaviorState State) const;
};