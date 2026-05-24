#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryProgressionManager.generated.h"

// Story progression stages for survival narrative
UENUM(BlueprintType)
enum class ENarr_StoryStage : uint8
{
    Arrival         UMETA(DisplayName = "Arrival"),
    FirstSurvival   UMETA(DisplayName = "First Survival"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Adaptation      UMETA(DisplayName = "Adaptation"),
    Mastery         UMETA(DisplayName = "Mastery"),
    Leadership      UMETA(DisplayName = "Leadership")
};

// Narrative events that can trigger story progression
UENUM(BlueprintType)
enum class ENarr_NarrativeEvent : uint8
{
    PlayerSpawned       UMETA(DisplayName = "Player Spawned"),
    FirstDinosaurSeen   UMETA(DisplayName = "First Dinosaur Seen"),
    FirstCraft          UMETA(DisplayName = "First Craft"),
    FirstShelter        UMETA(DisplayName = "First Shelter"),
    BiomeDiscovered     UMETA(DisplayName = "Biome Discovered"),
    NPCMet              UMETA(DisplayName = "NPC Met"),
    QuestCompleted      UMETA(DisplayName = "Quest Completed"),
    DinosaurKilled      UMETA(DisplayName = "Dinosaur Killed"),
    PlayerDied          UMETA(DisplayName = "Player Died")
};

// Story data for each narrative event
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_NarrativeEvent EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float TriggerDelay;

    FNarr_StoryEvent()
    {
        EventType = ENarr_NarrativeEvent::PlayerSpawned;
        EventDescription = "Default Event";
        NarrativeText = "Default narrative text";
        AudioFilePath = "";
        bTriggered = false;
        TriggerDelay = 0.0f;
    }
};

// Biome-specific narrative content
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_BiomeNarrative
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Narrative")
    EEng_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Narrative")
    FString BiomeIntroText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Narrative")
    FString BiomeDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Narrative")
    TArray<FString> EnvironmentalNarrative;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Narrative")
    TArray<FString> DangerWarnings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Narrative")
    bool bDiscovered;

    FNarr_BiomeNarrative()
    {
        BiomeType = EEng_BiomeType::Forest;
        BiomeIntroText = "You enter an unknown biome";
        BiomeDescription = "A mysterious environment";
        bDiscovered = false;
    }
};

/**
 * Story Progression Manager - Controls narrative flow and story events
 * Manages the player's journey from arrival to mastery of the prehistoric world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_StoryProgressionManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryProgressionManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story progression methods
    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void TriggerNarrativeEvent(ENarr_NarrativeEvent EventType, const FString& Context = "");

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void AdvanceStoryStage();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void DiscoverBiome(EEng_BiomeType BiomeType, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    FString GetCurrentNarrativeText() const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    ENarr_StoryStage GetCurrentStoryStage() const { return CurrentStoryStage; }

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    bool HasEventTriggered(ENarr_NarrativeEvent EventType) const;

    // Biome narrative methods
    UFUNCTION(BlueprintCallable, Category = "Biome Narrative")
    FString GetBiomeIntroText(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Narrative")
    TArray<FString> GetBiomeDangerWarnings(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Narrative")
    bool IsBiomeDiscovered(EEng_BiomeType BiomeType) const;

    // Audio narrative methods
    UFUNCTION(BlueprintCallable, Category = "Audio Narrative")
    void PlayNarrativeAudio(const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Audio Narrative")
    void QueueNarrativeText(const FString& NarrativeText, float DelaySeconds = 0.0f);

protected:
    // Current story state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story State")
    ENarr_StoryStage CurrentStoryStage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story State")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story State")
    TArray<FNarr_BiomeNarrative> BiomeNarratives;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story State")
    FString CurrentNarrativeText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story State")
    float LastEventTime;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Config")
    float MinTimeBetweenEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Config")
    bool bAutoAdvanceStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Config")
    bool bPlayAudioNarration;

private:
    // Internal methods
    void InitializeStoryEvents();
    void InitializeBiomeNarratives();
    void ProcessQueuedNarrative();
    FNarr_BiomeNarrative* GetBiomeNarrativeData(EEng_BiomeType BiomeType);
    
    // Timer for queued narrative
    FTimerHandle NarrativeQueueTimer;
    TArray<FString> QueuedNarrativeTexts;
};