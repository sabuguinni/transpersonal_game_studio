#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NPCBehaviorTypes.generated.h"

/**
 * NPC BEHAVIOR AGENT #11 - CORE BEHAVIOR TYPES
 * 
 * This file defines all fundamental NPC behavior types for the prehistoric survival game.
 * Focus: Realistic survival behaviors, tribal dynamics, and dinosaur AI patterns.
 * NO spiritual/mystical content - pure survival and social dynamics.
 */

// ═══════════════════════════════════════════════════════════════
// NPC PERSONALITY AND BEHAVIOR TRAITS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Aggressive = 0      UMETA(DisplayName = "Aggressive"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Curious = 2         UMETA(DisplayName = "Curious"),
    Protective = 3      UMETA(DisplayName = "Protective"),
    Territorial = 4     UMETA(DisplayName = "Territorial"),
    Social = 5          UMETA(DisplayName = "Social"),
    Solitary = 6        UMETA(DisplayName = "Solitary"),
    Leader = 7          UMETA(DisplayName = "Leader"),
    Follower = 8        UMETA(DisplayName = "Follower"),
    Survivor = 9        UMETA(DisplayName = "Survivor")
};

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0            UMETA(DisplayName = "Idle"),
    Patrolling = 1      UMETA(DisplayName = "Patrolling"),
    Hunting = 2         UMETA(DisplayName = "Hunting"),
    Feeding = 3         UMETA(DisplayName = "Feeding"),
    Sleeping = 4        UMETA(DisplayName = "Sleeping"),
    Alert = 5           UMETA(DisplayName = "Alert"),
    Fleeing = 6         UMETA(DisplayName = "Fleeing"),
    Fighting = 7        UMETA(DisplayName = "Fighting"),
    Socializing = 8     UMETA(DisplayName = "Socializing"),
    Working = 9         UMETA(DisplayName = "Working"),
    Investigating = 10  UMETA(DisplayName = "Investigating"),
    Following = 11      UMETA(DisplayName = "Following")
};

UENUM(BlueprintType)
enum class ENPC_Profession : uint8
{
    Hunter = 0          UMETA(DisplayName = "Hunter"),
    Gatherer = 1        UMETA(DisplayName = "Gatherer"),
    Crafter = 2         UMETA(DisplayName = "Crafter"),
    Scout = 3           UMETA(DisplayName = "Scout"),
    Guard = 4           UMETA(DisplayName = "Guard"),
    Healer = 5          UMETA(DisplayName = "Healer"),
    Elder = 6           UMETA(DisplayName = "Elder"),
    Child = 7           UMETA(DisplayName = "Child"),
    Shaman = 8          UMETA(DisplayName = "Shaman")
};

// ═══════════════════════════════════════════════════════════════
// NPC NEEDS AND SURVIVAL SYSTEMS
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SurvivalNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Warmth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Safety = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Social = 50.0f;

    FNPC_SurvivalNeeds()
    {
        Hunger = 100.0f;
        Thirst = 100.0f;
        Energy = 100.0f;
        Warmth = 100.0f;
        Safety = 100.0f;
        Social = 50.0f;
    }
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None = 0            UMETA(DisplayName = "None"),
    Low = 1             UMETA(DisplayName = "Low"),
    Medium = 2          UMETA(DisplayName = "Medium"),
    High = 3            UMETA(DisplayName = "High"),
    Critical = 4        UMETA(DisplayName = "Critical")
};

