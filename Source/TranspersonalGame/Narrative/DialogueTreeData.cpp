#include "DialogueTreeData.h"

// ============================================================
// UNarr_DialogueTree — Implementation
// ============================================================

const FNarr_DialogueNode* UNarr_DialogueTree::GetNode(int32 Index) const
{
    if (Nodes.IsValidIndex(Index))
    {
        return &Nodes[Index];
    }
    return nullptr;
}

const FNarr_DialogueNode* UNarr_DialogueTree::GetEntryNode() const
{
    return GetNode(EntryNodeIndex);
}

bool UNarr_DialogueTree::ValidateTree() const
{
    if (Nodes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueTree [%s]: No nodes defined."), *DialogueID.ToString());
        return false;
    }

    if (!Nodes.IsValidIndex(EntryNodeIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueTree [%s]: EntryNodeIndex %d is out of range (Nodes.Num=%d)."),
            *DialogueID.ToString(), EntryNodeIndex, Nodes.Num());
        return false;
    }

    bool bValid = true;

    for (int32 i = 0; i < Nodes.Num(); ++i)
    {
        const FNarr_DialogueNode& Node = Nodes[i];

        // Validate choice node indices
        if (Node.NodeType == ENarr_DialogueNodeType::PlayerChoice)
        {
            for (const FNarr_DialogueChoice& Choice : Node.Choices)
            {
                if (Choice.NextNodeIndex != -1 && !Nodes.IsValidIndex(Choice.NextNodeIndex))
                {
                    UE_LOG(LogTemp, Error, TEXT("DialogueTree [%s]: Node %d has choice pointing to invalid index %d."),
                        *DialogueID.ToString(), i, Choice.NextNodeIndex);
                    bValid = false;
                }
            }
        }

        // Validate condition branch indices
        if (Node.NodeType == ENarr_DialogueNodeType::Condition)
        {
            if (Node.TrueNodeIndex != -1 && !Nodes.IsValidIndex(Node.TrueNodeIndex))
            {
                UE_LOG(LogTemp, Error, TEXT("DialogueTree [%s]: Node %d TrueNodeIndex %d is invalid."),
                    *DialogueID.ToString(), i, Node.TrueNodeIndex);
                bValid = false;
            }
            if (Node.FalseNodeIndex != -1 && !Nodes.IsValidIndex(Node.FalseNodeIndex))
            {
                UE_LOG(LogTemp, Error, TEXT("DialogueTree [%s]: Node %d FalseNodeIndex %d is invalid."),
                    *DialogueID.ToString(), i, Node.FalseNodeIndex);
                bValid = false;
            }
        }
    }

    if (bValid)
    {
        UE_LOG(LogTemp, Log, TEXT("DialogueTree [%s]: Validation PASSED (%d nodes)."),
            *DialogueID.ToString(), Nodes.Num());
    }

    return bValid;
}
