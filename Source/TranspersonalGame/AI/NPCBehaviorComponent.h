#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../Core/SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

/**
 * NPC Behavior Component - Core NPC AI behavior system
 * Handles daily routines, memory, social interactions, and emotional states
 * 
 * Design Philosophy:
 * - NPCs have lives independent of the player
 * - Emergent behavior over scripted sequences
 * - Social dynamics drive interesting interactions
 * - Memory creates persistent relationships
 */

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0            UMETA(DisplayName = "Idle"),
    Patrolling = 1      UMETA(DisplayName = "Patrolling"),
    Working = 2         UMETA(DisplayName = "Working"),
    Socializing = 3     UMETA(DisplayName = "Socializing"),
    Eating = 4          UMETA(DisplayName = "Eating"),
    Sleeping = 5        UMETA(DisplayName = "Sleeping"),
    Fleeing = 6         UMETA(DisplayName = "Fleeing"),
    Investigating = 7   UMETA(DisplayName = "Investigating"),
    Following = 8       UMETA(DisplayName = "Following"),
    Hunting = 9         UMETA(DisplayName = "Hunting")
};

UENUM(BlueprintType)
enum class ENPC_Emotion : uint8
{
    Neutral = 0         UMETA(DisplayName = "Neutral"),
    Happy = 1           UMETA(DisplayName = "Happy"),
    Sad = 2             UMETA(DisplayName = "Sad"),
    Angry = 3           UMETA(DisplayName = "Angry"),
    Fearful = 4         UMETA(DisplayName = "Fearful"),
    Curious = 5         UMETA(DisplayName = "Curious"),
    Aggressive = 6      UMETA(DisplayName = "Aggressive"),
    Friendly = 7        UMETA(DisplayName = "Friendly"),
    Suspicious = 8      UMETA(DisplayName = "Suspicious"),
    Excited = 9         UMETA(DisplayName = "Excited")
};

UENUM(BlueprintType)
enum class ENPC_Relationship : uint8
{
    Unknown = 0         UMETA(DisplayName = "Unknown"),
    Stranger = 1        UMETA(DisplayName = "Stranger"),
    Acquaintance = 2    UMETA(DisplayName = "Acquaintance"),
    Friend = 3          UMETA(DisplayName = "Friend"),
    Enemy = 4           UMETA(DisplayName = "Enemy"),
    Ally = 5            UMETA(DisplayName = "Ally"),
    Family = 6          UMETA(DisplayName = "Family"),
    Leader = 7          UMETA(DisplayName = "Leader"),
    Follower = 8        UMETA(DisplayName = "Follower")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* Subject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastSeenLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_Emotion AssociatedEmotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_Relationship Relationship;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString MemoryDescription;

    FNPC_Memory()
    {
        Subject = nullptr;
        LastSeenLocation = FVector::ZeroVector;
        LastSeenTime = 0.0f;
        AssociatedEmotion = ENPC_Emotion::Neutral;
        Relationship = ENPC_Relationship::Unknown;
        TrustLevel = 0.0f;
        MemoryDescription = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_BehaviorState Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector ActivityLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FString ActivityDescription;

    FNPC_DailySchedule()
    {
        StartTime = 0.0f;
        EndTime = 0.0f;
        Activity = ENPC_BehaviorState::Idle;
        ActivityLocation = FVector::ZeroVector;
        ActivityDescription = TEXT("");
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

    // ═══════════════════════════════════════════════════════════════
    // CORE BEHAVIOR PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_Emotion CurrentEmotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float EmotionIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FString NPCRole;

    // ═══════════════════════════════════════════════════════════════
    // PERSONALITY TRAITS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Courage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // ═══════════════════════════════════════════════════════════════
    // DAILY ROUTINE SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FNPC_DailySchedule> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    bool bFollowDailySchedule;

    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetEmotion(ENPC_Emotion NewEmotion, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_Emotion GetCurrentEmotion() const;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(AActor* Subject, ENPC_Emotion Emotion, ENPC_Relationship Relationship, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_Memory* FindMemory(AActor* Subject);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemory(AActor* Subject, ENPC_Emotion Emotion, float TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetOldMemories();

    // ═══════════════════════════════════════════════════════════════
    // SOCIAL FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ReactToActor(AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    ENPC_Relationship GetRelationshipWith(AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void StartConversation(AActor* OtherActor);

    // ═══════════════════════════════════════════════════════════════
    // SCHEDULE FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Schedule")
    void UpdateDailyRoutine(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Schedule")
    FNPC_DailySchedule* GetCurrentScheduledActivity();

    UFUNCTION(BlueprintCallable, Category = "Schedule")
    void AddScheduledActivity(float StartTime, float EndTime, ENPC_BehaviorState Activity, FVector Location, const FString& Description);

private:
    // Internal state tracking
    float LastEmotionChangeTime;
    float LastBehaviorChangeTime;
    AActor* CurrentTarget;
    FVector HomeLocation;
    
    // Helper functions
    void ProcessEmotionalDecay(float DeltaTime);
    void CheckForNearbyActors();
    void ExecuteCurrentBehavior();
};