#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

class UDialogueComponent;
class UQuestComponent;

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Locked          UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class ENarr_StoryBeat : uint8
{
    Awakening       UMETA(DisplayName = "The Awakening"),
    FirstVision     UMETA(DisplayName = "First Vision"),
    ShamanMeeting   UMETA(DisplayName = "Meeting the Shaman"),
    BeastEncounter  UMETA(DisplayName = "First Beast Encounter"),
    PlantCeremony   UMETA(DisplayName = "Plant Medicine Ceremony"),
    SpiritWalk      UMETA(DisplayName = "Spirit Walk"),
    Transcendence   UMETA(DisplayName = "Final Transcendence")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_StoryBeat RequiredStoryBeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float EmotionalWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsTranscendentalMoment;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        AudioAssetPath = TEXT("");
        RequiredStoryBeat = ENarr_StoryBeat::Awakening;
        EmotionalWeight = 1.0f;
        bIsTranscendentalMoment = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryBeat CurrentBeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<ENarr_StoryBeat> CompletedBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TMap<FString, bool> CharacterRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedLoreEntries;

    FNarr_StoryState()
    {
        CurrentBeat = ENarr_StoryBeat::Awakening;
        ConsciousnessLevel = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, SpeakerName, const FText&, DialogueText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryBeatCompleted, ENarr_StoryBeat, CompletedBeat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConsciousnessLevelChanged, float, NewLevel);

/**
 * Core narrative system that manages story progression, dialogue, and consciousness evolution
 * Integrates with the transpersonal themes of the game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryBeat(ENarr_StoryBeat NewBeat);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(ENarr_StoryBeat Beat) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_StoryBeat GetCurrentStoryBeat() const;

    // Consciousness system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ModifyConsciousnessLevel(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    float GetConsciousnessLevel() const;

    // Dialogue system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAvailableDialogueOptions() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool SelectDialogueOption(int32 OptionIndex);

    // Character relationships
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetCharacterRelationship(const FString& CharacterName, bool bIsFriendly);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetCharacterRelationship(const FString& CharacterName) const;

    // Lore system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UnlockLoreEntry(const FString& LoreID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsLoreEntryUnlocked(const FString& LoreID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetUnlockedLoreEntries() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnStoryBeatCompleted OnStoryBeatCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnConsciousnessLevelChanged OnConsciousnessLevelChanged;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_StoryState CurrentStoryState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_DialogueEntry> ActiveDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString CurrentDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TWeakObjectPtr<AActor> CurrentSpeaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TWeakObjectPtr<AActor> CurrentListener;

private:
    void InitializeStoryState();
    void LoadDialogueData();
    FNarr_DialogueEntry* GetDialogueEntry(const FString& DialogueID);
    bool ValidateStoryBeatProgression(ENarr_StoryBeat NewBeat) const;
};