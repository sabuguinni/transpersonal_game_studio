#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

class UDialogueComponent;
class UQuestComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
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
    TArray<FString> ResponseOptions;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        AudioAssetPath = TEXT("");
        Duration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        Title = FText::GetEmpty();
        Description = FText::GetEmpty();
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
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
    FString VoiceActorProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float RelationshipLevel;

    FNarr_CharacterProfile()
    {
        CharacterID = TEXT("");
        CharacterName = FText::GetEmpty();
        BackgroundStory = FText::GetEmpty();
        VoiceActorProfile = TEXT("Default");
        RelationshipLevel = 0.0f;
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

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAvailableQuests() const;

    // Dialogue system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& CharacterID, const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SelectDialogueResponse(int32 ResponseIndex);

    // Character relationships
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ModifyRelationship(const FString& CharacterID, float DeltaValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    float GetRelationshipLevel(const FString& CharacterID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_CharacterProfile GetCharacterProfile(const FString& CharacterID) const;

    // Consciousness narrative integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerConsciousnessEvent(const FString& EventID, float IntensityLevel);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateNarrativeBasedOnConsciousness(float ConsciousnessLevel);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_StoryBeat> StoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_CharacterProfile> CharacterProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentDialogueIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsInDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentCharacterID;

private:
    void LoadStoryData();
    void LoadCharacterData();
    void InitializeDefaultStoryBeats();
    void InitializeDefaultCharacters();
};