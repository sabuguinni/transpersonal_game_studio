#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "NarrativeDialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString TriggerCondition;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Narrator");
        DialogueText = FText::GetEmpty();
        Duration = 5.0f;
        TriggerType = ENarr_DialogueTrigger::Manual;
        TriggerCondition = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    bool bIsCompleted;

    FNarr_QuestDialogue()
    {
        QuestID = TEXT("");
        bIsCompleted = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerEnvironmentalNarration(ENarr_BiomeType BiomeType, ENarr_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterQuestDialogue(const FString& QuestID, const TArray<FNarr_DialogueEntry>& Entries);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    TSoftObjectPtr<UDataTable> DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float DefaultVolume;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float FadeInDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float FadeOutDuration;

private:
    UPROPERTY()
    TMap<FString, FNarr_QuestDialogue> QuestDialogues;

    UPROPERTY()
    TArray<FNarr_DialogueEntry> EnvironmentalNarration;

    UPROPERTY()
    class UAudioComponent* CurrentAudioComponent;

    UPROPERTY()
    FTimerHandle DialogueTimerHandle;

    void OnDialogueFinished();
    void LoadEnvironmentalNarration();
    FNarr_DialogueEntry GetEnvironmentalDialogue(ENarr_BiomeType BiomeType, ENarr_ThreatLevel ThreatLevel);
};