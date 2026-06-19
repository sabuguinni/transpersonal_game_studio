// DialogueManager.cpp
// Narrative & Dialogue Agent #15 — Full dialogue tree implementation
// Quest integration: Q_FindTribe, Q_ProtectHerd, Q_RaptorThreat

#include "Narrative/DialogueManager.h"

UDialogueManager::UDialogueManager()
    : CurrentState(ENarr_DialogueState::Idle)
    , ActiveTreeID(TEXT(""))
    , CurrentNodeID(TEXT(""))
{
}

void UDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterDefaultDialogueTrees();
}

void UDialogueManager::Deinitialize()
{
    DialogueTrees.Empty();
    Super::Deinitialize();
}

void UDialogueManager::RegisterDefaultDialogueTrees()
{
    RegisterDialogueTree(BuildTribeElderTree());
    RegisterDialogueTree(BuildTribeScoutTree());
}

// -----------------------------------------------------------------------
// Tribe Elder — Quest giver for Q_FindTribe
// -----------------------------------------------------------------------
FNarr_DialogueTree UDialogueManager::BuildTribeElderTree()
{
    FNarr_DialogueTree Tree;
    Tree.TreeID = TEXT("DLG_TribeElder");
    Tree.NPCName = TEXT("Tribe Elder");
    Tree.RootNodeID = TEXT("elder_greet");

    // Node: elder_greet
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("elder_greet");
        Node.SpeakerName = TEXT("Elder Kara");
        Node.SpeakerRole = ENarr_SpeakerRole::NPC;
        Node.DialogueText = TEXT("Stranger. You carry the smell of long travel. Sit. The raptors have been watching our camp from the ridge — three nights now. We cannot move the herd until they are driven back.");
        Node.AudioCueID = TEXT("TribeElder_Dialogue");
        Node.bIsEndNode = false;

        FNarr_DialogueChoice C1;
        C1.ChoiceText = TEXT("I can help. Where are the raptors?");
        C1.NextNodeID = TEXT("elder_raptor_info");
        C1.QuestIDToActivate = TEXT("Q_RaptorThreat");
        C1.bEndsDialogue = false;

        FNarr_DialogueChoice C2;
        C2.ChoiceText = TEXT("How long has your tribe been here?");
        C2.NextNodeID = TEXT("elder_tribe_history");
        C2.bEndsDialogue = false;

        FNarr_DialogueChoice C3;
        C3.ChoiceText = TEXT("I need to keep moving.");
        C3.NextNodeID = TEXT("");
        C3.bEndsDialogue = true;

        Node.Choices.Add(C1);
        Node.Choices.Add(C2);
        Node.Choices.Add(C3);
        Tree.Nodes.Add(Node);
    }

    // Node: elder_raptor_info
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("elder_raptor_info");
        Node.SpeakerName = TEXT("Elder Kara");
        Node.SpeakerRole = ENarr_SpeakerRole::NPC;
        Node.DialogueText = TEXT("North — past the dry riverbed. My scout tracked them two days ago. Four adults, maybe more. Find their den. If you can collapse the entrance with heavy stones, they will move on.");
        Node.bIsEndNode = false;

        FNarr_DialogueChoice C1;
        C1.ChoiceText = TEXT("I will find the den.");
        C1.NextNodeID = TEXT("elder_send_off");
        C1.bEndsDialogue = false;

        FNarr_DialogueChoice C2;
        C2.ChoiceText = TEXT("What do you have to trade for this?");
        C2.NextNodeID = TEXT("elder_trade");
        C2.bEndsDialogue = false;

        Node.Choices.Add(C1);
        Node.Choices.Add(C2);
        Tree.Nodes.Add(Node);
    }

    // Node: elder_tribe_history
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("elder_tribe_history");
        Node.SpeakerName = TEXT("Elder Kara");
        Node.SpeakerRole = ENarr_SpeakerRole::NPC;
        Node.DialogueText = TEXT("Three seasons. We followed the great lizards south when the northern valley flooded. They know where the water is. Where they go, we go. It is not complicated.");
        Node.bIsEndNode = false;

        FNarr_DialogueChoice C1;
        C1.ChoiceText = TEXT("The raptors — where are they?");
        C1.NextNodeID = TEXT("elder_raptor_info");
        C1.QuestIDToActivate = TEXT("Q_RaptorThreat");
        C1.bEndsDialogue = false;

        FNarr_DialogueChoice C2;
        C2.ChoiceText = TEXT("I will leave you to it.");
        C2.bEndsDialogue = true;

        Node.Choices.Add(C1);
        Node.Choices.Add(C2);
        Tree.Nodes.Add(Node);
    }

    // Node: elder_trade
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("elder_trade");
        Node.SpeakerName = TEXT("Elder Kara");
        Node.SpeakerRole = ENarr_SpeakerRole::NPC;
        Node.DialogueText = TEXT("Drive off the raptors and I will show you how to shape the black stone into a blade that holds its edge. Better than flint. You will not find that knowledge anywhere else.");
        Node.bIsEndNode = false;

        FNarr_DialogueChoice C1;
        C1.ChoiceText = TEXT("Agreed. I will find the den.");
        C1.NextNodeID = TEXT("elder_send_off");
        C1.bEndsDialogue = false;

        Node.Choices.Add(C1);
        Tree.Nodes.Add(Node);
    }

    // Node: elder_send_off
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("elder_send_off");
        Node.SpeakerName = TEXT("Elder Kara");
        Node.SpeakerRole = ENarr_SpeakerRole::NPC;
        Node.DialogueText = TEXT("Take the north path. Stay low near the ridge — they hunt by movement. Come back when it is done.");
        Node.bIsEndNode = true;
        Tree.Nodes.Add(Node);
    }

    return Tree;
}

