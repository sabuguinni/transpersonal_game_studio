#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "NPCBehaviorTypes.generated.h"

/**
 * NPC BEHAVIOR AGENT #11 - COMPREHENSIVE NPC BEHAVIOR TYPES
 * 
 * This file defines all NPC behavior types, personality traits, needs systems,
 * memory structures, and social dynamics for the prehistoric survival world.
 * 
 * DESIGN PHILOSOPHY:
 * - NPCs exist to live their own lives, not serve the player
 * - Emergent behavior is more memorable than scripted sequences
 * - Every NPC has survival needs, fears, and social relationships
 * - Behavior is driven by personality + current needs + environmental threats
 */

// ═══════════════════════════════════════════════════════════════
// NPC PERSONALITY AND BEHAVIOR TYPES
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Brave = 0           UMETA(DisplayName = "Brave"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Aggressive = 2      UMETA(DisplayName = "Aggressive"),
    Peaceful = 3        UMETA(DisplayName = "Peaceful"),
    Social = 4          UMETA(DisplayName = "Social"),
    Loner = 5           UMETA(DisplayName = "Loner"),
    Curious = 6         UMETA(DisplayName = "Curious"),
    Paranoid = 7        UMETA(DisplayName = "Paranoid"),
    Leader = 8          UMETA(DisplayName = "Leader"),
    Follower = 9        UMETA(DisplayName = "Follower")
};

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0            UMETA(DisplayName = "Idle"),
    Working = 1         UMETA(DisplayName = "Working"),
    Eating = 2          UMETA(DisplayName = "Eating"),
    Sleeping = 3        UMETA(DisplayName = "Sleeping"),
    Socializing = 4     UMETA(DisplayName = "Socializing"),
    Patrolling = 5      UMETA(DisplayName = "Patrolling"),
    Fleeing = 6         UMETA(DisplayName = "Fleeing"),
    Fighting = 7        UMETA(DisplayName = "Fighting"),
    Hunting = 8         UMETA(DisplayName = "Hunting"),
    Gathering = 9       UMETA(DisplayName = "Gathering"),
    Crafting = 10       UMETA(DisplayName = "Crafting"),
    Panicking = 11      UMETA(DisplayName = "Panicking")
};

UENUM(BlueprintType)
enum class ENPC_Profession : uint8
{
    Hunter = 0          UMETA(DisplayName = "Hunter"),
    Gatherer = 1        UMETA(DisplayName = "Gatherer"),
    Crafter = 2         UMETA(DisplayName = "Crafter"),
    Guard = 3           UMETA(DisplayName = "Guard"),
    Shaman = 4          UMETA(DisplayName = "Shaman"),
    Elder = 5           UMETA(DisplayName = "Elder"),
    Child = 6           UMETA(DisplayName = "Child"),
    Scout = 7           UMETA(DisplayName = "Scout"),
    Builder = 8         UMETA(DisplayName = "Builder")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None = 0            UMETA(DisplayName = "None"),
    Low = 1             UMETA(DisplayName = "Low"),
    Medium = 2          UMETA(DisplayName = "Medium"),
    High = 3            UMETA(DisplayName = "High"),
    Extreme = 4         UMETA(DisplayName = "Extreme")
};

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

