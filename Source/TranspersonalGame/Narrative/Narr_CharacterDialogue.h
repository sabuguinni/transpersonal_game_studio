#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Narr_CharacterDialogue.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    ESurvivalContext RequiredContext;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        AudioFilePath = TEXT("");
        Duration = 3.0f;
        RequiredContext = ESurvivalContext::Normal;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    int32 NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    bool bRequiresItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString RequiredItemName;

    FNarr_DialogueChoice()
    {
        ChoiceText = FText::FromString(TEXT("Continue"));
        NextDialogueID = -1;
        bRequiresItem = false;
        RequiredItemName = TEXT("");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_CharacterDialogue : public UDataAsset
{
    GENERATED_BODY()

public:
    UNarr_CharacterDialogue();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    ECharacterArchetype CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> DialogueChoices;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentDialogueIndex;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> GetAvailableChoices() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool AdvanceDialogue(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ResetDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasMoreDialogue() const;

private:
    UPROPERTY()
    bool bDialogueCompleted;
};