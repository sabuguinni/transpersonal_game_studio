#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorTypes.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - NPC BEHAVIOR TYPES
 * NPC Behavior Agent #11 - PROD_CYCLE_023
 * 
 * Comprehensive type definitions for NPC behavior system including:
 * - Personality traits and archetypes
 * - Behavior states and transitions
 * - Social dynamics and relationships
 * - Memory and learning systems
 * - Profession-based behaviors
 * - Survival needs and responses
 */

// ═══════════════════════════════════════════════════════════════
// NPC PERSONALITY AND TRAITS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Brave = 0           UMETA(DisplayName = "Brave"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Aggressive = 2      UMETA(DisplayName = "Aggressive"),
    Peaceful = 3        UMETA(DisplayName = "Peaceful"),
    Curious = 4         UMETA(DisplayName = "Curious"),
    Reclusive = 5       UMETA(DisplayName = "Reclusive"),
    Leader = 6          UMETA(DisplayName = "Leader"),
    Follower = 7        UMETA(DisplayName = "Follower"),
    Protective = 8      UMETA(DisplayName = "Protective"),
    Selfish = 9         UMETA(DisplayName = "Selfish"),
    Wise = 10           UMETA(DisplayName = "Wise"),
    Impulsive = 11      UMETA(DisplayName = "Impulsive")
};

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
    Hunting = 9         UMETA(DisplayName = "Hunting"),
    Trading = 10        UMETA(DisplayName = "Trading"),
    Crafting = 11       UMETA(DisplayName = "Crafting"),
    Exploring = 12      UMETA(DisplayName = "Exploring"),
    Mourning = 13       UMETA(DisplayName = "Mourning"),
    Celebrating = 14    UMETA(DisplayName = "Celebrating")
};

UENUM(BlueprintType)
enum class ENPC_Profession : uint8
{
    Villager = 0        UMETA(DisplayName = "Villager"),
    Hunter = 1          UMETA(DisplayName = "Hunter"),
    Gatherer = 2        UMETA(DisplayName = "Gatherer"),
    Crafter = 3         UMETA(DisplayName = "Crafter"),
    Guard = 4           UMETA(DisplayName = "Guard"),
    Scout = 5           UMETA(DisplayName = "Scout"),
    Elder = 6           UMETA(DisplayName = "Elder"),
    Shaman = 7          UMETA(DisplayName = "Shaman"),
    Trader = 8          UMETA(DisplayName = "Trader"),
    Child = 9           UMETA(DisplayName = "Child")
};

// ═══════════════════════════════════════════════════════════════
// SURVIVAL NEEDS AND RESPONSES
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SurvivalNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Warmth = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social = 50.0f;

    FNPC_SurvivalNeeds()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Energy = 100.0f;
        Warmth = 50.0f;
        Safety = 50.0f;
        Social = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_EmotionalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
    float Happiness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Anger = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Stress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Curiosity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Trust = 50.0f;

    FNPC_EmotionalState()
    {
        Happiness = 0.0f;
        Fear = 0.0f;
        Anger = 0.0f;
        Stress = 0.0f;
        Curiosity = 50.0f;
        Trust = 50.0f;
    }
};

// ═══════════════════════════════════════════════════════════════
// MEMORY AND LEARNING SYSTEMS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_MemoryType : uint8
{
    Location = 0        UMETA(DisplayName = "Location"),
    Person = 1          UMETA(DisplayName = "Person"),
    Threat = 2          UMETA(DisplayName = "Threat"),
    Resource = 3        UMETA(DisplayName = "Resource"),
    Event = 4           UMETA(DisplayName = "Event"),
    Skill = 5           UMETA(DisplayName = "Skill"),
    Route = 6           UMETA(DisplayName = "Route"),
    Schedule = 7        UMETA(DisplayName = "Schedule")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_MemoryType Type = ENPC_MemoryType::Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float DecayRate = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsPermanent = false;

    FNPC_Memory()
    {
        Type = ENPC_MemoryType::Location;
        Location = FVector::ZeroVector;
        Description = TEXT("");
        Importance = 1.0f;
        Timestamp = 0.0f;
        DecayRate = 0.01f;
        bIsPermanent = false;
    }
};

