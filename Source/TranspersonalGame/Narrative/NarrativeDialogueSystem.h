#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "NarrativeDialogueSystem.generated.h"

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
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsNarration;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        DisplayDuration = 3.0f;
        bIsNarration = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_SurvivalTip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FText TipText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    ENarr_DinosaurSpecies TriggerSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    TSoftObjectPtr<USoundCue> WarningAudio;

    FNarr_SurvivalTip()
    {
        TipText = FText::GetEmpty();
        TriggerSpecies = ENarr_DinosaurSpecies::TRex;
        TriggerDistance = 1000.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeDialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerSurvivalTip(ENarr_DinosaurSpecies Species, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarration(const FText& NarrationText, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialoguePlaying() const;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Dialogue")
    TArray<FNarr_DialogueLine> IntroDialogues;

    UPROPERTY(EditDefaultsOnly, Category = "Survival")
    TArray<FNarr_SurvivalTip> SurvivalTips;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPlayingDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FNarr_DialogueLine CurrentDialogue;

private:
    FTimerHandle DialogueTimerHandle;
    
    void OnDialogueFinished();
    void InitializeDialogueData();
    void InitializeSurvivalTips();
};