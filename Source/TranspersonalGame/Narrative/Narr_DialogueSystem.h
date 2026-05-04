#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

// Forward declarations
class USoundBase;
class UWidget;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    USoundBase* VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    FNarr_DialogueLine()
    {
        SpeakerName = "Unknown";
        DialogueText = FText::FromString("...");
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
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bHasBeenPlayed;

    FNarr_DialogueSequence()
    {
        SequenceID = "DefaultSequence";
        bIsRepeatable = false;
        bHasBeenPlayed = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

    // Dialogue sequences for this NPC/trigger
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    // Current active dialogue
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    FNarr_DialogueSequence* CurrentSequence;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    bool bIsDialogueActive;

    // Audio component for voice playback
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* VoiceAudioComponent;

public:
    // Main dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool StartDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void NextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    // Sequence management
    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AddDialogueSequence(const FNarr_DialogueSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool HasSequence(const FString& SequenceID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    TArray<FString> GetAvailableSequenceIDs() const;

    // Audio playback
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayVoiceLine(USoundBase* VoiceClip);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopVoicePlayback();

    // Events for Blueprint integration
    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueStarted(const FString& SequenceID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueLineChanged(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueEnded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnPlayerChoiceRequired(const TArray<FNarr_DialogueLine>& Choices);

private:
    // Internal helper functions
    void InitializeAudioComponent();
    FNarr_DialogueSequence* FindSequenceByID(const FString& SequenceID);
    void ProcessCurrentLine();

    // Timer handle for auto-advancing dialogue
    FTimerHandle DialogueTimerHandle;

    void AutoAdvanceDialogue();
};