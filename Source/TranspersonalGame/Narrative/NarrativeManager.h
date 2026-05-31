#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsNarration;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::GetEmpty();
        Duration = 3.0f;
        bIsNarration = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText BeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletionFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("DefaultBeat");
        BeatTitle = FText::GetEmpty();
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
    FText CharacterDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_CharacterRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> KnownStoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsAlive;

    FNarr_CharacterProfile()
    {
        CharacterID = TEXT("Unknown");
        CharacterName = FText::GetEmpty();
        CharacterDescription = FText::GetEmpty();
        Role = ENarr_CharacterRole::Survivor;
        TrustLevel = 0.5f;
        bIsAlive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddCharacter(const FNarr_CharacterProfile& Character);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_CharacterProfile GetCharacter(const FString& CharacterID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateCharacterTrust(const FString& CharacterID, float TrustDelta);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarration(const FString& NarrationKey);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetStoryFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAvailableStoryBeats() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrationTrigger(const FString& TriggerID, const FVector& Location, float Radius);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_StoryBeat> StoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_CharacterProfile> Characters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, bool> StoryFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_DialogueLine> NarrationLibrary;

    UPROPERTY()
    UAudioComponent* DialogueAudioComponent;

    UPROPERTY()
    UAudioComponent* NarrationAudioComponent;

private:
    void LoadStoryData();
    void InitializeDefaultCharacters();
    void InitializeDefaultNarrations();
    bool CheckStoryBeatConditions(const FNarr_StoryBeat& StoryBeat) const;
    void CompleteStoryBeat(const FString& BeatID);
};