// ═══════════════════════════════════════════════════════════════
// NPC NEEDS AND SURVIVAL SYSTEM
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Needs
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float Warmth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float Safety = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float Social = 100.0f;

    FNPC_Needs()
    {
        Hunger = 100.0f;
        Thirst = 100.0f;
        Energy = 100.0f;
        Warmth = 100.0f;
        Safety = 100.0f;
        Social = 100.0f;
    }

    // Get the most critical need (lowest value)
    float GetMostCriticalNeed() const
    {
        return FMath::Min({Hunger, Thirst, Energy, Warmth, Safety, Social});
    }

    // Check if any need is critically low
    bool HasCriticalNeed(float CriticalThreshold = 20.0f) const
    {
        return GetMostCriticalNeed() < CriticalThreshold;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_PersonalityProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<ENPC_PersonalityTrait> PrimaryTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Bravery = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Loyalty = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    ENPC_Profession Profession = ENPC_Profession::Gatherer;

    FNPC_PersonalityProfile()
    {
        Bravery = 0.5f;
        Sociability = 0.5f;
        Aggression = 0.5f;
        Curiosity = 0.5f;
        Loyalty = 0.5f;
        Profession = ENPC_Profession::Gatherer;
    }

    // Check if NPC has a specific trait
    bool HasTrait(ENPC_PersonalityTrait Trait) const
    {
        return PrimaryTraits.Contains(Trait);
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC MEMORY AND KNOWLEDGE SYSTEM
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EmotionalWeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* RelatedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;

    FNPC_Memory()
    {
        EventDescription = TEXT("");
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EmotionalWeight = 0.0f;
        RelatedActor = nullptr;
        ThreatLevel = ENPC_ThreatLevel::None;
    }

    // Check if memory is still relevant (not too old)
    bool IsRelevant(float CurrentTime, float MaxAge = 300.0f) const
    {
        return (CurrentTime - Timestamp) < MaxAge;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    ENPC_RelationshipType RelationshipType = ENPC_RelationshipType::Stranger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Trust = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Respect = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteractionTime = 0.0f;

    FNPC_Relationship()
    {
        TargetActor = nullptr;
        RelationshipType = ENPC_RelationshipType::Stranger;
        Trust = 0.0f;
        Respect = 0.0f;
        Fear = 0.0f;
        LastInteractionTime = 0.0f;
    }

    // Get overall relationship strength (positive or negative)
    float GetRelationshipStrength() const
    {
        return (Trust + Respect) - Fear;
    }

    // Check if this is a positive relationship
    bool IsPositiveRelationship() const
    {
        return GetRelationshipStrength() > 0.0f;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC DIALOGUE AND COMMUNICATION TYPES
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_DialogueType : uint8
{
    Greeting = 0        UMETA(DisplayName = "Greeting"),
    Warning = 1         UMETA(DisplayName = "Warning"),
    Request = 2         UMETA(DisplayName = "Request"),
    Information = 3     UMETA(DisplayName = "Information"),
    Threat = 4          UMETA(DisplayName = "Threat"),
    Farewell = 5        UMETA(DisplayName = "Farewell"),
    Panic = 6           UMETA(DisplayName = "Panic"),
    Celebration = 7     UMETA(DisplayName = "Celebration")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENPC_DialogueType DialogueType = ENPC_DialogueType::Greeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float EmotionalIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<ENPC_PersonalityTrait> RequiredTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENPC_Profession RequiredProfession = ENPC_Profession::Gatherer;

    FNPC_DialogueLine()
    {
        DialogueType = ENPC_DialogueType::Greeting;
        Text = TEXT("");
        EmotionalIntensity = 0.5f;
        RequiredProfession = ENPC_Profession::Gatherer;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC ROUTINE AND SCHEDULE SYSTEM
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_ScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartHour = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndHour = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_BehaviorState Activity = ENPC_BehaviorState::Working;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FString ActivityDescription;

    FNPC_ScheduleEntry()
    {
        StartHour = 8.0f;
        EndHour = 12.0f;
        Activity = ENPC_BehaviorState::Working;
        TargetLocation = FVector::ZeroVector;
        ActivityDescription = TEXT("");
    }

    // Check if this activity should be active at the given time
    bool IsActiveAtTime(float CurrentHour) const
    {
        if (StartHour <= EndHour)
        {
            return CurrentHour >= StartHour && CurrentHour < EndHour;
        }
        else
        {
            // Activity spans midnight
            return CurrentHour >= StartHour || CurrentHour < EndHour;
        }
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FNPC_ScheduleEntry> ScheduleEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector WorkLocation = FVector::ZeroVector;

    FNPC_DailySchedule()
    {
        HomeLocation = FVector::ZeroVector;
        WorkLocation = FVector::ZeroVector;
    }

    // Get the current activity for the given time
    FNPC_ScheduleEntry* GetCurrentActivity(float CurrentHour)
    {
        for (FNPC_ScheduleEntry& Entry : ScheduleEntries)
        {
            if (Entry.IsActiveAtTime(CurrentHour))
            {
                return &Entry;
            }
        }
        return nullptr;
    }
};