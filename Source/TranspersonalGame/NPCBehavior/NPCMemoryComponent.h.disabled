#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "../Core/SharedTypes.h"
#include "NPCMemoryComponent.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - NPC BEHAVIOR AGENT #11
 * Advanced NPC Memory System Component
 * 
 * This component gives NPCs persistent memory of events, relationships,
 * and emotional states. NPCs remember player interactions, form opinions,
 * and modify their behavior based on past experiences.
 * 
 * Core Philosophy: Every NPC has a life beyond the player's presence.
 */

// ═══════════════════════════════════════════════════════════════
// NPC MEMORY ENUMS
// ═══════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class ENPC_EmotionType : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Happy           UMETA(DisplayName = "Happy"),
    Sad             UMETA(DisplayName = "Sad"),
    Angry           UMETA(DisplayName = "Angry"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Excited         UMETA(DisplayName = "Excited"),
    Suspicious      UMETA(DisplayName = "Suspicious"),
    Trusting        UMETA(DisplayName = "Trusting"),
    Curious         UMETA(DisplayName = "Curious"),
    Protective      UMETA(DisplayName = "Protective")
};

UENUM(BlueprintType)
enum class ENPC_MemoryType : uint8
{
    PlayerInteraction   UMETA(DisplayName = "Player Interaction"),
    NPCInteraction      UMETA(DisplayName = "NPC Interaction"),
    LocationEvent       UMETA(DisplayName = "Location Event"),
    CombatEvent         UMETA(DisplayName = "Combat Event"),
    QuestEvent          UMETA(DisplayName = "Quest Event"),
    EmotionalEvent      UMETA(DisplayName = "Emotional Event"),
    TradeEvent          UMETA(DisplayName = "Trade Event"),
    RitualEvent         UMETA(DisplayName = "Ritual Event")
};

UENUM(BlueprintType)
enum class ENPC_RelationshipType : uint8
{
    Stranger        UMETA(DisplayName = "Stranger"),
    Acquaintance    UMETA(DisplayName = "Acquaintance"),
    Friend          UMETA(DisplayName = "Friend"),
    BestFriend      UMETA(DisplayName = "Best Friend"),
    Enemy           UMETA(DisplayName = "Enemy"),
    Rival           UMETA(DisplayName = "Rival"),
    Family          UMETA(DisplayName = "Family"),
    Lover           UMETA(DisplayName = "Lover"),
    Mentor          UMETA(DisplayName = "Mentor"),
    Student         UMETA(DisplayName = "Student")
};

// ═══════════════════════════════════════════════════════════════
// NPC MEMORY STRUCTS
// ═══════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_MemoryType EventType = ENPC_MemoryType::PlayerInteraction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector EventLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EmotionalImpact = 0.0f; // -1.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString InvolvedCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsImportant = false;

    FNPC_MemoryEvent()
    {
        EventDescription = TEXT("Unknown Event");
        InvolvedCharacter = TEXT("Unknown");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    ENPC_RelationshipType RelationType = ENPC_RelationshipType::Stranger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float TrustLevel = 0.0f; // -1.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float AffectionLevel = 0.0f; // -1.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float RespectLevel = 0.0f; // -1.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    TArray<FNPC_MemoryEvent> SharedMemories;

    FNPC_Relationship()
    {
        CharacterName = TEXT("Unknown");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_EmotionalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    ENPC_EmotionType CurrentEmotion = ENPC_EmotionType::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float EmotionIntensity = 0.5f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float EmotionDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    FString EmotionTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    bool bIsTemporary = true;

    FNPC_EmotionalState()
    {
        EmotionTrigger = TEXT("Unknown");
    }
};

// ═══════════════════════════════════════════════════════════════
// NPC MEMORY COMPONENT CLASS
// ═══════════════════════════════════════════════════════════════

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_MemoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_MemoryComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY MANAGEMENT
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    TArray<FNPC_MemoryEvent> MemoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    TArray<FNPC_Relationship> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    FNPC_EmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    int32 MaxMemoryEvents = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    float MemoryDecayRate = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    float EmotionDecayRate = 0.1f;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemoryEvent(const FNPC_MemoryEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void UpdateRelationship(const FString& CharacterName, float TrustChange, float AffectionChange, float RespectChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void SetEmotionalState(ENPC_EmotionType NewEmotion, float Intensity, float Duration, const FString& Trigger);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FNPC_Relationship GetRelationship(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_MemoryEvent> GetMemoriesOfType(ENPC_MemoryType MemoryType);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_MemoryEvent> GetMemoriesWithCharacter(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    float GetTrustLevel(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    bool HasMetCharacter(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ForgetOldMemories();

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void UpdateEmotionalDecay(float DeltaTime);

    // ═══════════════════════════════════════════════════════════════
    // BEHAVIORAL INFLUENCE FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    float GetPlayerTrustModifier();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldApproachPlayer();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldAvoidPlayer();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FString GetGreetingBasedOnRelationship(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    float GetEmotionalInfluenceOnAction(const FString& ActionType);

private:
    // Internal memory management
    void CleanupMemories();
    void SortMemoriesByImportance();
    FNPC_Relationship* FindOrCreateRelationship(const FString& CharacterName);
};