// -----------------------------------------------------------------------
// Tribe Scout — Quest giver for Q_ProtectHerd
// -----------------------------------------------------------------------
FNarr_DialogueTree UDialogueManager::BuildTribeScoutTree()
{
    FNarr_DialogueTree Tree;
    Tree.TreeID = TEXT("DLG_TribeScout");
    Tree.NPCName = TEXT("Tribe Scout");
    Tree.RootNodeID = TEXT("scout_greet");

    // Node: scout_greet
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("scout_greet");
        Node.SpeakerName = TEXT("Scout Davan");
        Node.SpeakerRole = ENarr_SpeakerRole::NPC;
        Node.DialogueText = TEXT("The herd is moving! Three adults and two young ones — if the raptors catch them in the open valley, they will not survive. We need to get ahead of them and drive the predators off.");
        Node.AudioCueID = TEXT("TribeScout_Dialogue");
        Node.bIsEndNode = false;

        FNarr_DialogueChoice C1;
        C1.ChoiceText = TEXT("Which way is the herd moving?");
        C1.NextNodeID = TEXT("scout_herd_direction");
        C1.QuestIDToActivate = TEXT("Q_ProtectHerd");
        C1.bEndsDialogue = false;

        FNarr_DialogueChoice C2;
        C2.ChoiceText = TEXT("How many raptors are following them?");
        C2.NextNodeID = TEXT("scout_raptor_count");
        C2.bEndsDialogue = false;

        FNarr_DialogueChoice C3;
        C3.ChoiceText = TEXT("Not my problem.");
        C3.bEndsDialogue = true;

        Node.Choices.Add(C1);
        Node.Choices.Add(C2);
        Node.Choices.Add(C3);
        Tree.Nodes.Add(Node);
    }

    // Node: scout_herd_direction
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("scout_herd_direction");
        Node.SpeakerName = TEXT("Scout Davan");
        Node.SpeakerRole = ENarr_SpeakerRole::NPC;
        Node.DialogueText = TEXT("Southwest — toward the river crossing. The young ones are slow. If you cut through the ravine you can get ahead of them. Drive the raptors toward the cliffs — they will not follow into broken ground.");
        Node.bIsEndNode = false;

        FNarr_DialogueChoice C1;
        C1.ChoiceText = TEXT("I will go now.");
        C1.NextNodeID = TEXT("scout_send_off");
        C1.bEndsDialogue = false;

        Node.Choices.Add(C1);
        Tree.Nodes.Add(Node);
    }

    // Node: scout_raptor_count
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("scout_raptor_count");
        Node.SpeakerName = TEXT("Scout Davan");
        Node.SpeakerRole = ENarr_SpeakerRole::NPC;
        Node.DialogueText = TEXT("Three. Maybe four — I lost one in the brush. They are coordinating. One drives from behind, the others wait at the flanks. Do not let them separate you from the herd.");
        Node.bIsEndNode = false;

        FNarr_DialogueChoice C1;
        C1.ChoiceText = TEXT("Southwest — I will cut through the ravine.");
        C1.NextNodeID = TEXT("scout_send_off");
        C1.bEndsDialogue = false;

        Node.Choices.Add(C1);
        Tree.Nodes.Add(Node);
    }

    // Node: scout_send_off
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("scout_send_off");
        Node.SpeakerName = TEXT("Scout Davan");
        Node.SpeakerRole = ENarr_SpeakerRole::NPC;
        Node.DialogueText = TEXT("Go. I will watch from the ridge and signal if they change direction. Three fires means they turned north.");
        Node.bIsEndNode = true;
        Tree.Nodes.Add(Node);
    }

    return Tree;
}

