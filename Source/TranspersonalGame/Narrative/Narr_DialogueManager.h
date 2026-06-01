#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceClip;

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
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bHasBeenPlayed;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("Default");
        bIsRepeatable = true;
        bHasBeenPlayed = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueSequence(const FString& SequenceID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayNarration(const FText& NarrationText, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlaySurvivalWarning(ESurvivalThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDinosaurEncounterDialogue(EDinosaurSpecies Species, bool bIsHostile);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, FNarr_DialogueSequence> RegisteredSequences;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    UAudioComponent* DialogueAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FNarr_DialogueLine CurrentDialogueLine;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsPlayingDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString CurrentSequenceID;

    UFUNCTION()
    void OnDialogueLineFinished();

    UFUNCTION()
    void PlayNextDialogueLine();

    void CreateSurvivalWarningSequences();
    void CreateDinosaurEncounterSequences();

private:
    FTimerHandle DialogueTimerHandle;
};