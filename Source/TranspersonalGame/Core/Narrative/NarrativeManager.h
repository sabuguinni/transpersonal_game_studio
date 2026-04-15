#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    float EmotionalIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FString> TriggeredEvents;

    FNarr_DialogueEntry()
    {
        CharacterName = TEXT("");
        DialogueText = TEXT("");
        AudioAssetPath = TEXT("");
        EmotionalIntensity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString BeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString BeatDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> DialogueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    int32 Priority;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        BeatTitle = TEXT("");
        BeatDescription = TEXT("");
        bIsCompleted = false;
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    FString CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    TArray<FString> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    FString VoiceProfile;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    TArray<FString> KnownInformation;

    FNarr_CharacterProfile()
    {
        CharacterID = TEXT("");
        DisplayName = TEXT("");
        BackgroundStory = TEXT("");
        VoiceProfile = TEXT("Default");
        TrustLevel = 0.5f;
    }
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle,
    WaitingForInput,
    Playing,
    Completed,
    Interrupted
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    // Story Progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_StoryBeat> GetAvailableStoryBeats() const;

    // Character Management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterCharacter(const FNarr_CharacterProfile& Character);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_CharacterProfile GetCharacterProfile(const FString& CharacterID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateCharacterTrust(const FString& CharacterID, float TrustDelta);

    // Condition System
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool EvaluateCondition(const FString& Condition) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetGameFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetGameFlag(const FString& FlagName) const;

    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueAudio(const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopDialogueAudio();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueState CurrentDialogueState;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, FNarr_StoryBeat> StoryBeats;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, FNarr_CharacterProfile> Characters;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, bool> GameFlags;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    TSoftObjectPtr<UDataTable> DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    float DefaultDialogueSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    bool bAutoAdvanceDialogue;

private:
    void LoadDialogueData();
    void InitializeStoryBeats();
    void InitializeCharacters();
    bool CheckPrerequisites(const TArray<FString>& Prerequisites) const;
    void ExecuteEvents(const TArray<FString>& Events);
};