#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ChoiceOptions;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
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
    bool bRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 RequiredQuestStage;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("");
        bRepeatable = false;
        RequiredQuestStage = 0;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DefaultDialogueDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bAutoAdvanceDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    FString CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    FString CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    bool bDialogueActive;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void StartDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AddDialogueSequence(const FNarr_DialogueSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FString GetCurrentSpeaker() const { return CurrentSpeaker; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FString GetCurrentDialogue() const { return CurrentDialogue; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void RegisterTribalDialogues();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void RegisterSurvivalDialogues();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void RegisterHuntDialogues();

private:
    int32 CurrentDialogueIndex;
    FString ActiveSequenceID;
    FTimerHandle DialogueTimerHandle;

    void ProcessCurrentDialogue();
    void AutoAdvanceToNext();
};

#include "Narr_DialogueSystem.generated.h"