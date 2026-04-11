#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NPCBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPCPersonality : uint8
{
    Cautious        UMETA(DisplayName = "Cautious"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Curious         UMETA(DisplayName = "Curious"),
    Social          UMETA(DisplayName = "Social"),
    Solitary        UMETA(DisplayName = "Solitary"),
    Wise            UMETA(DisplayName = "Wise"),
    Playful         UMETA(DisplayName = "Playful"),
    Protective      UMETA(DisplayName = "Protective")
};

UENUM(BlueprintType)
enum class ENPCMoodState : uint8
{
    Content         UMETA(DisplayName = "Content"),
    Happy           UMETA(DisplayName = "Happy"),
    Sad             UMETA(DisplayName = "Sad"),
    Angry           UMETA(DisplayName = "Angry"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Excited         UMETA(DisplayName = "Excited"),
    Tired           UMETA(DisplayName = "Tired"),
    Hungry          UMETA(DisplayName = "Hungry"),
    Alert           UMETA(DisplayName = "Alert"),
    Relaxed         UMETA(DisplayName = "Relaxed")
};

UENUM(BlueprintType)
enum class ENPCActivity : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Wandering       UMETA(DisplayName = "Wandering"),
    Working         UMETA(DisplayName = "Working"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Eating          UMETA(DisplayName = "Eating"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Investigating   UMETA(DisplayName = "Investigating")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Relationship; // -1.0 (enemy) to 1.0 (friend)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 InteractionCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString LastInteractionType;

    FNPCMemoryEntry()
    {
        Actor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        Relationship = 0.0f;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
        LastInteractionType = TEXT("None");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartTime; // Hour of day (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float Duration; // Duration in hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPCActivity Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    int32 Priority; // 0-10, higher = more important

    FNPCScheduleEntry()
    {
        StartTime = 0.0f;
        Duration = 1.0f;
        Activity = ENPCActivity::Idle;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        Priority = 1;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core NPC Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    ENPCPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    ENPCMoodState CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    ENPCActivity CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    float Energy; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    float Hunger; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    float Social; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    float Alertness; // 0-100

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPCMemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // Schedule System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FNPCScheduleEntry> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float CurrentTimeOfDay;

    // Behavior Tree Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // Detection and Awareness
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    float SightAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    TArray<AActor*> KnownActors;

    // Personality Traits (0-1 scale)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateMood(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ProcessSchedule();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddMemoryEntry(AActor* Actor, const FString& InteractionType, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPCMemoryEntry* GetMemoryEntry(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateAwareness();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool CanSeeActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool CanHearActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetCurrentActivity(ENPCActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartConversation(AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    float GetRelationshipWith(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ModifyRelationship(AActor* Actor, float Change);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPCActivity GetOptimalActivity();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void EmergencyBehavior(const FString& Threat);

private:
    // Internal state tracking
    float LastMoodUpdate;
    float LastNeedsUpdate;
    float LastScheduleCheck;
    float LastAwarenessUpdate;

    // Helper functions
    void DecayMemories(float DeltaTime);
    void ApplyPersonalityToMood();
    FNPCScheduleEntry* GetCurrentScheduleEntry();
    void HandleActivityTransition(ENPCActivity OldActivity, ENPCActivity NewActivity);
    float CalculatePersonalityInfluence(ENPCMoodState MoodState);
};