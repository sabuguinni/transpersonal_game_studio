#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "NPCBehaviorTypes.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - NPC BEHAVIOR TYPES
 * NPC Behavior Agent #11
 * 
 * Defines all shared types for the NPC behavior system.
 * This includes personality traits, behavior states, needs, and social dynamics.
 */

// ═══════════════════════════════════════════════════════════════
// NPC PERSONALITY & TRAITS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Aggressive = 0      UMETA(DisplayName = "Aggressive"),
    Peaceful = 1        UMETA(DisplayName = "Peaceful"),
    Curious = 2         UMETA(DisplayName = "Curious"),
    Cautious = 3        UMETA(DisplayName = "Cautious"),
    Social = 4          UMETA(DisplayName = "Social"),
    Solitary = 5        UMETA(DisplayName = "Solitary"),
    Brave = 6           UMETA(DisplayName = "Brave"),
    Cowardly = 7        UMETA(DisplayName = "Cowardly"),
    Intelligent = 8     UMETA(DisplayName = "Intelligent"),
    Simple = 9          UMETA(DisplayName = "Simple")
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
    Crafting = 10       UMETA(DisplayName = "Crafting"),
    Trading = 11        UMETA(DisplayName = "Trading")
};

UENUM(BlueprintType)
enum class ENPC_Profession : uint8
{
    None = 0            UMETA(DisplayName = "None"),
    Gatherer = 1        UMETA(DisplayName = "Gatherer"),
    Hunter = 2          UMETA(DisplayName = "Hunter"),
    Crafter = 3         UMETA(DisplayName = "Crafter"),
    Guard = 4           UMETA(DisplayName = "Guard"),
    Shaman = 5          UMETA(DisplayName = "Shaman"),
    Trader = 6          UMETA(DisplayName = "Trader"),
    Scout = 7           UMETA(DisplayName = "Scout"),
    Elder = 8           UMETA(DisplayName = "Elder"),
    Child = 9           UMETA(DisplayName = "Child")
};

// ═══════════════════════════════════════════════════════════════
// NPC NEEDS & DRIVES
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Needs
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Comfort = 50.0f;

    FNPC_Needs()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Energy = 100.0f;
        Safety = 50.0f;
        Social = 50.0f;
        Comfort = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Personality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<ENPC_PersonalityTrait> Traits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Courage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    FNPC_Personality()
    {
        Traits.Empty();
        Aggression = 0.5f;
        Curiosity = 0.5f;
        Sociability = 0.5f;
        Courage = 0.5f;
        Intelligence = 0.5f;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC MEMORY & RELATIONSHIPS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_RelationshipType : uint8
{
    Neutral = 0         UMETA(DisplayName = "Neutral"),
    Friend = 1          UMETA(DisplayName = "Friend"),
    Enemy = 2           UMETA(DisplayName = "Enemy"),
    Family = 3          UMETA(DisplayName = "Family"),
    Romantic = 4        UMETA(DisplayName = "Romantic"),
    Rival = 5           UMETA(DisplayName = "Rival"),
    Mentor = 6          UMETA(DisplayName = "Mentor"),
    Student = 7         UMETA(DisplayName = "Student"),
    Leader = 8          UMETA(DisplayName = "Leader"),
    Follower = 9        UMETA(DisplayName = "Follower")
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
    float EmotionalIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FString> InvolvedActors;

    FNPC_Memory()
    {
        EventDescription = TEXT("");
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EmotionalIntensity = 0.0f;
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
    ENPC_RelationshipType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
    float Affection = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Trust = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Respect = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteraction;

    FNPC_Relationship()
    {
        TargetName = TEXT("");
        Type = ENPC_RelationshipType::Neutral;
        Affection = 0.0f;
        Trust = 50.0f;
        Respect = 50.0f;
        LastInteraction = 0.0f;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC SCHEDULE & ROUTINES
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_ScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_BehaviorState Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FString ActivityDescription;

    FNPC_ScheduleEntry()
    {
        StartHour = 8.0f;
        EndHour = 12.0f;
        Activity = ENPC_BehaviorState::Working;
        TargetLocation = FVector::ZeroVector;
        ActivityDescription = TEXT("Default Activity");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FNPC_ScheduleEntry> Schedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    bool bUseRandomVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float VariationRange;

    FNPC_DailySchedule()
    {
        Schedule.Empty();
        bUseRandomVariation = true;
        VariationRange = 0.5f;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC DIALOGUE & COMMUNICATION
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_DialogueType : uint8
{
    Greeting = 0        UMETA(DisplayName = "Greeting"),
    Farewell = 1        UMETA(DisplayName = "Farewell"),
    Question = 2        UMETA(DisplayName = "Question"),
    Answer = 3          UMETA(DisplayName = "Answer"),
    Warning = 4         UMETA(DisplayName = "Warning"),
    Request = 5         UMETA(DisplayName = "Request"),
    Trade = 6           UMETA(DisplayName = "Trade"),
    Gossip = 7          UMETA(DisplayName = "Gossip"),
    Complaint = 8       UMETA(DisplayName = "Complaint"),
    Praise = 9          UMETA(DisplayName = "Praise")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENPC_DialogueType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float EmotionalTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Cooldown;

    FNPC_DialogueLine()
    {
        Type = ENPC_DialogueType::Greeting;
        Text = TEXT("Hello there.");
        EmotionalTone = 0.0f;
        RequiredConditions.Empty();
        Cooldown = 30.0f;
    }
};