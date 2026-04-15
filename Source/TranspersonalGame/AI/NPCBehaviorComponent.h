#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Core/SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

/**
 * NPC Behavior Agent #11 - Cycle 021
 * 
 * Core NPC behavior system that drives all non-player character AI.
 * Manages personality traits, needs, social dynamics, and behavior states.
 * Each NPC has their own life and routines independent of the player.
 */

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0            UMETA(DisplayName = "Idle"),
    Working = 1         UMETA(DisplayName = "Working"),
    Socializing = 2     UMETA(DisplayName = "Socializing"),
    Eating = 3          UMETA(DisplayName = "Eating"),
    Sleeping = 4        UMETA(DisplayName = "Sleeping"),
    Patrolling = 5      UMETA(DisplayName = "Patrolling"),
    Fleeing = 6         UMETA(DisplayName = "Fleeing"),
    Fighting = 7        UMETA(DisplayName = "Fighting"),
    Gathering = 8       UMETA(DisplayName = "Gathering"),
    Hunting = 9         UMETA(DisplayName = "Hunting")
};

UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Aggressive = 0      UMETA(DisplayName = "Aggressive"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Curious = 2         UMETA(DisplayName = "Curious"),
    Loyal = 3           UMETA(DisplayName = "Loyal"),
    Independent = 4     UMETA(DisplayName = "Independent"),
    Social = 5          UMETA(DisplayName = "Social"),
    Brave = 6           UMETA(DisplayName = "Brave"),
    Cowardly = 7        UMETA(DisplayName = "Cowardly")
};

UENUM(BlueprintType)
enum class ENPC_Mood : uint8
{
    Content = 0         UMETA(DisplayName = "Content"),
    Happy = 1           UMETA(DisplayName = "Happy"),
    Angry = 2           UMETA(DisplayName = "Angry"),
    Fearful = 3         UMETA(DisplayName = "Fearful"),
    Sad = 4             UMETA(DisplayName = "Sad"),
    Excited = 5         UMETA(DisplayName = "Excited"),
    Stressed = 6        UMETA(DisplayName = "Stressed"),
    Relaxed = 7         UMETA(DisplayName = "Relaxed")
};

UENUM(BlueprintType)
enum class ENPC_Relationship : uint8
{
    Stranger = 0        UMETA(DisplayName = "Stranger"),
    Acquaintance = 1    UMETA(DisplayName = "Acquaintance"),
    Friend = 2          UMETA(DisplayName = "Friend"),
    Enemy = 3           UMETA(DisplayName = "Enemy"),
    Ally = 4            UMETA(DisplayName = "Ally"),
    Family = 5          UMETA(DisplayName = "Family"),
    Rival = 6           UMETA(DisplayName = "Rival"),
    Neutral = 7         UMETA(DisplayName = "Neutral")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Needs
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Comfort = 60.0f;

    FNPC_Needs()
    {
        Hunger = 50.0f;
        Energy = 100.0f;
        Social = 50.0f;
        Safety = 75.0f;
        Comfort = 60.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EmotionalImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FString> InvolvedActors;

    FNPC_Memory()
    {
        EventDescription = TEXT("");
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EmotionalImpact = 0.0f;
        InvolvedActors.Empty();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    FString TargetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    ENPC_Relationship RelationshipType = ENPC_Relationship::Stranger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
    float Affection = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Trust = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Respect = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteractionTime = 0.0f;

    FNPC_Relationship()
    {
        TargetName = TEXT("");
        RelationshipType = ENPC_Relationship::Stranger;
        Affection = 0.0f;
        Trust = 50.0f;
        Respect = 50.0f;
        LastInteractionTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ═══════════════════════════════════════════════════════════════
    // CORE BEHAVIOR PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FString NPCName = TEXT("Unknown NPC");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentBehaviorState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_Mood CurrentMood = ENPC_Mood::Content;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    TArray<ENPC_PersonalityTrait> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FNPC_Needs CurrentNeeds;

    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR TREE INTEGRATION
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardAsset* BlackboardAsset;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class AAIController* AIController;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY AND RELATIONSHIPS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    TArray<FNPC_Relationship> Relationships;

    // ═══════════════════════════════════════════════════════════════
    // DAILY ROUTINES AND SCHEDULING
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TMap<float, ENPC_BehaviorState> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FVector WorkLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FVector> PatrolPoints;

    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateMood(ENPC_Mood NewMood);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddPersonalityTrait(ENPC_PersonalityTrait Trait);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool HasPersonalityTrait(ENPC_PersonalityTrait Trait) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetMostUrgentNeed() const;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(const FString& EventDescription, FVector Location, float EmotionalImpact, const TArray<FString>& InvolvedActors);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_Memory> GetMemoriesAtLocation(FVector Location, float Radius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_Memory> GetMemoriesWithActor(const FString& ActorName) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetOldMemories();

    // ═══════════════════════════════════════════════════════════════
    // RELATIONSHIP FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    void UpdateRelationship(const FString& TargetName, ENPC_Relationship RelationType, float AffectionChange, float TrustChange, float RespectChange);

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    FNPC_Relationship GetRelationship(const FString& TargetName) const;

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    bool IsHostileTowards(const FString& TargetName) const;

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    bool IsFriendlyTowards(const FString& TargetName) const;

    // ═══════════════════════════════════════════════════════════════
    // ROUTINE FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void SetDailySchedule(const TMap<float, ENPC_BehaviorState>& Schedule);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    ENPC_BehaviorState GetScheduledBehavior(float TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void AddPatrolPoint(FVector Point);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FVector GetNextPatrolPoint() const;

    // ═══════════════════════════════════════════════════════════════
    // AI INTEGRATION FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "AI")
    void InitializeBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardValues();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopBehaviorTree();

private:
    // Internal state tracking
    float StateTimer = 0.0f;
    int32 CurrentPatrolIndex = 0;
    float LastNeedsUpdate = 0.0f;
    float NeedsUpdateInterval = 5.0f; // Update needs every 5 seconds
    
    // Helper functions
    void InitializeDefaultSchedule();
    void ProcessBehaviorState(float DeltaTime);
    void HandleStateTransition(ENPC_BehaviorState NewState);
    float CalculateNeedUrgency(float NeedValue) const;
};