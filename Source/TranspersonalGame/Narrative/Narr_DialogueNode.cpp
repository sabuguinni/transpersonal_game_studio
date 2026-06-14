#include "Narr_DialogueNode.h"

UNarr_DialogueNode::UNarr_DialogueNode()
{
    NodeID = TEXT("");
    NodeType = ENarr_DialogueNodeType::NPCResponse;
    SpeakerName = FText::GetEmpty();
    DialogueText = FText::GetEmpty();
    AudioClipPath = TEXT("");
    bEndsConversation = false;
    TriggerEventID = TEXT("");
}

bool UNarr_DialogueNode::CanSelectChoice(int32 ChoiceIndex, const TArray<FString>& PlayerInventory) const
{
    if (!PlayerChoices.IsValidIndex(ChoiceIndex))
    {
        return false;
    }

    const FNarr_DialogueChoice& Choice = PlayerChoices[ChoiceIndex];
    
    if (!Choice.bRequiresItem)
    {
        return true;
    }

    return PlayerInventory.Contains(Choice.RequiredItemID);
}

FString UNarr_DialogueNode::GetNextNodeID(int32 ChoiceIndex) const
{
    if (PlayerChoices.IsValidIndex(ChoiceIndex))
    {
        return PlayerChoices[ChoiceIndex].NextNodeID;
    }
    
    return TEXT("");
}

bool UNarr_DialogueNode::HasValidChoices(const TArray<FString>& PlayerInventory) const
{
    for (int32 i = 0; i < PlayerChoices.Num(); i++)
    {
        if (CanSelectChoice(i, PlayerInventory))
        {
            return true;
        }
    }
    
    return false;
}