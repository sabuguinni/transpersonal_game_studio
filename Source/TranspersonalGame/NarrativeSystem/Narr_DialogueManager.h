#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

class UNarr_DialogueComponent;

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
    TSoftObjectPtr<USoundCue> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> NextDialogueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        VoiceClip = nullptr;
        Duration = 3.0f;
        bIsPlayerChoice = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("");
        bIsRepeatable = false;
        Priority = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, DialogueID, const FString&, SpeakerName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, const FString&, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDialogueChoice, const FString&, DialogueID, const TArray<FString>&, Choices, int32, SelectedIndex);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID, AActor* Speaker = nullptr, AActor* Listener = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectDialogueChoice(int32 ChoiceIndex);

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasDialogue(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentDialogueEntry() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueChoice OnDialogueChoice;

protected:
    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentDialogueIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FNarr_DialogueSequence CurrentSequence;

    // Dialogue storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    TMap<FString, FNarr_DialogueSequence> DialogueSequences;

    // Audio management
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* VoiceAudioComponent;

    // Speaker references
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    TWeakObjectPtr<AActor> CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    TWeakObjectPtr<AActor> CurrentListener;

private:
    void PlayVoiceClip(const FNarr_DialogueEntry& Entry);
    void StopVoiceClip();
    bool ValidateDialogueEntry(const FNarr_DialogueEntry& Entry) const;
    void ProcessDialogueChoice(const FNarr_DialogueEntry& Entry);
};