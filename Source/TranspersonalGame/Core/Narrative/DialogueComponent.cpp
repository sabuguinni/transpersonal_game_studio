#include "DialogueComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UDialogueComponent::UDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsDialogueActive = false;
    CurrentTreeIndex = -1;
    CurrentNodeID = 0;
    CurrentLineIndex = 0;
}

void UDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default dialogue trees for tribal characters
    if (DialogueTrees.Num() == 0)
    {
        // Create a default greeting tree
        FNarr_DialogueTree GreetingTree;
        GreetingTree.TreeName = TEXT("Greeting");
        GreetingTree.StartNodeID = 0;

        FNarr_DialogueNode GreetingNode;
        GreetingNode.NodeID = 0;
        GreetingNode.bEndsConversation = true;

        FNarr_DialogueLine GreetingLine;
        GreetingLine.SpeakerName = TEXT("Tribal Hunter");
        GreetingLine.DialogueText = FText::FromString(TEXT("The beasts grow restless. Stay alert, traveler."));
        GreetingLine.Duration = 3.0f;

        GreetingNode.Lines.Add(GreetingLine);
        GreetingTree.Nodes.Add(GreetingNode);
        DialogueTrees.Add(GreetingTree);
    }
}

void UDialogueComponent::StartDialogue(const FString& TreeName)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already active"));
        return;
    }

    // Find the dialogue tree
    CurrentTreeIndex = -1;
    for (int32 i = 0; i < DialogueTrees.Num(); i++)
    {
        if (DialogueTrees[i].TreeName == TreeName)
        {
            CurrentTreeIndex = i;
            break;
        }
    }

    if (CurrentTreeIndex == -1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue tree '%s' not found"), *TreeName);
        return;
    }

    bIsDialogueActive = true;
    CurrentNodeID = DialogueTrees[CurrentTreeIndex].StartNodeID;
    CurrentLineIndex = 0;

    ProcessCurrentNode();
}

void UDialogueComponent::AdvanceDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    FNarr_DialogueNode* CurrentNode = GetCurrentNode();
    if (!CurrentNode)
    {
        EndDialogue();
        return;
    }

    CurrentLineIndex++;

    // Check if we've finished all lines in this node
    if (CurrentLineIndex >= CurrentNode->Lines.Num())
    {
        if (CurrentNode->bEndsConversation)
        {
            EndDialogue();
            return;
        }

        // Check for player choices
        if (CurrentNode->NextNodeIDs.Num() > 1)
        {
            HandlePlayerChoices(*CurrentNode);
            return;
        }

        // Move to next node if only one option
        if (CurrentNode->NextNodeIDs.Num() == 1)
        {
            CurrentNodeID = CurrentNode->NextNodeIDs[0];
            CurrentLineIndex = 0;
            ProcessCurrentNode();
        }
        else
        {
            EndDialogue();
        }
    }
    else
    {
        ProcessCurrentNode();
    }
}

void UDialogueComponent::MakeChoice(int32 ChoiceIndex)
{
    if (!bIsDialogueActive)
    {
        return;
    }

    FNarr_DialogueNode* CurrentNode = GetCurrentNode();
    if (!CurrentNode || ChoiceIndex < 0 || ChoiceIndex >= CurrentNode->NextNodeIDs.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid choice index: %d"), ChoiceIndex);
        return;
    }

    CurrentNodeID = CurrentNode->NextNodeIDs[ChoiceIndex];
    CurrentLineIndex = 0;
    ProcessCurrentNode();
}

void UDialogueComponent::EndDialogue()
{
    bIsDialogueActive = false;
    CurrentTreeIndex = -1;
    CurrentNodeID = 0;
    CurrentLineIndex = 0;

    OnDialogueEnded.Broadcast();
}

void UDialogueComponent::AddDialogueTree(const FNarr_DialogueTree& NewTree)
{
    // Check if tree with this name already exists
    for (int32 i = 0; i < DialogueTrees.Num(); i++)
    {
        if (DialogueTrees[i].TreeName == NewTree.TreeName)
        {
            DialogueTrees[i] = NewTree;
            return;
        }
    }

    DialogueTrees.Add(NewTree);
}

bool UDialogueComponent::HasDialogueTree(const FString& TreeName) const
{
    for (const FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeName == TreeName)
        {
            return true;
        }
    }
    return false;
}

void UDialogueComponent::ProcessCurrentNode()
{
    FNarr_DialogueNode* CurrentNode = GetCurrentNode();
    if (!CurrentNode || CurrentLineIndex >= CurrentNode->Lines.Num())
    {
        EndDialogue();
        return;
    }

    const FNarr_DialogueLine& CurrentLine = CurrentNode->Lines[CurrentLineIndex];
    
    // Broadcast dialogue started event
    OnDialogueStarted.Broadcast(CurrentLine.SpeakerName, CurrentLine.DialogueText);

    // Log dialogue for debugging
    UE_LOG(LogTemp, Log, TEXT("%s: %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText.ToString());
}

FNarr_DialogueNode* UDialogueComponent::GetCurrentNode()
{
    if (CurrentTreeIndex < 0 || CurrentTreeIndex >= DialogueTrees.Num())
    {
        return nullptr;
    }

    FNarr_DialogueTree& CurrentTree = DialogueTrees[CurrentTreeIndex];
    for (FNarr_DialogueNode& Node : CurrentTree.Nodes)
    {
        if (Node.NodeID == CurrentNodeID)
        {
            return &Node;
        }
    }

    return nullptr;
}

void UDialogueComponent::HandlePlayerChoices(const FNarr_DialogueNode& Node)
{
    TArray<FText> Choices;
    
    // For now, create generic choice text based on next node count
    for (int32 i = 0; i < Node.NextNodeIDs.Num(); i++)
    {
        FString ChoiceText = FString::Printf(TEXT("Choice %d"), i + 1);
        Choices.Add(FText::FromString(ChoiceText));
    }

    OnDialogueChoiceRequired.Broadcast(Choices);
}