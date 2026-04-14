#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "../Core/SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// Forward declarations
class UNPC_AIController;
class UBehaviorTree;
class UBlackboardComponent;

/**
 * NPC Behavior States - defines the current activity/mood of an NPC
 */
UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0            UMETA(DisplayName = "Idle"),
    Wandering = 1       UMETA(DisplayName = "Wandering"),
    Working = 2         UMETA(DisplayName = "Working"),
    Socializing = 3     UMETA(DisplayName = "Socializing"),
    Sleeping = 4        UMETA(DisplayName = "Sleeping"),
    Eating = 5          UMETA(DisplayName = "Eating"),
    Hunting = 6         UMETA(DisplayName = "Hunting"),
    Gathering = 7       UMETA(DisplayName = "Gathering"),
    Fleeing = 8         UMETA(DisplayName = "Fleeing"),
    Investigating = 9   UMETA(DisplayName = "Investigating"),
    Following = 10      UMETA(DisplayName = "Following"),
    Guarding = 11       UMETA(DisplayName = "Guarding")
};

/**
 * NPC Personality Traits - affects behavior decisions
 */
UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Curious = 0         UMETA(DisplayName = "Curious"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Aggressive = 2      UMETA(DisplayName = "Aggressive"),
    Friendly = 3        UMETA(DisplayName = "Friendly"),
    Lazy = 4            UMETA(DisplayName = "Lazy"),
    Hardworking = 5     UMETA(DisplayName = "Hardworking"),
    Leader = 6          UMETA(DisplayName = "Leader"),
    Follower = 7        UMETA(DisplayName = "Follower")
};

/**
 * NPC Relationship Types - how NPCs relate to each other
 */
UENUM(BlueprintType)
enum class ENPC_RelationshipType : uint8
{
    Stranger = 0        UMETA(DisplayName = "Stranger"),
    Acquaintance = 1    UMETA(DisplayName = "Acquaintance"),
    Friend = 2          UMETA(DisplayName = "Friend"),
    Family = 3          UMETA(DisplayName = "Family"),
    Rival = 4           UMETA(DisplayName = "Rival"),
    Enemy = 5           UMETA(DisplayName = "Enemy"),
    Leader = 6          UMETA(DisplayName = "Leader"),
    Follower = 7        UMETA(DisplayName = "Follower")
};

/**
 * NPC Memory Entry - stores information about events, people, places
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FString> AssociatedActors;

    FNPC_MemoryEntry()
    {
        EventType = TEXT("");
        Description = TEXT("");
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        Importance = 0.5f;
        AssociatedActors.Empty();
    }
};

/**
 * NPC Relationship Data - tracks relationships with other characters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_RelationshipData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    FString ActorName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    ENPC_RelationshipType RelationshipType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Affection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Trust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Respect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteractionTime;

    FNPC_RelationshipData()
    {
        ActorName = TEXT("");
        RelationshipType = ENPC_RelationshipType::Stranger;
        Affection = 0.0f;
        Trust = 0.0f;
        Respect = 0.0f;
        LastInteractionTime = 0.0f;
    }
};

/**
 * NPC Daily Schedule Entry - defines what NPCs do at different times
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_ScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_BehaviorState Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FString ActivityDescription;

    FNPC_ScheduleEntry()
    {
        StartTime = 0.0f;
        EndTime = 24.0f;
        Activity = ENPC_BehaviorState::Idle;
        TargetLocation = FVector::ZeroVector;
        ActivityDescription = TEXT("Idle");
    }
};

/**
 * Core NPC Behavior Component
 * Manages NPC AI behavior, memory, relationships, and daily routines
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
    // ═══════════════════════════════════════════════════════════════
    // CORE BEHAVIOR PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    TArray<ENPC_PersonalityTrait> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float Awareness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float Energy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float Mood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float SocialNeed;

    // ═══════════════════════════════════════════════════════════════
    // AI CONTROLLER AND BEHAVIOR TREE
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TSubclassOf<AAIController> AIControllerClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AAIController* NPCAIController;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> ShortTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> LongTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxShortTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxLongTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // ═══════════════════════════════════════════════════════════════
    // RELATIONSHIP SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    TArray<FNPC_RelationshipData> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    float RelationshipDecayRate;

    // ═══════════════════════════════════════════════════════════════
    // DAILY SCHEDULE SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FNPC_ScheduleEntry> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    bool bFollowSchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float ScheduleFlexibility;

    // ═══════════════════════════════════════════════════════════════
    // PERCEPTION AND INTERACTION
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    TArray<AActor*> PerceivedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    AActor* CurrentInteractionTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange;

    // ═══════════════════════════════════════════════════════════════
    // CORE BEHAVIOR FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ProcessSchedule();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetCurrentScheduledActivity();

    // ═══════════════════════════════════════════════════════════════
    // MEMORY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(const FString& EventType, const FString& Description, FVector Location, float Importance);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ProcessMemoryDecay(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_MemoryEntry> RecallMemories(const FString& EventType);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ConsolidateMemories();

    // ═══════════════════════════════════════════════════════════════
    // RELATIONSHIP FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    void UpdateRelationship(const FString& ActorName, float AffectionChange, float TrustChange, float RespectChange);

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    FNPC_RelationshipData GetRelationship(const FString& ActorName);

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    void ProcessRelationshipDecay(float DeltaTime);

    // ═══════════════════════════════════════════════════════════════
    // PERCEPTION FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void UpdatePerception();

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void OnActorPerceived(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    bool CanSeeActor(AActor* Actor);

    // ═══════════════════════════════════════════════════════════════
    // INTERACTION FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartInteraction(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void EndInteraction();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ProcessInteraction(float DeltaTime);

    // ═══════════════════════════════════════════════════════════════
    // UTILITY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetCurrentGameTime();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool HasPersonalityTrait(ENPC_PersonalityTrait Trait);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void InitializeNPC();

private:
    // Internal timers and state tracking
    FTimerHandle ScheduleUpdateTimer;
    FTimerHandle MemoryProcessTimer;
    FTimerHandle RelationshipUpdateTimer;
    FTimerHandle PerceptionUpdateTimer;
    
    float LastScheduleUpdate;
    float StateChangeTime;
    bool bIsInitialized;
};