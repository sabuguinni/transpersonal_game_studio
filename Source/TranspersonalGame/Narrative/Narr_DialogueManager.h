#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
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
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_EmotionalTone EmotionalTone;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
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
    int32 Priority;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("");
        bIsRepeatable = false;
        Priority = 1;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DialogueDisplayTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bAutoAdvanceDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    int32 MaxConcurrentDialogues;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    FString CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    FString CurrentDialogueText;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    FString CurrentSequenceID;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueSequence(const FNarr_DialogueSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasDialogueSequence(const FString& SequenceID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerContextualDialogue(ENarr_SurvivalEvent EventType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueDisplayTime(float NewDisplayTime);

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FString GetCurrentSpeaker() const { return CurrentSpeaker; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FString GetCurrentDialogueText() const { return CurrentDialogueText; }

private:
    void ProcessCurrentDialogueLine();
    void OnDialogueLineComplete();
    FNarr_DialogueSequence* FindDialogueSequence(const FString& SequenceID);
    void InitializeDefaultDialogues();

    float CurrentLineTimer;
    int32 ActiveDialogueCount;
};