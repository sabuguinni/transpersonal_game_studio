#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "TranspersonalGame.h"
#include "Narr_DialogueSystem.generated.h"

// Dialogue node types for primitive survival communication
UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Warning,        // Danger alerts, predator warnings
    Instruction,    // Survival guidance, hunting tips
    Observation,    // Environmental awareness, tracking info
    Challenge,      // Combat taunts, territorial disputes
    Greeting,       // Basic tribal acknowledgment
    Farewell        // Departure, death rites
};

// Survival context for dialogue triggers
UENUM(BlueprintType)
enum class ENarr_SurvivalContext : uint8
{
    Hunting,        // Tracking, stalking, killing prey
    Danger,         // Predator proximity, environmental hazards
    Crafting,       // Tool making, shelter building
    Exploration,    // Territory mapping, resource discovery
    Combat,         // Fighting dinosaurs, defending territory
    Rest            // Safe zones, tribal gatherings
};

// Primitive emotional states for realistic survival dialogue
UENUM(BlueprintType)
enum class ENarr_EmotionalState : uint8
{
    Calm,           // Normal, alert state
    Fearful,        // Predator nearby, danger sensed
    Aggressive,     // Ready to fight, territorial
    Exhausted,      // Low stamina, need rest
    Hungry,         // Need food, hunting urgency
    Triumphant      // Successful hunt, victory over predator
};

// Single dialogue entry with survival-focused content
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    // Dialogue text in primitive survival language
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    // Audio file URL for voice playback
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL;

    // Type of survival communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    // Survival context that triggers this dialogue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    ENarr_SurvivalContext TriggerContext;

    // Emotional state required for this dialogue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    ENarr_EmotionalState RequiredEmotion;

    // Distance range for dialogue trigger (meters)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float TriggerDistance;

    // Cooldown before this dialogue can trigger again (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "5.0", ClampMax = "300.0"))
    float CooldownTime;

    // Priority level (higher = more likely to play)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Priority", meta = (ClampMin = "1", ClampMax = "10"))
    int32 Priority;

    FNarr_DialogueEntry()
    {
        DialogueText = TEXT("Danger approaches.");
        AudioURL = TEXT("");
        DialogueType = ENarr_DialogueType::Warning;
        TriggerContext = ENarr_SurvivalContext::Danger;
        RequiredEmotion = ENarr_EmotionalState::Fearful;
        TriggerDistance = 15.0f;
        CooldownTime = 30.0f;
        Priority = 5;
    }
};

// Character archetype for survival-based NPCs
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterArchetype
{
    GENERATED_BODY()

    // Character name/identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;

    // Role in tribal survival hierarchy
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role")
    FString SurvivalRole;

    // Expertise areas (hunting, crafting, tracking, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    TArray<FString> Expertise;

    // Base emotional tendency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    ENarr_EmotionalState DefaultEmotion;

    // Voice characteristics for audio generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString VoiceProfile;

    // Dialogue entries specific to this character
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    FNarr_CharacterArchetype()
    {
        CharacterName = TEXT("Tribal Hunter");
        SurvivalRole = TEXT("Scout");
        Expertise = {TEXT("Tracking"), TEXT("Stealth")};
        DefaultEmotion = ENarr_EmotionalState::Calm;
        VoiceProfile = TEXT("Gruff Male");
        DialogueEntries = TArray<FNarr_DialogueEntry>();
    }
};

// Dialogue trigger conditions based on survival gameplay
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTrigger
{
    GENERATED_BODY()

    // Player proximity required (meters)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float ProximityDistance;

    // Required survival context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    ENarr_SurvivalContext RequiredContext;

    // Player health threshold (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HealthThreshold;

    // Player hunger level (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HungerThreshold;

    // Time of day (0.0 = dawn, 0.5 = noon, 1.0 = midnight)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TimeOfDay;

    // Nearby dinosaur threat level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger", meta = (ClampMin = "0", ClampMax = "5"))
    int32 ThreatLevel;

    // Minimum time between triggers (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (ClampMin = "10.0", ClampMax = "600.0"))
    float MinTimeBetweenTriggers;

    FNarr_DialogueTrigger()
    {
        ProximityDistance = 20.0f;
        RequiredContext = ENarr_SurvivalContext::Exploration;
        HealthThreshold = 0.5f;
        HungerThreshold = 0.3f;
        TimeOfDay = 0.5f;
        ThreatLevel = 1;
        MinTimeBetweenTriggers = 60.0f;
    }
};

// Main dialogue system component for NPCs
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Character archetype defining this NPC's dialogue behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FNarr_CharacterArchetype CharacterData;

    // Trigger conditions for dialogue activation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    FNarr_DialogueTrigger TriggerConditions;

    // Current emotional state affecting dialogue selection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    ENarr_EmotionalState CurrentEmotion;

    // Audio component for voice playback
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AudioComponent;

    // Last dialogue trigger time
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastTriggerTime;

    // Currently playing dialogue entry
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FNarr_DialogueEntry CurrentDialogue;

    // Check if dialogue should trigger based on survival context
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool ShouldTriggerDialogue(AActor* PlayerActor, ENarr_SurvivalContext Context);

    // Select appropriate dialogue based on current conditions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry SelectDialogue(ENarr_SurvivalContext Context, ENarr_EmotionalState Emotion);

    // Play selected dialogue with audio
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogue(const FNarr_DialogueEntry& DialogueEntry);

    // Update emotional state based on survival conditions
    UFUNCTION(BlueprintCallable, Category = "Emotion")
    void UpdateEmotionalState(float PlayerHealth, float ThreatLevel, bool IsNearPredator);

    // Add new dialogue entry to character
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueEntry(const FNarr_DialogueEntry& NewEntry);

    // Get all dialogue entries of specific type
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> GetDialoguesByType(ENarr_DialogueType Type);

    // Check if character has dialogue for specific context
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasDialogueForContext(ENarr_SurvivalContext Context);

private:
    // Calculate distance to player
    float GetDistanceToPlayer() const;

    // Get current time of day from world
    float GetCurrentTimeOfDay() const;

    // Detect nearby threats (dinosaurs, environmental hazards)
    int32 GetNearbyThreatLevel() const;

    // Get player's current survival context
    ENarr_SurvivalContext GetPlayerSurvivalContext() const;
};