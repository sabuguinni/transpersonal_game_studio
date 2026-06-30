#include "DialogueSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// ============================================================
// UNarr_DialogueComponent — Implementation
// ============================================================

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    NPCRole = ENarr_NPCRole::Survivor;
    bIsInDialogue = false;
    InteractionRadius = 300.0f;
    CurrentLineIndex = 0;

    // Seed a default conversation tree so CDO is never empty
    ConversationTree.ConversationID = TEXT("default_survivor");
    ConversationTree.NPCRole = ENarr_NPCRole::Survivor;
    ConversationTree.RootNodeID = TEXT("root");

    FNarr_DialogueLine DefaultLine;
    DefaultLine.SpeakerID = TEXT("Survivor");
    DefaultLine.LineText = FText::FromString(TEXT("Stay close to the fire at night. The big ones do not like the light."));
    DefaultLine.Tone = ENarr_DialogueTone::Warning;
    DefaultLine.DisplayDuration = 5.0f;

    FNarr_DialogueNode RootNode;
    RootNode.NodeID = TEXT("root");
    RootNode.Lines.Add(DefaultLine);
    RootNode.bEndsConversation = true;

    ConversationTree.Nodes.Add(RootNode);
    CurrentNodeID = ConversationTree.RootNodeID;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentNodeID = ConversationTree.RootNodeID;
    CurrentLineIndex = 0;
    bIsInDialogue = false;
}

void UNarr_DialogueComponent::StartDialogue()
{
    if (bIsInDialogue)
    {
        return;
    }

    CurrentNodeID = ConversationTree.RootNodeID;
    CurrentLineIndex = 0;
    bIsInDialogue = true;

    UE_LOG(LogTemp, Log, TEXT("[DialogueSystem] Started conversation: %s | NPC Role: %d"),
        *ConversationTree.ConversationID,
        (int32)NPCRole);
}

void UNarr_DialogueComponent::AdvanceDialogue(const FString& ChosenResponseID)
{
    if (!bIsInDialogue)
    {
        return;
    }

    FNarr_DialogueNode* Node = FindNode(CurrentNodeID);
    if (!Node)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueSystem] Node not found: %s"), *CurrentNodeID);
        EndDialogue();
        return;
    }

    // If a response was chosen, navigate to that node
    if (!ChosenResponseID.IsEmpty())
    {
        CurrentNodeID = ChosenResponseID;
        CurrentLineIndex = 0;
        UE_LOG(LogTemp, Log, TEXT("[DialogueSystem] Navigating to node: %s"), *CurrentNodeID);
        return;
    }

    // Otherwise follow the linear next node
    if (!Node->NextNodeID.IsEmpty())
    {
        CurrentNodeID = Node->NextNodeID;
        CurrentLineIndex = 0;
        UE_LOG(LogTemp, Log, TEXT("[DialogueSystem] Advanced to node: %s"), *CurrentNodeID);
    }
    else if (Node->bEndsConversation)
    {
        EndDialogue();
    }
}

void UNarr_DialogueComponent::EndDialogue()
{
    bIsInDialogue = false;
    CurrentNodeID = ConversationTree.RootNodeID;
    CurrentLineIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("[DialogueSystem] Conversation ended: %s"), *ConversationTree.ConversationID);
}

FNarr_DialogueNode UNarr_DialogueComponent::GetCurrentNode() const
{
    for (const FNarr_DialogueNode& Node : ConversationTree.Nodes)
    {
        if (Node.NodeID == CurrentNodeID)
        {
            return Node;
        }
    }
    return FNarr_DialogueNode();
}

int32 UNarr_DialogueComponent::GetCurrentLineIndex() const
{
    return CurrentLineIndex;
}

bool UNarr_DialogueComponent::AdvanceLine()
{
    FNarr_DialogueNode* Node = FindNode(CurrentNodeID);
    if (!Node)
    {
        return false;
    }

    if (CurrentLineIndex + 1 < Node->Lines.Num())
    {
        CurrentLineIndex++;
        UE_LOG(LogTemp, Log, TEXT("[DialogueSystem] Line advanced to %d in node %s"),
            CurrentLineIndex, *CurrentNodeID);
        return true;
    }

    // No more lines in this node
    return false;
}

FNarr_DialogueNode* UNarr_DialogueComponent::FindNode(const FString& NodeID)
{
    for (FNarr_DialogueNode& Node : ConversationTree.Nodes)
    {
        if (Node.NodeID == NodeID)
        {
            return &Node;
        }
    }
    return nullptr;
}
