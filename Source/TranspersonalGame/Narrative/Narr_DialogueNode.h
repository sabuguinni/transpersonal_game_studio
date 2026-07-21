#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Narr_DialogueNode.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueNodeType : uint8
{
    PlayerChoice,
    NPCResponse,
    QuestGiver,
    Trader,
    Warning
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FText ChoiceText;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString NextNodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bRequiresItem;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString RequiredItemID;

    FNarr_DialogueChoice()
    {
        ChoiceText = FText::GetEmpty();
        NextNodeID = TEXT("");
        bRequiresItem = false;
        RequiredItemID = TEXT("");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueNode : public UObject
{
    GENERATED_BODY()

public:
    UNarr_DialogueNode();

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString NodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    ENarr_DialogueNodeType NodeType;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FText SpeakerName;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> PlayerChoices;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString AudioClipPath;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bEndsConversation;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString TriggerEventID;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanSelectChoice(int32 ChoiceIndex, const TArray<FString>& PlayerInventory) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetNextNodeID(int32 ChoiceIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasValidChoices(const TArray<FString>& PlayerInventory) const;
};