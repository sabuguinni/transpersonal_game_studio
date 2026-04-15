#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../Core/SharedTypes.h"
#include "NPCBehaviorTypes.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - NPC BEHAVIOR TYPES
 * NPC Behavior Agent #11
 * 
 * Defines all shared types for the NPC behavior system.
 * NPCs are autonomous agents with daily routines, memory, and social dynamics.
 */

// ═══════════════════════════════════════════════════════════════
// NPC BEHAVIOR ENUMS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0            UMETA(DisplayName = "Idle"),
    Wandering = 1       UMETA(DisplayName = "Wandering"),
    Working = 2         UMETA(DisplayName = "Working"),
    Socializing = 3     UMETA(DisplayName = "Socializing"),
    Eating = 4          UMETA(DisplayName = "Eating"),
    Sleeping = 5        UMETA(DisplayName = "Sleeping"),
    Fleeing = 6         UMETA(DisplayName = "Fleeing"),
    Investigating = 7   UMETA(DisplayName = "Investigating"),
    Following = 8       UMETA(DisplayName = "Following"),
    Guarding = 9        UMETA(DisplayName = "Guarding")
};

UENUM(BlueprintType)
enum class ENPC_Profession : uint8
{
    Gatherer = 0        UMETA(DisplayName = "Gatherer"),
    Hunter = 1          UMETA(DisplayName = "Hunter"),
    Crafter = 2         UMETA(DisplayName = "Crafter"),
    Guard = 3           UMETA(DisplayName = "Guard"),
    Trader = 4          UMETA(DisplayName = "Trader"),
    Healer = 5          UMETA(DisplayName = "Healer"),
    Scout = 6           UMETA(DisplayName = "Scout"),
    Elder = 7           UMETA(DisplayName = "Elder"),
    Child = 8           UMETA(DisplayName = "Child"),
    Shaman = 9          UMETA(DisplayName = "Shaman")
};

UENUM(BlueprintType)
enum class ENPC_Mood : uint8
{
    Content = 0         UMETA(DisplayName = "Content"),
    Happy = 1           UMETA(DisplayName = "Happy"),
    Sad = 2             UMETA(DisplayName = "Sad"),
    Angry = 3           UMETA(DisplayName = "Angry"),
    Fearful = 4         UMETA(DisplayName = "Fearful"),
    Excited = 5         UMETA(DisplayName = "Excited"),
    Tired = 6           UMETA(DisplayName = "Tired"),
    Hungry = 7          UMETA(DisplayName = "Hungry"),
    Sick = 8            UMETA(DisplayName = "Sick"),
    Aggressive = 9      UMETA(DisplayName = "Aggressive")
};

UENUM(BlueprintType)
enum class ENPC_Relationship : uint8
{
    Stranger = 0        UMETA(DisplayName = "Stranger"),
    Acquaintance = 1    UMETA(DisplayName = "Acquaintance"),
    Friend = 2          UMETA(DisplayName = "Friend"),
    Enemy = 3           UMETA(DisplayName = "Enemy"),
    Family = 4          UMETA(DisplayName = "Family"),
    Mate = 5            UMETA(DisplayName = "Mate"),
    Leader = 6          UMETA(DisplayName = "Leader"),
    Follower = 7        UMETA(DisplayName = "Follower"),
    Rival = 8           UMETA(DisplayName = "Rival"),
    Neutral = 9         UMETA(DisplayName = "Neutral")
};

UENUM(BlueprintType)
enum class ENPC_ActivityType : uint8
{
    Rest = 0            UMETA(DisplayName = "Rest"),
    Work = 1            UMETA(DisplayName = "Work"),
    Eat = 2             UMETA(DisplayName = "Eat"),
    Socialize = 3       UMETA(DisplayName = "Socialize"),
    Patrol = 4          UMETA(DisplayName = "Patrol"),
    Craft = 5           UMETA(DisplayName = "Craft"),
    Hunt = 6            UMETA(DisplayName = "Hunt"),
    Gather = 7          UMETA(DisplayName = "Gather"),
    Trade = 8           UMETA(DisplayName = "Trade"),
    Ritual = 9          UMETA(DisplayName = "Ritual")
};

// ═══════════════════════════════════════════════════════════════
// NPC BEHAVIOR STRUCTS
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Stats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Thirst = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Happiness = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Energy = 100.0f;

    FNPC_Stats()
    {
        Health = 100.0f;
        Stamina = 100.0f;
        Hunger = 0.0f;
        Thirst = 0.0f;
        Happiness = 75.0f;
        Fear = 0.0f;
        Energy = 100.0f;
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
    float EmotionalWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FString> InvolvedActors;

    FNPC_Memory()
    {
        EventDescription = TEXT("");
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EmotionalWeight = 0.0f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Affection = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Trust = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Respect = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteraction = 0.0f;

    FNPC_Relationship()
    {
        TargetName = TEXT("");
        RelationshipType = ENPC_Relationship::Stranger;
        Affection = 0.0f;
        Trust = 0.0f;
        Respect = 0.0f;
        LastInteraction = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartHour = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndHour = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_ActivityType Activity = ENPC_ActivityType::Work;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector PreferredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float Priority = 1.0f;

    FNPC_DailySchedule()
    {
        StartHour = 6.0f;
        EndHour = 8.0f;
        Activity = ENPC_ActivityType::Work;
        PreferredLocation = FVector::ZeroVector;
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Personality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Courage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggression = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Loyalty = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Intelligence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Patience = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Empathy = 0.5f;

    FNPC_Personality()
    {
        Courage = 0.5f;
        Sociability = 0.5f;
        Curiosity = 0.5f;
        Aggression = 0.2f;
        Loyalty = 0.7f;
        Intelligence = 0.5f;
        Patience = 0.5f;
        Empathy = 0.5f;
    }
};