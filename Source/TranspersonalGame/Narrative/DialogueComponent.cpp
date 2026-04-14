#include "DialogueComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UDialogueComponent::UDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    bDialogueActive = false;
    CurrentNodeID = 0;
    CurrentTreeName = TEXT("");
    DialogueDataTable = nullptr;
}

void UDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default dialogue trees
    FNarr_DialogueTree WelcomeTree;
    WelcomeTree.TreeName = TEXT("WelcomeTree");
    WelcomeTree.SpeakerArchetype = ENarr_CharacterArchetype::Wise;
    
    FNarr_DialogueNode WelcomeNode;
    WelcomeNode.SpeakerName = TEXT("Elder Mystic");
    WelcomeNode.DialogueText = FText::FromString(TEXT("Welcome, awakened one. You stand at the threshold of understanding."));
    WelcomeNode.EmotionalTone = ENarr_EmotionalState::Peaceful;
    WelcomeNode.PlayerResponses.Add(TEXT("Tell me more about this place."));
    WelcomeNode.PlayerResponses.Add(TEXT("I'm ready to begin my journey."));
    WelcomeNode.NextNodeIDs.Add(1);
    WelcomeNode.NextNodeIDs.Add(2);
    
    WelcomeTree.DialogueNodes.Add(WelcomeNode);
    WelcomeTree.RootNodeID = 0;
    
    AddDialogueTree(WelcomeTree);
}

void UDialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Handle dialogue timing and auto-advance if needed
    if (bDialogueActive)
    {
        // Future: Add auto-advance timer logic here
    }
}

void UDialogueComponent::StartDialogue(const FString& TreeName)
{
    if (bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already active. Ending current dialogue first."));
        EndDialogue();
    }

    if (!HasDialogueTree(TreeName))
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue tree '%s' not found!"), *TreeName);
        return;
    }

    const FNarr_DialogueTree* Tree = DialogueTrees.Find(TreeName);
    if (!Tree || Tree->DialogueNodes.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid dialogue tree: %s"), *TreeName);
        return;
    }

    CurrentTreeName = TreeName;
    CurrentNodeID = Tree->RootNodeID;
    
    if (Tree->DialogueNodes.IsValidIndex(CurrentNodeID))
    {
        CurrentNode = Tree->DialogueNodes[CurrentNodeID];
        
        // Check consciousness requirements
        if (!CheckConsciousnessRequirement(CurrentNode))
        {
            UE_LOG(LogTemp, Warning, TEXT("Player does not meet consciousness requirements for this dialogue"));
            return;
        }
        
        bDialogueActive = true;
        SetComponentTickEnabled(true);
        
        // Broadcast dialogue started event
        OnDialogueStarted.Broadcast(CurrentNode.SpeakerName, CurrentNode.DialogueText);
        
        UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s - %s"), *CurrentNode.SpeakerName, *CurrentNode.DialogueText.ToString());
    }
}

void UDialogueComponent::EndDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    bDialogueActive = false;
    SetComponentTickEnabled(false);
    
    // Broadcast dialogue ended event
    OnDialogueEnded.Broadcast(true);
    
    // Reset state
    CurrentTreeName = TEXT("");
    CurrentNodeID = 0;
    CurrentNode = FNarr_DialogueNode();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

void UDialogueComponent::SelectPlayerResponse(int32 ResponseIndex)
{
    if (!bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("No active dialogue to respond to"));
        return;
    }

    if (!CurrentNode.PlayerResponses.IsValidIndex(ResponseIndex) || 
        !CurrentNode.NextNodeIDs.IsValidIndex(ResponseIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid response index: %d"), ResponseIndex);
        return;
    }

    FString SelectedResponse = CurrentNode.PlayerResponses[ResponseIndex];
    int32 NextNodeID = CurrentNode.NextNodeIDs[ResponseIndex];
    
    // Broadcast response selection event
    OnPlayerResponseSelected.Broadcast(ResponseIndex, SelectedResponse, NextNodeID);
    
    UE_LOG(LogTemp, Log, TEXT("Player selected response: %s"), *SelectedResponse);
    
    // Advance to next node
    AdvanceToNextNode(NextNodeID);
}

