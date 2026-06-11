#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
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
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT(""));
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
    ENarr_DialogueTrigger TriggerCondition;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("DefaultSequence");
        TriggerCondition = ENarr_DialogueTrigger::Manual;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerContextualDialogue(ENarr_DialogueTrigger TriggerType, const FVector& Location);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FNarr_DialogueSequence* CurrentSequence;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bDialogueActive;

    UFUNCTION()
    void AdvanceDialogue();

    UFUNCTION()
    void ProcessDialogueLine(const FNarr_DialogueLine& Line);
};