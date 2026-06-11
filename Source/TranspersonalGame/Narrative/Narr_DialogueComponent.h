#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresPlayerResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FText> PlayerResponses;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
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
    int32 Priority;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("DefaultSequence");
        bIsRepeatable = true;
        Priority = 1;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FString CurrentSequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FString> CompletedSequences;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanInteract(AActor* PlayerActor) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueSequence(const FNarr_DialogueSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasCompletedSequence(const FString& SequenceID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableSequences() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetPlayerResponse(int32 ResponseIndex);

private:
    FNarr_DialogueSequence* FindSequence(const FString& SequenceID);
    void MarkSequenceCompleted(const FString& SequenceID);
};