#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "../SharedTypes.h"
#include "DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundBase> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_EmotionType EmotionType;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::GetEmpty();
        Duration = 3.0f;
        EmotionType = ENarr_EmotionType::Neutral;
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
    ENarr_TriggerType TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("");
        TriggerCondition = ENarr_TriggerType::LocationBased;
        bIsRepeatable = false;
        Priority = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerContextualDialogue(ENarr_TriggerType TriggerType, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueVolume(float Volume);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, FNarr_DialogueSequence> RegisteredSequences;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FNarr_DialogueSequence CurrentSequence;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsPlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float DialogueVolume;

    UPROPERTY()
    class UAudioComponent* DialogueAudioComponent;

    UPROPERTY()
    FTimerHandle DialogueTimerHandle;

private:
    void PlayNextLine();
    void OnLineFinished();
    void InitializeDefaultSequences();
    FNarr_DialogueSequence CreateSurvivalSequence();
    FNarr_DialogueSequence CreateDangerSequence();
    FNarr_DialogueSequence CreateDiscoverySequence();
};