#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Greeting    UMETA(DisplayName = "Greeting"),
    QuestOffer  UMETA(DisplayName = "QuestOffer"),
    QuestActive UMETA(DisplayName = "QuestActive"),
    QuestDone   UMETA(DisplayName = "QuestDone"),
    Farewell    UMETA(DisplayName = "Farewell")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState TriggerState;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , TriggerState(ENarr_DialogueState::Idle)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_NPCDialogueData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState CurrentState;

    FNarr_NPCDialogueData()
        : NPCName(TEXT("Unknown"))
        , CurrentState(ENarr_DialogueState::Idle)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_NPCDialogueData DialogueData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsInteractable;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetDialogueState(ENarr_DialogueState NewState);

protected:
    virtual void BeginPlay() override;

private:
    int32 CurrentLineIndex;
};
