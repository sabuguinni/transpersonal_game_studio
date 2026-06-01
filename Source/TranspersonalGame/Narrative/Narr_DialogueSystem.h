#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    USoundCue* VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_EmotionalTone EmotionalTone;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        VoiceClip = nullptr;
        Duration = 3.0f;
        EmotionalTone = ENarr_EmotionalTone::Neutral;
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
    TArray<FString> RequiredFlags;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("Default");
        bIsRepeatable = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const FString&, SequenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueLineChanged, const FNarr_DialogueLine&, CurrentLine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine() const { return CurrentDialogueLine; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool GetDialogueFlag(const FString& FlagName) const;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueLineChanged OnDialogueLineChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEnded OnDialogueEnded;

protected:
    UPROPERTY()
    TMap<FString, FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY()
    TMap<FString, bool> DialogueFlags;

    UPROPERTY()
    UAudioComponent* DialogueAudioComponent;

    UPROPERTY()
    bool bIsDialogueActive;

    UPROPERTY()
    FString CurrentSequenceID;

    UPROPERTY()
    int32 CurrentLineIndex;

    UPROPERTY()
    FNarr_DialogueLine CurrentDialogueLine;

    UPROPERTY()
    FTimerHandle DialogueTimerHandle;

    void PlayCurrentDialogueLine();
    void OnDialogueLineFinished();
    bool CheckSequenceRequirements(const FNarr_DialogueSequence& Sequence) const;
    void LoadDefaultDialogueSequences();
};