#include "Narr_DialogueTree.h"
#include "Engine/Engine.h"

UNarr_DialogueTree::UNarr_DialogueTree()
{
    TreeName = TEXT("Default Dialogue Tree");
    CharacterName = TEXT("Unknown");
    RootNodeID = TEXT("root");
}

FNarr_DialogueNode UNarr_DialogueTree::GetRootNode() const
{
    for (const FNarr_DialogueNode& Node : DialogueNodes)
    {
        if (Node.bIsRootNode || Node.NodeID == RootNodeID)
        {
            return Node;
        }
    }
    
    // Return empty node if no root found
    return FNarr_DialogueNode();
}

FNarr_DialogueNode UNarr_DialogueTree::GetNodeByID(const FString& NodeID) const
{
    for (const FNarr_DialogueNode& Node : DialogueNodes)
    {
        if (Node.NodeID == NodeID)
        {
            return Node;
        }
    }
    
    // Return empty node if not found
    return FNarr_DialogueNode();
}

TArray<FNarr_DialogueChoice> UNarr_DialogueTree::GetAvailableChoices(const FString& NodeID, float PlayerTrustLevel) const
{
    TArray<FNarr_DialogueChoice> AvailableChoices;
    
    FNarr_DialogueNode Node = GetNodeByID(NodeID);
    if (Node.NodeID.IsEmpty())
    {
        return AvailableChoices;
    }
    
    for (const FNarr_DialogueChoice& Choice : Node.Choices)
    {
        if (CheckCondition(Choice, PlayerTrustLevel))
        {
            AvailableChoices.Add(Choice);
        }
    }
    
    return AvailableChoices;
}

bool UNarr_DialogueTree::ValidateTree() const
{
    if (DialogueNodes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue tree %s has no nodes"), *TreeName);
        return false;
    }
    
    // Check for root node
    bool bHasRootNode = false;
    for (const FNarr_DialogueNode& Node : DialogueNodes)
    {
        if (Node.bIsRootNode || Node.NodeID == RootNodeID)
        {
            bHasRootNode = true;
            break;
        }
    }
    
    if (!bHasRootNode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue tree %s has no root node"), *TreeName);
        return false;
    }
    
    // Check for broken references
    for (const FNarr_DialogueNode& Node : DialogueNodes)
    {
        for (const FNarr_DialogueChoice& Choice : Node.Choices)
        {
            if (!Choice.NextNodeID.IsEmpty() && !Choice.bEndsConversation)
            {
                FNarr_DialogueNode NextNode = GetNodeByID(Choice.NextNodeID);
                if (NextNode.NodeID.IsEmpty())
                {
                    UE_LOG(LogTemp, Warning, TEXT("Dialogue tree %s has broken reference: %s -> %s"), 
                        *TreeName, *Node.NodeID, *Choice.NextNodeID);
                    return false;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue tree %s validation passed"), *TreeName);
    return true;
}

bool UNarr_DialogueTree::CheckCondition(const FNarr_DialogueChoice& Choice, float PlayerTrustLevel) const
{
    switch (Choice.RequiredCondition)
    {
        case ENarr_DialogueCondition::None:
            return true;
            
        case ENarr_DialogueCondition::TrustLevel:
            return PlayerTrustLevel >= Choice.ConditionValue;
            
        case ENarr_DialogueCondition::StoryPhase:
            // Would need access to story manager for this
            return true;
            
        case ENarr_DialogueCondition::QuestComplete:
            // Would need access to quest system for this
            return true;
            
        case ENarr_DialogueCondition::ItemInInventory:
            // Would need access to inventory system for this
            return true;
            
        case ENarr_DialogueCondition::TimeOfDay:
            // Would need access to time system for this
            return true;
            
        default:
            return true;
    }
}