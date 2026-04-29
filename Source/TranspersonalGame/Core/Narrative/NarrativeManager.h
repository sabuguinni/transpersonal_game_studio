#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> SetFlags;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        AudioAssetPath = TEXT("");
        Duration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletionFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("story_beat_01");
        Title = FText::FromString(TEXT("Survival Begins"));
        Description = FText::FromString(TEXT("Learn the basics of survival"));
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> KnownDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float TrustLevel;

    FNarr_CharacterProfile()
    {
        CharacterID = TEXT("npc_01");
        CharacterName = FText::FromString(TEXT("Tribal Elder"));
        BackgroundStory = FText::FromString(TEXT("A wise elder who knows the ancient ways"));
        TrustLevel = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& CharacterID, const FString& DialogueSetID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueEntry GetNextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    // Story Progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetStoryFlag(const FString& FlagName) const;

    // Character Management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterCharacter(const FNarr_CharacterProfile& Character);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_CharacterProfile GetCharacterProfile(const FString& CharacterID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateCharacterTrust(const FString& CharacterID, float DeltaTrust);

    // Lore and World Building
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddLoreEntry(const FString& Category, const FString& Title, const FText& Content);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetLoreCategories() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FText GetLoreEntry(const FString& Category, const FString& Title) const;

protected:
    // Dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentCharacterID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_DialogueEntry> CurrentDialogueSet;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentDialogueIndex;

    // Story state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, bool> StoryFlags;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_StoryBeat> StoryBeats;

    // Character data
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, FNarr_CharacterProfile> Characters;

    // Lore database
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, TMap<FString, FText>> LoreDatabase;

private:
    void InitializeDefaultStory();
    void InitializeDefaultCharacters();
    void InitializeDefaultLore();
    bool CheckDialogueConditions(const FNarr_DialogueEntry& Entry) const;
    void ApplyDialogueEffects(const FNarr_DialogueEntry& Entry);
};