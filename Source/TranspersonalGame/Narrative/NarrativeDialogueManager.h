#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "NarrativeDialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Narration,
    CharacterDialogue,
    SystemMessage,
    QuestBriefing,
    EnvironmentalStory
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    PlayerEnterZone,
    QuestProgress,
    CombatStart,
    Discovery,
    TimeOfDay,
    WeatherChange
};

USTRUCT(BlueprintType)
struct FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PrerequisiteQuests;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        DialogueText = FText::GetEmpty();
        DialogueType = ENarr_DialogueType::Narration;
        TriggerType = ENarr_DialogueTrigger::PlayerEnterZone;
        CharacterName = TEXT("");
        AudioFilePath = TEXT("");
        Duration = 0.0f;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 1000.0f;
        bIsRepeatable = false;
    }
};

USTRUCT(BlueprintType)
struct FNarr_StoryProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletedDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> DiscoveredLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedNarratives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 MainStoryProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TMap<FString, int32> CharacterRelationships;

    FNarr_StoryProgress()
    {
        MainStoryProgress = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeDialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core dialogue system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(const FString& DialogueID, AActor* TriggeringActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueEntry(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueAvailable(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueAudio(const FString& AudioFilePath, float Volume = 1.0f);

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Story")
    void MarkDialogueCompleted(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void DiscoverLocation(const FString& LocationName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void UnlockNarrative(const FString& NarrativeID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceMainStory(int32 ProgressAmount = 1);

    // Character relationships
    UFUNCTION(BlueprintCallable, Category = "Character")
    void ModifyCharacterRelationship(const FString& CharacterName, int32 RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Character")
    int32 GetCharacterRelationship(const FString& CharacterName) const;

    // Environmental storytelling
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void TriggerEnvironmentalStory(const FVector& Location, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void RegisterStoryElement(const FString& ElementID, const FVector& Location, const FString& StoryText);

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void TriggerQuestDialogue(const FString& QuestID, const FString& DialoguePhase);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FString> GetAvailableQuestDialogues(const FString& QuestID) const;

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetDialogueVolume(float Volume);

    // Save/Load system
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveStoryProgress();

    UFUNCTION(BlueprintCallable, Category = "Save")
    void LoadStoryProgress();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Story")
    const FNarr_StoryProgress& GetStoryProgress() const { return StoryProgress; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    const TArray<FNarr_DialogueEntry>& GetAllDialogues() const { return DialogueEntries; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FNarr_StoryProgress StoryProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TMap<FString, FVector> StoryElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    UAudioComponent* DialogueAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DefaultDialogueVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString CurrentPlayingDialogue;

private:
    void InitializeDefaultDialogues();
    void LoadDialogueDatabase();
    bool CheckDialoguePrerequisites(const FNarr_DialogueEntry& DialogueEntry) const;
    void BroadcastDialogueEvent(const FString& DialogueID, const FString& EventType);
};