// ═══════════════════════════════════════════════════════════════
// SOCIAL RELATIONSHIPS AND DYNAMICS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_RelationshipType : uint8
{
    Stranger = 0        UMETA(DisplayName = "Stranger"),
    Acquaintance = 1    UMETA(DisplayName = "Acquaintance"),
    Friend = 2          UMETA(DisplayName = "Friend"),
    Family = 3          UMETA(DisplayName = "Family"),
    Romantic = 4        UMETA(DisplayName = "Romantic"),
    Rival = 5           UMETA(DisplayName = "Rival"),
    Enemy = 6           UMETA(DisplayName = "Enemy"),
    Leader = 7          UMETA(DisplayName = "Leader"),
    Follower = 8        UMETA(DisplayName = "Follower"),
    Mentor = 9          UMETA(DisplayName = "Mentor"),
    Student = 10        UMETA(DisplayName = "Student"),
    Trader = 11         UMETA(DisplayName = "Trader")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    FString TargetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    ENPC_RelationshipType Type = ENPC_RelationshipType::Stranger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
    float Affection = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Trust = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Respect = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteraction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    int32 InteractionCount = 0;

    FNPC_Relationship()
    {
        TargetName = TEXT("");
        Type = ENPC_RelationshipType::Stranger;
        Affection = 0.0f;
        Trust = 50.0f;
        Respect = 50.0f;
        LastInteraction = 0.0f;
        InteractionCount = 0;
    }
};

// ═══════════════════════════════════════════════════════════════
// DIALOGUE AND COMMUNICATION
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_DialogueType : uint8
{
    Greeting = 0        UMETA(DisplayName = "Greeting"),
    Farewell = 1        UMETA(DisplayName = "Farewell"),
    Question = 2        UMETA(DisplayName = "Question"),
    Information = 3     UMETA(DisplayName = "Information"),
    Warning = 4         UMETA(DisplayName = "Warning"),
    Request = 5         UMETA(DisplayName = "Request"),
    Trade = 6           UMETA(DisplayName = "Trade"),
    Gossip = 7          UMETA(DisplayName = "Gossip"),
    Complaint = 8       UMETA(DisplayName = "Complaint"),
    Praise = 9          UMETA(DisplayName = "Praise"),
    Threat = 10         UMETA(DisplayName = "Threat"),
    Confession = 11     UMETA(DisplayName = "Confession")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENPC_DialogueType Type = ENPC_DialogueType::Greeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> Conditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable = true;

    FNPC_DialogueLine()
    {
        Type = ENPC_DialogueType::Greeting;
        Text = TEXT("");
        Priority = 1.0f;
        bIsRepeatable = true;
    }
};

// ═══════════════════════════════════════════════════════════════
// BEHAVIOR SCHEDULING AND ROUTINES
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_ScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartHour = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndHour = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_BehaviorState Activity = ENPC_BehaviorState::Working;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    int32 Priority = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    bool bIsFlexible = true;

    FNPC_ScheduleEntry()
    {
        StartHour = 8.0f;
        EndHour = 10.0f;
        Activity = ENPC_BehaviorState::Working;
        Location = FVector::ZeroVector;
        Description = TEXT("");
        Priority = 1;
        bIsFlexible = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    ENPC_Profession Profession = ENPC_Profession::Villager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TArray<ENPC_PersonalityTrait> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FNPC_SurvivalNeeds Needs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FNPC_EmotionalState Emotions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TArray<FNPC_ScheduleEntry> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TArray<FNPC_Relationship> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TArray<FNPC_DialogueLine> DialogueLines;

    FNPC_BehaviorProfile()
    {
        NPCName = TEXT("");
        Profession = ENPC_Profession::Villager;
    }
};