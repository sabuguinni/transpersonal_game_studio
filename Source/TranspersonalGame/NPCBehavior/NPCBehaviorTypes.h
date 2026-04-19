#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../Core/SharedTypes.h"
#include "NPCBehaviorTypes.generated.h"

/**
 * NPC BEHAVIOR SYSTEM - TYPE DEFINITIONS
 * NPC Behavior Agent #11
 * 
 * Comprehensive type system for NPC behavior, personality, and social dynamics.
 * Designed for realistic prehistoric tribal society simulation.
 */

// ═══════════════════════════════════════════════════════════════
// PERSONALITY & TRAITS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Aggressive = 0      UMETA(DisplayName = "Aggressive"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Curious = 2         UMETA(DisplayName = "Curious"),
    Loyal = 3           UMETA(DisplayName = "Loyal"),
    Independent = 4     UMETA(DisplayName = "Independent"),
    Social = 5          UMETA(DisplayName = "Social"),
    Territorial = 6     UMETA(DisplayName = "Territorial"),
    Protective = 7      UMETA(DisplayName = "Protective"),
    Resourceful = 8     UMETA(DisplayName = "Resourceful"),
    Fearful = 9         UMETA(DisplayName = "Fearful")
};

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0            UMETA(DisplayName = "Idle"),
    Working = 1         UMETA(DisplayName = "Working"),
    Patrolling = 2      UMETA(DisplayName = "Patrolling"),
    Hunting = 3         UMETA(DisplayName = "Hunting"),
    Gathering = 4       UMETA(DisplayName = "Gathering"),
    Socializing = 5     UMETA(DisplayName = "Socializing"),
    Resting = 6         UMETA(DisplayName = "Resting"),
    Eating = 7          UMETA(DisplayName = "Eating"),
    Fleeing = 8         UMETA(DisplayName = "Fleeing"),
    Fighting = 9        UMETA(DisplayName = "Fighting"),
    Following = 10      UMETA(DisplayName = "Following"),
    Investigating = 11  UMETA(DisplayName = "Investigating")
};

UENUM(BlueprintType)
enum class ENPC_Profession : uint8
{
    Hunter = 0          UMETA(DisplayName = "Hunter"),
    Gatherer = 1        UMETA(DisplayName = "Gatherer"),
    Crafter = 2         UMETA(DisplayName = "Crafter"),
    Guard = 3           UMETA(DisplayName = "Guard"),
    Elder = 4           UMETA(DisplayName = "Elder"),
    Healer = 5          UMETA(DisplayName = "Healer"),
    Scout = 6           UMETA(DisplayName = "Scout"),
    Child = 7           UMETA(DisplayName = "Child"),
    Trader = 8          UMETA(DisplayName = "Trader")
};

// ═══════════════════════════════════════════════════════════════
// NEEDS & MOTIVATIONS
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
    float Safety = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Comfort = 50.0f;

    FNPC_Needs()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Energy = 100.0f;
        Safety = 75.0f;
        Social = 50.0f;
        Comfort = 50.0f;
    }
};

// ═══════════════════════════════════════════════════════════════
// MEMORY SYSTEM
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_MemoryType : uint8
{
    Location = 0        UMETA(DisplayName = "Location"),
    Person = 1          UMETA(DisplayName = "Person"),
    Event = 2           UMETA(DisplayName = "Event"),
    Threat = 3          UMETA(DisplayName = "Threat"),
    Resource = 4        UMETA(DisplayName = "Resource"),
    Interaction = 5     UMETA(DisplayName = "Interaction")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_MemoryType Type = ENPC_MemoryType::Event;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float DecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* RelatedActor = nullptr;

    FNPC_Memory()
    {
        Type = ENPC_MemoryType::Event;
        Location = FVector::ZeroVector;
        Description = TEXT("");
        Importance = 1.0f;
        Timestamp = 0.0f;
        DecayRate = 0.1f;
        RelatedActor = nullptr;
    }
};

// ═══════════════════════════════════════════════════════════════
// RELATIONSHIP SYSTEM
// ═══════════════════════════════════════════════════════════════

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

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    AActor* TargetActor = nullptr;

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

    FNPC_Relationship()
    {
        TargetActor = nullptr;
        Type = ENPC_RelationshipType::Stranger;
        Affection = 0.0f;
        Trust = 50.0f;
        Respect = 50.0f;
        LastInteraction = 0.0f;
    }
};

// ═══════════════════════════════════════════════════════════════
// DIALOGUE SYSTEM
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_DialogueType : uint8
{
    Greeting = 0        UMETA(DisplayName = "Greeting"),
    Information = 1     UMETA(DisplayName = "Information"),
    Quest = 2           UMETA(DisplayName = "Quest"),
    Trade = 3           UMETA(DisplayName = "Trade"),
    Warning = 4         UMETA(DisplayName = "Warning"),
    Gossip = 5          UMETA(DisplayName = "Gossip"),
    Farewell = 6        UMETA(DisplayName = "Farewell"),
    Combat = 7          UMETA(DisplayName = "Combat")
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
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> Conditions;

    FNPC_DialogueLine()
    {
        Type = ENPC_DialogueType::Greeting;
        Text = TEXT("");
        AudioPath = TEXT("");
        Priority = 1.0f;
        Conditions.Empty();
    }
};

// ═══════════════════════════════════════════════════════════════
// BEHAVIOR TREE DATA
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState PreviousState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsInterruptible = true;

    FNPC_BehaviorData()
    {
        CurrentState = ENPC_BehaviorState::Idle;
        PreviousState = ENPC_BehaviorState::Idle;
        StateTimer = 0.0f;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        AlertLevel = 0.0f;
        bIsInterruptible = true;
    }
};

// ═══════════════════════════════════════════════════════════════
// SCHEDULE SYSTEM
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_ScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndTime = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_BehaviorState Activity = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float Priority = 1.0f;

    FNPC_ScheduleEntry()
    {
        StartTime = 0.0f;
        EndTime = 24.0f;
        Activity = ENPC_BehaviorState::Idle;
        Location = FVector::ZeroVector;
        Priority = 1.0f;
    }
};

// ═══════════════════════════════════════════════════════════════
// COMBAT BEHAVIOR
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_CombatRole : uint8
{
    Warrior = 0         UMETA(DisplayName = "Warrior"),
    Archer = 1          UMETA(DisplayName = "Archer"),
    Support = 2         UMETA(DisplayName = "Support"),
    Civilian = 3        UMETA(DisplayName = "Civilian")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_CombatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ENPC_CombatRole Role = ENPC_CombatRole::Civilian;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Courage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Skill = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PreferredRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bWillFight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bWillFlee = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeThreshold = 25.0f;

    FNPC_CombatData()
    {
        Role = ENPC_CombatRole::Civilian;
        Courage = 50.0f;
        Skill = 50.0f;
        PreferredRange = 200.0f;
        bWillFight = true;
        bWillFlee = true;
        FleeThreshold = 25.0f;
    }
};