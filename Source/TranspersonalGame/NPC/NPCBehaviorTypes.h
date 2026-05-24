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
 * Comprehensive NPC behavior system for prehistoric survival game.
 * Defines personality traits, behavior states, professions, needs, memory, and relationships.
 */

// ═══════════════════════════════════════════════════════════════
// NPC PERSONALITY AND BEHAVIOR ENUMS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Aggressive = 0      UMETA(DisplayName = "Aggressive"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Curious = 2         UMETA(DisplayName = "Curious"),
    Territorial = 3     UMETA(DisplayName = "Territorial"),
    Social = 4          UMETA(DisplayName = "Social"),
    Solitary = 5        UMETA(DisplayName = "Solitary"),
    Protective = 6      UMETA(DisplayName = "Protective"),
    Opportunistic = 7   UMETA(DisplayName = "Opportunistic"),
    Fearful = 8         UMETA(DisplayName = "Fearful"),
    Dominant = 9        UMETA(DisplayName = "Dominant")
};

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0            UMETA(DisplayName = "Idle"),
    Patrolling = 1      UMETA(DisplayName = "Patrolling"),
    Hunting = 2         UMETA(DisplayName = "Hunting"),
    Feeding = 3         UMETA(DisplayName = "Feeding"),
    Sleeping = 4        UMETA(DisplayName = "Sleeping"),
    Fleeing = 5         UMETA(DisplayName = "Fleeing"),
    Fighting = 6        UMETA(DisplayName = "Fighting"),
    Socializing = 7     UMETA(DisplayName = "Socializing"),
    Investigating = 8   UMETA(DisplayName = "Investigating"),
    Migrating = 9       UMETA(DisplayName = "Migrating"),
    Nesting = 10        UMETA(DisplayName = "Nesting"),
    Alert = 11          UMETA(DisplayName = "Alert")
};

UENUM(BlueprintType)
enum class ENPC_Profession : uint8
{
    Hunter = 0          UMETA(DisplayName = "Hunter"),
    Gatherer = 1        UMETA(DisplayName = "Gatherer"),
    Crafter = 2         UMETA(DisplayName = "Crafter"),
    Healer = 3          UMETA(DisplayName = "Healer"),
    Scout = 4           UMETA(DisplayName = "Scout"),
    Elder = 5           UMETA(DisplayName = "Elder"),
    Warrior = 6         UMETA(DisplayName = "Warrior"),
    Child = 7           UMETA(DisplayName = "Child"),
    Shaman = 8          UMETA(DisplayName = "Shaman")
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
    Neutral = 0         UMETA(DisplayName = "Neutral"),
    Friendly = 1        UMETA(DisplayName = "Friendly"),
    Hostile = 2         UMETA(DisplayName = "Hostile"),
    Family = 3          UMETA(DisplayName = "Family"),
    Mate = 4            UMETA(DisplayName = "Mate"),
    Rival = 5           UMETA(DisplayName = "Rival"),
    Leader = 6          UMETA(DisplayName = "Leader"),
    Follower = 7        UMETA(DisplayName = "Follower")
};

UENUM(BlueprintType)
enum class ENPC_DialogueType : uint8
{
    Greeting = 0        UMETA(DisplayName = "Greeting"),
    Trade = 1           UMETA(DisplayName = "Trade"),
    Quest = 2           UMETA(DisplayName = "Quest"),
    Information = 3     UMETA(DisplayName = "Information"),
    Warning = 4         UMETA(DisplayName = "Warning"),
    Goodbye = 5         UMETA(DisplayName = "Goodbye"),
    Threat = 6          UMETA(DisplayName = "Threat"),
    Help = 7            UMETA(DisplayName = "Help")
};

// ═══════════════════════════════════════════════════════════════
// NPC NEEDS AND SURVIVAL STRUCTS
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Needs
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float Thirst = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float Safety = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float Social = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
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

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastSeenPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenPlayerTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> DangerousLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> SafeLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> FoodLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> WaterLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryRetentionTime = 300.0f; // 5 minutes

    FNPC_Memory()
    {
        LastSeenPlayerLocation = FVector::ZeroVector;
        LastSeenPlayerTime = 0.0f;
        MemoryRetentionTime = 300.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    ENPC_RelationshipType RelationType = ENPC_RelationshipType::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float RelationshipStrength = 0.0f; // -100 to 100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    int32 InteractionCount = 0;

    FNPC_Relationship()
    {
        TargetActor = nullptr;
        RelationType = ENPC_RelationshipType::Neutral;
        RelationshipStrength = 0.0f;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC BEHAVIOR DATA STRUCTS
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
    float StateChangeTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    FNPC_BehaviorData()
    {
        CurrentState = ENPC_BehaviorState::Idle;
        PreviousState = ENPC_BehaviorState::Idle;
        StateChangeTime = 0.0f;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        MovementSpeed = 300.0f;
        DetectionRange = 1500.0f;
        AttackRange = 200.0f;
        CurrentThreatLevel = ENPC_ThreatLevel::None;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_PersonalityProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<ENPC_PersonalityTrait> PrimaryTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    ENPC_Profession Profession = ENPC_Profession::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggressiveness = 50.0f; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Fearfulness = 50.0f; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Sociability = 50.0f; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Intelligence = 50.0f; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Loyalty = 50.0f; // 0-100

    FNPC_PersonalityProfile()
    {
        Profession = ENPC_Profession::Hunter;
        Aggressiveness = 50.0f;
        Fearfulness = 50.0f;
        Sociability = 50.0f;
        Intelligence = 50.0f;
        Loyalty = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DialogueData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENPC_DialogueType DialogueType = ENPC_DialogueType::Greeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueCooldown = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanRepeat = true;

    FNPC_DialogueData()
    {
        DialogueType = ENPC_DialogueType::Greeting;
        DialogueText = TEXT("");
        DialogueCooldown = 30.0f;
        bCanRepeat = true;
    }
};