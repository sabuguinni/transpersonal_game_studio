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
 * Defines personality traits, behavior states, needs, memory, and relationships.
 * Focus: Realistic human tribal behavior in dangerous prehistoric world.
 */

// ═══════════════════════════════════════════════════════════════
// NPC PERSONALITY TRAITS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Brave = 0           UMETA(DisplayName = "Brave"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Aggressive = 2      UMETA(DisplayName = "Aggressive"),
    Peaceful = 3        UMETA(DisplayName = "Peaceful"),
    Curious = 4         UMETA(DisplayName = "Curious"),
    Withdrawn = 5       UMETA(DisplayName = "Withdrawn"),
    Leader = 6          UMETA(DisplayName = "Leader"),
    Follower = 7        UMETA(DisplayName = "Follower"),
    Protective = 8      UMETA(DisplayName = "Protective"),
    Selfish = 9         UMETA(DisplayName = "Selfish")
};

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0            UMETA(DisplayName = "Idle"),
    Patrolling = 1      UMETA(DisplayName = "Patrolling"),
    Hunting = 2         UMETA(DisplayName = "Hunting"),
    Gathering = 3       UMETA(DisplayName = "Gathering"),
    Crafting = 4        UMETA(DisplayName = "Crafting"),
    Socializing = 5     UMETA(DisplayName = "Socializing"),
    Sleeping = 6        UMETA(DisplayName = "Sleeping"),
    Fleeing = 7         UMETA(DisplayName = "Fleeing"),
    Fighting = 8        UMETA(DisplayName = "Fighting"),
    Investigating = 9   UMETA(DisplayName = "Investigating"),
    Following = 10      UMETA(DisplayName = "Following"),
    Guarding = 11       UMETA(DisplayName = "Guarding")
};

UENUM(BlueprintType)
enum class ENPC_Profession : uint8
{
    Hunter = 0          UMETA(DisplayName = "Hunter"),
    Gatherer = 1        UMETA(DisplayName = "Gatherer"),
    Crafter = 2         UMETA(DisplayName = "Crafter"),
    Guard = 3           UMETA(DisplayName = "Guard"),
    Scout = 4           UMETA(DisplayName = "Scout"),
    Healer = 5          UMETA(DisplayName = "Healer"),
    Elder = 6           UMETA(DisplayName = "Elder"),
    Child = 7           UMETA(DisplayName = "Child"),
    Chieftain = 8       UMETA(DisplayName = "Chieftain")
};

// ═══════════════════════════════════════════════════════════════
// NPC NEEDS SYSTEM
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
    float Warmth = 50.0f;

    FNPC_Needs()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Energy = 100.0f;
        Safety = 75.0f;
        Social = 50.0f;
        Warmth = 50.0f;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC MEMORY SYSTEM
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_MemoryType : uint8
{
    DangerLocation = 0      UMETA(DisplayName = "Danger Location"),
    SafeLocation = 1        UMETA(DisplayName = "Safe Location"),
    ResourceLocation = 2    UMETA(DisplayName = "Resource Location"),
    PersonEncounter = 3     UMETA(DisplayName = "Person Encounter"),
    DinosaurSighting = 4    UMETA(DisplayName = "Dinosaur Sighting"),
    ImportantEvent = 5      UMETA(DisplayName = "Important Event")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_MemoryType MemoryType = ENPC_MemoryType::ImportantEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float DecayRate = 0.1f;

    FNPC_Memory()
    {
        MemoryType = ENPC_MemoryType::ImportantEvent;
        Location = FVector::ZeroVector;
        Description = TEXT("");
        Importance = 1.0f;
        TimeStamp = 0.0f;
        DecayRate = 0.1f;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC RELATIONSHIP SYSTEM
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_RelationshipType : uint8
{
    Stranger = 0        UMETA(DisplayName = "Stranger"),
    Acquaintance = 1    UMETA(DisplayName = "Acquaintance"),
    Friend = 2          UMETA(DisplayName = "Friend"),
    Enemy = 3           UMETA(DisplayName = "Enemy"),
    Family = 4          UMETA(DisplayName = "Family"),
    Mate = 5            UMETA(DisplayName = "Mate"),
    Rival = 6           UMETA(DisplayName = "Rival"),
    Leader = 7          UMETA(DisplayName = "Leader"),
    Follower = 8        UMETA(DisplayName = "Follower")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    ENPC_RelationshipType RelationType = ENPC_RelationshipType::Stranger;

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
        RelationType = ENPC_RelationshipType::Stranger;
        Affection = 0.0f;
        Trust = 50.0f;
        Respect = 50.0f;
        LastInteraction = 0.0f;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC DIALOGUE SYSTEM
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
    Emergency = 7       UMETA(DisplayName = "Emergency")
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
    TArray<ENPC_PersonalityTrait> RequiredTraits;

    FNPC_DialogueLine()
    {
        DialogueType = ENPC_DialogueType::Greeting;
        Text = TEXT("");
        AudioPath = TEXT("");
        Priority = 1.0f;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC TASK SYSTEM
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_TaskType : uint8
{
    MoveTo = 0          UMETA(DisplayName = "Move To"),
    Interact = 1        UMETA(DisplayName = "Interact"),
    Gather = 2          UMETA(DisplayName = "Gather"),
    Craft = 3           UMETA(DisplayName = "Craft"),
    Hunt = 4            UMETA(DisplayName = "Hunt"),
    Guard = 5           UMETA(DisplayName = "Guard"),
    Follow = 6          UMETA(DisplayName = "Follow"),
    Wait = 7            UMETA(DisplayName = "Wait"),
    Flee = 8            UMETA(DisplayName = "Flee"),
    Attack = 9          UMETA(DisplayName = "Attack")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Task
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    ENPC_TaskType TaskType = ENPC_TaskType::Wait;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsCompleted = false;

    FNPC_Task()
    {
        TaskType = ENPC_TaskType::Wait;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        Priority = 1.0f;
        Duration = 0.0f;
        bIsCompleted = false;
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC PERCEPTION DATA
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_PerceptionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float AlertnessLevel = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    TArray<TWeakObjectPtr<AActor>> VisibleActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    TArray<TWeakObjectPtr<AActor>> HeardActors;

    FNPC_PerceptionData()
    {
        SightRange = 1500.0f;
        HearingRange = 800.0f;
        SightAngle = 90.0f;
        AlertnessLevel = 50.0f;
    }
};