#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "../SharedTypes.h"
#include "NarrativeDialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundBase> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        VoiceAudio = nullptr;
        Duration = 3.0f;
        DialogueType = ENarr_DialogueType::Narration;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsActive;

    FNarr_QuestDialogue()
    {
        QuestID = TEXT("");
        bIsActive = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeDialogueManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayQuestDialogue(const FString& QuestID, int32 EntryIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterQuestDialogue(const FString& QuestID, const TArray<FNarr_DialogueEntry>& DialogueEntries);

    // Environmental narration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerEnvironmentalNarration(ENarr_BiomeType BiomeType, ENarr_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerSurvivalWarning(ENarr_SurvivalWarningType WarningType);

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetNarrationVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetNarrationVolume() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> DialogueAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TMap<FString, FNarr_QuestDialogue> QuestDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float NarrationVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsCurrentlyPlaying;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FString CurrentDialogueID;

private:
    void InitializeDialogueDatabase();
    void LoadEnvironmentalNarration();
    void OnDialogueFinished();

    UFUNCTION()
    void HandleAudioFinished();

    FTimerHandle DialogueTimerHandle;
};