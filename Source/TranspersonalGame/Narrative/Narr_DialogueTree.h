#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Narr_DialogueTree.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    None,
    TrustLevel,
    StoryPhase,
    QuestComplete,
    ItemInInventory,
    TimeOfDay
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResponseText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_DialogueCondition RequiredCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConditionValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrustModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEndsConversation;

    FNarr_DialogueChoice()
    {
        ChoiceText = TEXT("");
        ResponseText = TEXT("");
        RequiredCondition = ENarr_DialogueCondition::None;
        ConditionValue = 0.0f;
        NextNodeID = TEXT("");
        TrustModifier = 0.0f;
        bEndsConversation = false;
    }
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRootNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AudioAssetPath;

    FNarr_DialogueNode()
    {
        NodeID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        bIsRootNode = false;
        AudioAssetPath = TEXT("");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueTree : public UDataAsset
{
    GENERATED_BODY()

public:
    UNarr_DialogueTree();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RootNodeID;

    // Dialogue tree navigation
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetRootNode() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetNodeByID(const FString& NodeID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> GetAvailableChoices(const FString& NodeID, float PlayerTrustLevel) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool ValidateTree() const;

protected:
    bool CheckCondition(const FNarr_DialogueChoice& Choice, float PlayerTrustLevel) const;
};