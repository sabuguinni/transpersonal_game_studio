#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle,
    Patrol,
    Chase,
    Attack,
    Flee,
    Investigate,
    Social,
    Rest
};

UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm,
    Alert,
    Aggressive,
    Fearful,
    Curious,
    Tired,
    Hungry
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FString EventType;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float Importance;

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EventType = TEXT("");
        Importance = 0.0f;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior State Management
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    ENPC_EmotionalState CurrentEmotionalState;

    // Patrol System
    UPROPERTY(BlueprintReadWrite, Category = "Patrol")
    FVector PatrolCenter;

    UPROPERTY(BlueprintReadWrite, Category = "Patrol")
    float PatrolRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadWrite, Category = "Patrol")
    int32 CurrentPatrolIndex;

    // Memory System
    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> Memories;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // Detection and Awareness
    UPROPERTY(BlueprintReadWrite, Category = "Detection")
    float SightRange;

    UPROPERTY(BlueprintReadWrite, Category = "Detection")
    float HearingRange;

    UPROPERTY(BlueprintReadWrite, Category = "Detection")
    float AlertLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Detection")
    AActor* CurrentTarget;

    // Social Behavior
    UPROPERTY(BlueprintReadWrite, Category = "Social")
    TArray<AActor*> KnownNPCs;

    UPROPERTY(BlueprintReadWrite, Category = "Social")
    float SocialRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Social")
    bool bCanInteractWithPlayer;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetEmotionalState(ENPC_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void GeneratePatrolPoints();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    FVector GetNextPatrolPoint();

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(FVector Location, FString EventType, float Importance);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemories(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_MemoryEntry GetMostImportantMemory();

    // Detection Functions
    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool CanSeeActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool CanHearActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    void UpdateDetection();

    // Social Functions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void FindNearbyNPCs();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void InteractWithNPC(AActor* OtherNPC);

private:
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle MemoryUpdateTimer;
    FTimerHandle DetectionUpdateTimer;

    void UpdateBehavior();
    void HandleIdleBehavior();
    void HandlePatrolBehavior();
    void HandleChaseBehavior();
    void HandleInvestigateBehavior();
    void HandleSocialBehavior();
};