// ═══════════════════════════════════════════════════════════════
// NPC MEMORY AND AWARENESS SYSTEM
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        EventType = TEXT("");
        Timestamp = 0.0f;
        Importance = 1.0f;
        ThreatLevel = ENPC_ThreatLevel::None;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_AwarenessData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    float SightRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    float HearingRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    float SightAngle = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    float AlertnessLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    TArray<FNPC_MemoryEntry> RecentMemories;

    FNPC_AwarenessData()
    {
        SightRange = 1000.0f;
        HearingRange = 500.0f;
        SightAngle = 120.0f;
        AlertnessLevel = 0.5f;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC RELATIONSHIP AND SOCIAL SYSTEM
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_RelationshipType : uint8
{
    Stranger = 0        UMETA(DisplayName = "Stranger"),
    Acquaintance = 1    UMETA(DisplayName = "Acquaintance"),
    Friend = 2          UMETA(DisplayName = "Friend"),
    Ally = 3            UMETA(DisplayName = "Ally"),
    Family = 4          UMETA(DisplayName = "Family"),
    Enemy = 5           UMETA(DisplayName = "Enemy"),
    Rival = 6           UMETA(DisplayName = "Rival"),
    Leader = 7          UMETA(DisplayName = "Leader"),
    Follower = 8        UMETA(DisplayName = "Follower")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    FString TargetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    ENPC_RelationshipType RelationType = ENPC_RelationshipType::Stranger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Trust = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Respect = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteraction = 0.0f;

    FNPC_Relationship()
    {
        TargetName = TEXT("");
        RelationType = ENPC_RelationshipType::Stranger;
        Trust = 0.0f;
        Respect = 0.0f;
        Fear = 0.0f;
        LastInteraction = 0.0f;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC DIALOGUE AND COMMUNICATION SYSTEM
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_DialogueType : uint8
{
    Greeting = 0        UMETA(DisplayName = "Greeting"),
    Warning = 1         UMETA(DisplayName = "Warning"),
    Request = 2         UMETA(DisplayName = "Request"),
    Information = 3     UMETA(DisplayName = "Information"),
    Trade = 4           UMETA(DisplayName = "Trade"),
    Threat = 5          UMETA(DisplayName = "Threat"),
    Farewell = 6        UMETA(DisplayName = "Farewell"),
    Combat = 7          UMETA(DisplayName = "Combat"),
    Pain = 8            UMETA(DisplayName = "Pain"),
    Death = 9           UMETA(DisplayName = "Death")
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
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresResponse = false;

    FNPC_DialogueLine()
    {
        DialogueType = ENPC_DialogueType::Greeting;
        Text = TEXT("");
        AudioPath = TEXT("");
        Priority = 1.0f;
        bRequiresResponse = false;
    }
};

// ═══════════════════════════════════════════════════════════════
// DINOSAUR AI SPECIFIC TYPES
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex = 0            UMETA(DisplayName = "T-Rex"),
    Raptor = 1          UMETA(DisplayName = "Raptor"),
    Triceratops = 2     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus = 3   UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus = 4     UMETA(DisplayName = "Stegosaurus"),
    Pteranodon = 5      UMETA(DisplayName = "Pteranodon"),
    Compsognathus = 6   UMETA(DisplayName = "Compsognathus"),
    Ankylosaurus = 7    UMETA(DisplayName = "Ankylosaurus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurBehavior : uint8
{
    Grazing = 0         UMETA(DisplayName = "Grazing"),
    Hunting = 1         UMETA(DisplayName = "Hunting"),
    Patrolling = 2      UMETA(DisplayName = "Patrolling"),
    Resting = 3         UMETA(DisplayName = "Resting"),
    Migrating = 4       UMETA(DisplayName = "Migrating"),
    Territorial = 5     UMETA(DisplayName = "Territorial"),
    Mating = 6          UMETA(DisplayName = "Mating"),
    Nesting = 7         UMETA(DisplayName = "Nesting"),
    Fleeing = 8         UMETA(DisplayName = "Fleeing"),
    Aggressive = 9      UMETA(DisplayName = "Aggressive")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DinosaurTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float Curiosity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float Territoriality = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float PackInstinct = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float HuntingSkill = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float PatrolRadius = 2000.0f;

    FNPC_DinosaurTraits()
    {
        Species = ENPC_DinosaurSpecies::TRex;
        Aggression = 0.5f;
        Curiosity = 0.3f;
        Territoriality = 0.7f;
        PackInstinct = 0.2f;
        HuntingSkill = 0.6f;
        PatrolRadius = 2000.0f;
    }
};