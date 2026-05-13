#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
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
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class USoundCue* VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresPlayerResponse;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        VoiceClip = nullptr;
        DisplayDuration = 3.0f;
        bRequiresPlayerResponse = false;
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
    int32 RequiredStoryBeat;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("Default");
        bIsRepeatable = true;
        RequiredStoryBeat = 0;
    }
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Playing UMETA(DisplayName = "Playing"),
    WaitingForResponse UMETA(DisplayName = "Waiting For Response"),
    Completed UMETA(DisplayName = "Completed")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core dialogue management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    // Dialogue sequence management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasDialogueSequence(const FString& SequenceID) const;

    // Character-specific dialogue
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerCharacterDialogue(const FString& CharacterName, const FString& Context);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetCharacterVoiceProfile(const FString& CharacterName, class USoundCue* VoiceProfile);

    // Story integration
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void UpdateStoryProgress(int32 NewStoryBeat);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableDialogueSequences() const;

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueAudio(class USoundCue* AudioClip);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogueAudio();

protected:
    // Dialogue state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    ENarr_DialogueState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentSequenceID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentLineIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    float CurrentLineTimer;

    // Dialogue data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    TMap<FString, FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    TMap<FString, class USoundCue*> CharacterVoiceProfiles;

    // Story integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Integration")
    int32 CurrentStoryBeat;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* DialogueAudioComponent;

    // Default dialogue sequences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Dialogue")
    TArray<FNarr_DialogueSequence> DefaultSequences;

private:
    void InitializeDefaultDialogues();
    void ProcessCurrentDialogueLine();
    bool CanPlaySequence(const FNarr_DialogueSequence& Sequence) const;
    void OnDialogueLineCompleted();
};