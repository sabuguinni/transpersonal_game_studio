#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../../Core/SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// NPC Memory Entry for tracking interactions and events
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EmotionalWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString AssociatedActor;

    FNPC_MemoryEntry()
    {
        EventType = TEXT("");
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EmotionalWeight = 0.0f;
        AssociatedActor = TEXT("");
    }
};

// NPC Relationship data for social dynamics
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FString TargetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RespectLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FNPC_MemoryEntry> SharedMemories;

    FNPC_Relationship()
    {
        TargetName = TEXT("");
        TrustLevel = 0.0f;
        FearLevel = 0.0f;
        RespectLevel = 0.0f;
    }
};

// NPC Daily Schedule Entry
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_ScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartTime; // 0-24 hour format

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FString ActivityName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    int32 Priority;

    FNPC_ScheduleEntry()
    {
        StartTime = 8.0f;
        Duration = 2.0f;
        ActivityName = TEXT("Idle");
        TargetLocation = FVector::ZeroVector;
        Priority = 1;
    }
};

/**
 * Advanced NPC Behavior Component
 * Handles memory, emotions, social relationships, daily routines, and dynamic decision making
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE NPC PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCRole; // Hunter, Shaman, Gatherer, Elder, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString Tribe;

    // === PERSONALITY TRAITS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Courage; // 0 = Cowardly, 1 = Fearless

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability; // 0 = Hermit, 1 = Social butterfly

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity; // 0 = Cautious, 1 = Explorer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression; // 0 = Peaceful, 1 = Aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence; // 0 = Simple, 1 = Genius

    // === CURRENT STATE ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float CurrentMood; // -1 = Very Sad, 0 = Neutral, 1 = Very Happy

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float StressLevel; // 0 = Calm, 1 = Panicked

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float EnergyLevel; // 0 = Exhausted, 1 = Full energy

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FString CurrentActivity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FVector HomeLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FVector WorkLocation;

    // === MEMORY SYSTEM ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_MemoryEntry> ShortTermMemory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_MemoryEntry> LongTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxShortTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // === SOCIAL SYSTEM ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social")
    TArray<FNPC_Relationship> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialInteractionRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social")
    TArray<AActor*> NearbyNPCs;

    // === DAILY ROUTINE ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FNPC_ScheduleEntry> DailySchedule;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Schedule")
    int32 CurrentScheduleIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    bool bFollowSchedule;

    // === AI BEHAVIOR ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float DecisionMakingInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    float LastDecisionTime;

    // === PUBLIC METHODS ===

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddMemory(const FString& EventType, const FVector& Location, float EmotionalWeight, const FString& AssociatedActor = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateRelationship(const FString& TargetName, float TrustDelta, float FearDelta, float RespectDelta);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPC_Relationship GetRelationship(const FString& TargetName);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetCurrentActivity(const FString& NewActivity);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FString GetCurrentScheduledActivity();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToEvent(const FString& EventType, float Intensity, AActor* Source = nullptr);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldReactToPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateMood(float MoodDelta);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateStress(float StressDelta);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FVector GetPreferredDestination();

private:
    void UpdateMemorySystem(float DeltaTime);
    void UpdateSocialAwareness(float DeltaTime);
    void UpdateSchedule(float DeltaTime);
    void ProcessDecisionMaking(float DeltaTime);
    void ConsolidateMemories();
    float GetGameTimeOfDay();
    void FindNearbyNPCs();
    float CalculatePersonalityInfluence(const FString& EventType);
};