#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Sound/DialogueWave.h"
#include "Sound/DialogueVoice.h"
#include "Engine/DataTable.h"
#include "DialogueComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDialogueLineSpoken, const FString&, SpeakerName, const FString&, DialogueText, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueSequenceComplete, FGameplayTag, DialogueTag);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (MultiLine = true))
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDialogueWave* DialogueWave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDialogueVoice* SpeakerVoice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bWaitForInput = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FGameplayTag> RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FGameplayTag> TriggeredEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FGameplayTag EmotionTag;

    FDialogueLine()
    {
        DisplayDuration = 3.0f;
        bWaitForInput = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FGameplayTag SequenceTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FDialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FGameplayTag> PrerequisiteTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority = 0;

    FDialogueSequence()
    {
        bIsRepeatable = true;
        Priority = 0;
    }
};

UENUM(BlueprintType)
enum class EDialogueState : uint8
{
    Idle,
    Playing,
    WaitingForInput,
    Paused,
    Complete
};

/**
 * Dialogue Component - Handles character dialogue and conversation sequences
 * Attached to NPCs and interactive objects that can speak
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue Sequence Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogueSequence(FGameplayTag SequenceTag, class AActor* Listener = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogueSequence();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PauseDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ResumeDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue();

    // Dialogue Line Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueLine(const FDialogueLine& DialogueLine, class AActor* Listener = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueSequence(const FDialogueSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasDialogueSequence(FGameplayTag SequenceTag) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanPlaySequence(FGameplayTag SequenceTag) const;

    // State Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    EDialogueState GetDialogueState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return CurrentState != EDialogueState::Idle; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FGameplayTag GetCurrentSequenceTag() const { return CurrentSequenceTag; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    int32 GetCurrentLineIndex() const { return CurrentLineIndex; }

    // Character Configuration
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetCharacterName(const FString& NewName) { CharacterName = NewName; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetCharacterName() const { return CharacterName; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDefaultVoice(class UDialogueVoice* NewVoice) { DefaultVoice = NewVoice; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueLineSpoken OnDialogueLineSpoken;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueSequenceComplete OnDialogueSequenceComplete;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    class UDialogueVoice* DefaultVoice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TMap<FGameplayTag, FDialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DefaultLineDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoAdvanceDialogue = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float AutoAdvanceDelay = 0.5f;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    EDialogueState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FGameplayTag CurrentSequenceTag;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    class AActor* CurrentListener;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    class UAudioComponent* CurrentAudioComponent;

    // Timers
    FTimerHandle LineTimerHandle;
    FTimerHandle AutoAdvanceTimerHandle;

private:
    void LoadDialogueFromDataTable();
    void ProcessCurrentLine();
    void CompleteCurrentLine();
    void AdvanceToNextLine();
    void CompleteSequence();
    void TriggerLineEvents(const FDialogueLine& Line);
    
    UFUNCTION()
    void OnAudioComponentFinished();
};