// -----------------------------------------------------------------------
// Core dialogue system methods
// -----------------------------------------------------------------------

void UDialogueManager::RegisterDialogueTree(const FNarr_DialogueTree& Tree)
{
    if (!Tree.TreeID.IsEmpty())
    {
        DialogueTrees.Add(Tree.TreeID, Tree);
    }
}

bool UDialogueManager::HasDialogueTree(const FString& TreeID) const
{
    return DialogueTrees.Contains(TreeID);
}

bool UDialogueManager::StartDialogue(const FString& TreeID)
{
    if (!DialogueTrees.Contains(TreeID))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Tree '%s' not found"), *TreeID);
        return false;
    }

    const FNarr_DialogueTree& Tree = DialogueTrees[TreeID];
    ActiveTreeID = TreeID;
    CurrentNodeID = Tree.RootNodeID;
    CurrentState = ENarr_DialogueState::Active;

    const FNarr_DialogueNode* RootNode = FindNode(Tree, Tree.RootNodeID);
    if (RootNode)
    {
        OnDialogueNodeChanged.Broadcast(CurrentNodeID, *RootNode);
        if (RootNode->Choices.Num() > 0)
        {
            CurrentState = ENarr_DialogueState::Waiting;
        }
    }

    return true;
}

bool UDialogueManager::SelectChoice(int32 ChoiceIndex)
{
    if (CurrentState != ENarr_DialogueState::Waiting)
    {
        return false;
    }

    if (!DialogueTrees.Contains(ActiveTreeID))
    {
        return false;
    }

    const FNarr_DialogueTree& Tree = DialogueTrees[ActiveTreeID];
    const FNarr_DialogueNode* CurrentNode = FindNode(Tree, CurrentNodeID);

    if (!CurrentNode || !CurrentNode->Choices.IsValidIndex(ChoiceIndex))
    {
        return false;
    }

    const FNarr_DialogueChoice& Choice = CurrentNode->Choices[ChoiceIndex];

    // Activate quest if specified
    if (!Choice.QuestIDToActivate.IsEmpty())
    {
        // Quest activation handled by QuestManager subsystem — log for now
        UE_LOG(LogTemp, Log, TEXT("DialogueManager: Activating quest '%s'"), *Choice.QuestIDToActivate);
    }

    if (Choice.bEndsDialogue || Choice.NextNodeID.IsEmpty())
    {
        EndDialogue();
        return true;
    }

    return AdvanceToNode(Choice.NextNodeID);
}

bool UDialogueManager::AdvanceToNode(const FString& NodeID)
{
    if (!DialogueTrees.Contains(ActiveTreeID))
    {
        return false;
    }

    const FNarr_DialogueTree& Tree = DialogueTrees[ActiveTreeID];
    const FNarr_DialogueNode* Node = FindNode(Tree, NodeID);

    if (!Node)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Node '%s' not found in tree '%s'"), *NodeID, *ActiveTreeID);
        return false;
    }

    CurrentNodeID = NodeID;
    CurrentState = Node->Choices.Num() > 0 ? ENarr_DialogueState::Waiting : ENarr_DialogueState::Active;

    OnDialogueNodeChanged.Broadcast(CurrentNodeID, *Node);

    if (Node->bIsEndNode)
    {
        EndDialogue();
    }

    return true;
}

void UDialogueManager::EndDialogue()
{
    FString EndedTreeID = ActiveTreeID;
    CurrentState = ENarr_DialogueState::Completed;
    ActiveTreeID = TEXT("");
    CurrentNodeID = TEXT("");
    OnDialogueEnded.Broadcast(EndedTreeID);
}

FNarr_DialogueNode UDialogueManager::GetCurrentNode() const
{
    if (!DialogueTrees.Contains(ActiveTreeID))
    {
        return FNarr_DialogueNode();
    }

    const FNarr_DialogueTree& Tree = DialogueTrees[ActiveTreeID];
    const FNarr_DialogueNode* Node = FindNode(Tree, CurrentNodeID);
    return Node ? *Node : FNarr_DialogueNode();
}

const FNarr_DialogueNode* UDialogueManager::FindNode(const FNarr_DialogueTree& Tree, const FString& NodeID) const
{
    for (const FNarr_DialogueNode& Node : Tree.Nodes)
    {
        if (Node.NodeID == NodeID)
        {
            return &Node;
        }
    }
    return nullptr;
}
