#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle,
    Speaking,
    WaitingForResponse,
    Completed
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        AudioPath = TEXT("");
        Duration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("Default");
        bIsQuestRelated = false;
        QuestID = TEXT("");
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

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    ENarr_DialogueState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    FString CurrentSequenceID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DialogueRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bAutoAdvance;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool StartDialogue(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool AddDialogueSequence(const FNarr_DialogueSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool IsPlayerInRange() const;

    UFUNCTION(BlueprintPure, Category = "Dialogue System")
    bool IsDialogueActive() const { return CurrentState != ENarr_DialogueState::Idle; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue System")
    void OnDialogueStarted(const FString& SequenceID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue System")
    void OnDialogueLineChanged(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue System")
    void OnDialogueCompleted(const FString& SequenceID);

private:
    float CurrentLineTimer;
    AActor* PlayerActor;

    void UpdatePlayerReference();
    void ProcessCurrentLine(float DeltaTime);
};