void UDialogueComponent::AdvanceToNextNode(int32 NodeID)
{
    if (!bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot advance dialogue - no active dialogue"));
        return;
    }

    const FNarr_DialogueTree* Tree = DialogueTrees.Find(CurrentTreeName);
    if (!Tree)
    {
        UE_LOG(LogTemp, Error, TEXT("Current dialogue tree not found: %s"), *CurrentTreeName);
        EndDialogue();
        return;
    }

    // Check if NodeID is valid (negative means end dialogue)
    if (NodeID < 0 || !Tree->DialogueNodes.IsValidIndex(NodeID))
    {
        UE_LOG(LogTemp, Log, TEXT("Reached end of dialogue tree"));
        EndDialogue();
        return;
    }

    // Validate transition
    if (!ValidateNodeTransition(CurrentNodeID, NodeID))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid node transition from %d to %d"), CurrentNodeID, NodeID);
        return;
    }

    // Update current node
    CurrentNodeID = NodeID;
    CurrentNode = Tree->DialogueNodes[NodeID];
    
    // Check consciousness requirements for new node
    if (!CheckConsciousnessRequirement(CurrentNode))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player consciousness level insufficient for node %d"), NodeID);
        EndDialogue();
        return;
    }
    
    // Process any dialogue effects
    ProcessDialogueEffects(CurrentNode);
    
    // Broadcast new dialogue content
    OnDialogueStarted.Broadcast(CurrentNode.SpeakerName, CurrentNode.DialogueText);
    
    UE_LOG(LogTemp, Log, TEXT("Advanced to node %d: %s"), NodeID, *CurrentNode.DialogueText.ToString());
}

void UDialogueComponent::LoadDialogueTree(const FString& TreeName)
{
    // Future: Load from data table or external file
    UE_LOG(LogTemp, Log, TEXT("Loading dialogue tree: %s"), *TreeName);
    
    if (DialogueDataTable)
    {
        // Implementation for loading from data table would go here
        UE_LOG(LogTemp, Log, TEXT("Dialogue data table found, but loading not yet implemented"));
    }
}

void UDialogueComponent::AddDialogueTree(const FNarr_DialogueTree& NewTree)
{
    if (NewTree.TreeName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot add dialogue tree with empty name"));
        return;
    }

    DialogueTrees.Add(NewTree.TreeName, NewTree);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue tree: %s with %d nodes"), *NewTree.TreeName, NewTree.DialogueNodes.Num());
}

bool UDialogueComponent::HasDialogueTree(const FString& TreeName) const
{
    return DialogueTrees.Contains(TreeName);
}

bool UDialogueComponent::ValidateNodeTransition(int32 FromNodeID, int32 ToNodeID) const
{
    const FNarr_DialogueTree* Tree = DialogueTrees.Find(CurrentTreeName);
    if (!Tree || !Tree->DialogueNodes.IsValidIndex(FromNodeID))
    {
        return false;
    }

    const FNarr_DialogueNode& FromNode = Tree->DialogueNodes[FromNodeID];
    return FromNode.NextNodeIDs.Contains(ToNodeID) || ToNodeID < 0; // Allow negative IDs for dialogue end
}

bool UDialogueComponent::CheckConsciousnessRequirement(const FNarr_DialogueNode& Node) const
{
    if (!Node.bRequiresConsciousnessLevel)
    {
        return true;
    }

    // Future: Get player consciousness level from game state
    // For now, assume player meets requirements
    int32 PlayerConsciousnessLevel = 1; // Placeholder
    
    return PlayerConsciousnessLevel >= Node.MinConsciousnessLevel;
}

void UDialogueComponent::ProcessDialogueEffects(const FNarr_DialogueNode& Node)
{
    // Future: Process dialogue effects like:
    // - Consciousness level changes
    // - Relationship changes
    // - Quest triggers
    // - Item rewards
    
    UE_LOG(LogTemp, Log, TEXT("Processing dialogue effects for node with emotional tone: %d"), (int32)Node.EmotionalTone);
}