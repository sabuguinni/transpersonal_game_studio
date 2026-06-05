#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "../SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

// Forward declarations
class UNarr_DialogueComponent;
class ANarr_DialogueTrigger;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundBase> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FText> PlayerResponses;

    FNarr_DialogueLine()
    {
        DisplayDuration = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ESurvivalContext TriggerContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    FNarr_DialogueSequence()
    {
        TriggerContext = ESurvivalContext::Exploration;
        bIsRepeatable = false;
        Priority = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(FName SequenceID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayContextualDialogue(ESurvivalContext Context, const FVector& Location);

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayVoiceLine(USoundBase* VoiceAudio, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetDialogueVolume(float Volume);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TMap<FName, FNarr_DialogueSequence> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FNarr_DialogueSequence> ContextualDialogues;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FNarr_DialogueSequence* CurrentSequence;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float DialogueVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* VoiceAudioComponent;

private:
    void InitializeDefaultDialogues();
    void AdvanceDialogue();
    FNarr_DialogueSequence* FindBestContextualDialogue(ESurvivalContext Context);

    FTimerHandle DialogueTimerHandle;
    bool bDialogueActive;
};