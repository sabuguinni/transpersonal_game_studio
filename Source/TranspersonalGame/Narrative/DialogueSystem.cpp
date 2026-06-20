#include "DialogueSystem.h"

// ============================================================
// Constructor
// ============================================================

ADialogueSystem::ADialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    bDialogueActive = false;
    ActiveLineIndex = 0;
    LineTimer = 0.0f;
}

// ============================================================
// BeginPlay — register all default trees
// ============================================================

void ADialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    InitialiseDefaultTrees();
}

// ============================================================
// Tick — auto-advance lines when timer expires (no choices)
// ============================================================

void ADialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDialogueActive) return;

    FNarr_DialogueTree* Tree = FindTree(ActiveTreeID);
    if (!Tree) return;

    FNarr_DialogueNode* Node = FindNode(ActiveTreeID, ActiveNodeID);
    if (!Node) return;

    // Auto-advance only when there are no player choices
    if (Node->ChoiceLabels.Num() == 0)
    {
        LineTimer += DeltaTime;
        if (ActiveLineIndex < Node->Lines.Num())
        {
            float Duration = Node->Lines[ActiveLineIndex].DisplayDuration;
            if (LineTimer >= Duration)
            {
                LineTimer = 0.0f;
                ActiveLineIndex++;

                if (ActiveLineIndex >= Node->Lines.Num())
                {
                    // All lines played — auto-advance to next node
                    AdvanceDialogue(0);
                }
            }
        }
    }
}

// ============================================================
// StartDialogue
// ============================================================

void ADialogueSystem::StartDialogue(const FString& TreeID)
{
    FNarr_DialogueTree* Tree = FindTree(TreeID);
    if (!Tree)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueSystem: Tree '%s' not found"), *TreeID);
        return;
    }

    bDialogueActive = true;
    ActiveTreeID = TreeID;
    ActiveNodeID = Tree->EntryNodeID;
    ActiveLineIndex = 0;
    LineTimer = 0.0f;

    // Mark entry node active
    FNarr_DialogueNode* EntryNode = FindNode(TreeID, Tree->EntryNodeID);
    if (EntryNode)
    {
        EntryNode->State = ENarr_DialogueState::Active;
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Started tree '%s', entry node '%s'"), *TreeID, *Tree->EntryNodeID);
}

// ============================================================
// AdvanceDialogue — move to next node via choice or auto
// ============================================================

void ADialogueSystem::AdvanceDialogue(int32 ChoiceIndex)
{
    if (!bDialogueActive) return;

    FNarr_DialogueNode* CurrentNode = FindNode(ActiveTreeID, ActiveNodeID);
    if (!CurrentNode) { EndDialogue(); return; }

    // Mark current node completed
    CurrentNode->State = ENarr_DialogueState::Completed;

    // Activate quest if specified
    if (!CurrentNode->ActivatesQuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Activating quest '%s'"), *CurrentNode->ActivatesQuestID);
        // Quest manager integration hook — broadcast via delegate in full integration
    }

    // Find next node
    if (CurrentNode->NextNodeIDs.Num() == 0)
    {
        EndDialogue();
        return;
    }

    int32 SafeIndex = FMath::Clamp(ChoiceIndex, 0, CurrentNode->NextNodeIDs.Num() - 1);
    FString NextID = CurrentNode->NextNodeIDs[SafeIndex];

    if (NextID.IsEmpty() || NextID == "END")
    {
        EndDialogue();
        return;
    }

    ActiveNodeID = NextID;
    ActiveLineIndex = 0;
    LineTimer = 0.0f;

    FNarr_DialogueNode* NextNode = FindNode(ActiveTreeID, NextID);
    if (NextNode)
    {
        NextNode->State = ENarr_DialogueState::Active;
    }
}

// ============================================================
// EndDialogue
// ============================================================

void ADialogueSystem::EndDialogue()
{
    bDialogueActive = false;
    ActiveTreeID = FString();
    ActiveNodeID = FString();
    ActiveLineIndex = 0;
    LineTimer = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Dialogue ended"));
}

// ============================================================
// GetCurrentLine
// ============================================================

FNarr_DialogueLine ADialogueSystem::GetCurrentLine() const
{
    // Const-cast needed because FindNode is non-const (modifies state)
    ADialogueSystem* MutableThis = const_cast<ADialogueSystem*>(this);
    FNarr_DialogueNode* Node = MutableThis->FindNode(ActiveTreeID, ActiveNodeID);
    if (!Node || ActiveLineIndex >= Node->Lines.Num())
    {
        return FNarr_DialogueLine();
    }
    return Node->Lines[ActiveLineIndex];
}

// ============================================================
// GetCurrentChoices
// ============================================================

TArray<FString> ADialogueSystem::GetCurrentChoices() const
{
    ADialogueSystem* MutableThis = const_cast<ADialogueSystem*>(this);
    FNarr_DialogueNode* Node = MutableThis->FindNode(ActiveTreeID, ActiveNodeID);
    if (!Node) return TArray<FString>();
    return Node->ChoiceLabels;
}

// ============================================================
// RegisterDialogueTree
// ============================================================

void ADialogueSystem::RegisterDialogueTree(const FNarr_DialogueTree& Tree)
{
    // Replace if already exists
    for (int32 i = 0; i < DialogueTrees.Num(); i++)
    {
        if (DialogueTrees[i].TreeID == Tree.TreeID)
        {
            DialogueTrees[i] = Tree;
            return;
        }
    }
    DialogueTrees.Add(Tree);
}

// ============================================================
// InitialiseDefaultTrees
// ============================================================

void ADialogueSystem::InitialiseDefaultTrees()
{
    BuildElderKaelTree();
    BuildCrafterMiraTree();
    BuildScoutTaraTree();
    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Initialised %d dialogue trees"), DialogueTrees.Num());
}

// ============================================================
// FindTree / FindNode helpers
// ============================================================

FNarr_DialogueTree* ADialogueSystem::FindTree(const FString& TreeID)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID) return &Tree;
    }
    return nullptr;
}

FNarr_DialogueNode* ADialogueSystem::FindNode(const FString& TreeID, const FString& NodeID)
{
    FNarr_DialogueTree* Tree = FindTree(TreeID);
    if (!Tree) return nullptr;
    for (FNarr_DialogueNode& Node : Tree->Nodes)
    {
        if (Node.NodeID == NodeID) return &Node;
    }
    return nullptr;
}

// ============================================================
// BuildElderKaelTree — Raptor Hunt + T-Rex Survival quests
// ============================================================

void ADialogueSystem::BuildElderKaelTree()
{
    FNarr_DialogueTree Tree;
    Tree.TreeID = "Elder_Kael";
    Tree.NPCName = "Elder Kael";
    Tree.EntryNodeID = "kael_greeting";

    // Node: Greeting
    {
        FNarr_DialogueNode Node;
        Node.NodeID = "kael_greeting";
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Elder,
            "You are new to this camp. I can tell by the way you hold yourself — too upright. The predators see that.",
            5.0f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Elder,
            "Crouch when you move. Stay low. The raptors hunt by movement, not smell.",
            4.5f));
        Node.ChoiceLabels.Add("Tell me about the raptors.");
        Node.ChoiceLabels.Add("What is the greatest danger here?");
        Node.NextNodeIDs.Add("kael_raptor_info");
        Node.NextNodeIDs.Add("kael_trex_warning");
        Tree.Nodes.Add(Node);
    }

    // Node: Raptor info → activates raptor hunt quest
    {
        FNarr_DialogueNode Node;
        Node.NodeID = "kael_raptor_info";
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Elder,
            "The raptors came from the east. Three seasons ago they took my brother. Now they circle the camp every night.",
            5.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Elder,
            "Three of them. Moving fast through the eastern treeline. You need to deal with them before they grow bolder.",
            5.0f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Player,
            "I will go to the eastern ridge.",
            2.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Elder,
            "Go. Come back with proof. Then I will teach you what I know.",
            4.0f));
        Node.ActivatesQuestID = "Quest_RaptorThreat";
        Node.NextNodeIDs.Add("END");
        Tree.Nodes.Add(Node);
    }

    // Node: T-Rex warning → activates survival quest
    {
        FNarr_DialogueNode Node;
        Node.NodeID = "kael_trex_warning";
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Elder,
            "The great predator. The one with the heavy step that shakes the ground.",
            4.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Elder,
            "It came from the northern valley three nights ago. Took two hunters before we could scatter.",
            5.0f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Elder,
            "If you hear that rumble in the earth — freeze. Drop to the ground. It hunts by movement. Stay still.",
            5.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Player,
            "I understand. I will be careful.",
            2.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Elder,
            "Careful is not enough. You must survive three minutes in its territory. Only then will you know fear — and how to master it.",
            6.0f));
        Node.ActivatesQuestID = "Quest_GreatPredator";
        Node.NextNodeIDs.Add("END");
        Tree.Nodes.Add(Node);
    }

    RegisterDialogueTree(Tree);
}

// ============================================================
// BuildCrafterMiraTree — Stone Tools + Night Attack quests
// ============================================================

void ADialogueSystem::BuildCrafterMiraTree()
{
    FNarr_DialogueTree Tree;
    Tree.TreeID = "Crafter_Mira";
    Tree.NPCName = "Crafter Mira";
    Tree.EntryNodeID = "mira_greeting";

    // Node: Greeting
    {
        FNarr_DialogueNode Node;
        Node.NodeID = "mira_greeting";
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Crafter,
            "You want tools? Then you need stone. Not just any stone.",
            3.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Crafter,
            "The grey stones from the hillside crumble. Useless. You need the black flint from the river.",
            5.0f));
        Node.ChoiceLabels.Add("Where is the flint river?");
        Node.ChoiceLabels.Add("What can you make from flint?");
        Node.NextNodeIDs.Add("mira_flint_location");
        Node.NextNodeIDs.Add("mira_tools_info");
        Tree.Nodes.Add(Node);
    }

    // Node: Flint location → activates gather quest
    {
        FNarr_DialogueNode Node;
        Node.NodeID = "mira_flint_location";
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Crafter,
            "Two ridges east of camp. You will know it by the sound — water over flat stone.",
            5.0f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Crafter,
            "Take only the black stones. Five of them. Do not waste time on grey.",
            4.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Player,
            "I will bring you five black stones.",
            2.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Crafter,
            "Good. And watch the riverbank. Raptors drink there at dusk.",
            4.0f));
        Node.ActivatesQuestID = "Quest_StoneTools";
        Node.NextNodeIDs.Add("END");
        Tree.Nodes.Add(Node);
    }

    // Node: Tools info → activates night defense quest
    {
        FNarr_DialogueNode Node;
        Node.NodeID = "mira_tools_info";
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Crafter,
            "A blade that cuts raptor hide. A spear tip that can pierce the thick skin of a juvenile predator.",
            5.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Crafter,
            "But tools alone will not save this camp. Last night they came close. Very close.",
            5.0f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Player,
            "How do we defend the camp at night?",
            3.0f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Crafter,
            "Fire. Noise. And someone willing to stand at the perimeter. Five attacks repelled and they learn to fear us.",
            6.0f));
        Node.ActivatesQuestID = "Quest_NightAttack";
        Node.NextNodeIDs.Add("END");
        Tree.Nodes.Add(Node);
    }

    RegisterDialogueTree(Tree);
}

// ============================================================
// BuildScoutTaraTree — Explore + lore of the north ridge
// ============================================================

void ADialogueSystem::BuildScoutTaraTree()
{
    FNarr_DialogueTree Tree;
    Tree.TreeID = "Scout_Tara";
    Tree.NPCName = "Scout Tara";
    Tree.EntryNodeID = "tara_greeting";

    // Node: Greeting
    {
        FNarr_DialogueNode Node;
        Node.NodeID = "tara_greeting";
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Scout,
            "I have been to the north ridge. Once. I did not go back.",
            4.0f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Scout,
            "But there is water there. Clean water. And the herd animals go there to drink.",
            4.5f));
        Node.ChoiceLabels.Add("What did you see on the ridge?");
        Node.ChoiceLabels.Add("How do I get there safely?");
        Node.NextNodeIDs.Add("tara_ridge_lore");
        Node.NextNodeIDs.Add("tara_safe_route");
        Tree.Nodes.Add(Node);
    }

    // Node: Ridge lore
    {
        FNarr_DialogueNode Node;
        Node.NodeID = "tara_ridge_lore";
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Scout,
            "The long-necks. Dozens of them. Moving slow through the valley below the ridge.",
            5.0f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Scout,
            "They do not attack. But when they run — and they will run — do not be in their path.",
            5.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Scout,
            "Something spooked them while I watched. I heard the ground shake before I saw what it was.",
            5.0f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Player,
            "The great predator.",
            2.0f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Scout,
            "Yes. That is why I did not go back.",
            3.5f));
        Node.NextNodeIDs.Add("END");
        Tree.Nodes.Add(Node);
    }

    // Node: Safe route → activates explore quest
    {
        FNarr_DialogueNode Node;
        Node.NodeID = "tara_safe_route";
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Scout,
            "Stay on the high ground. The predators hunt in the valleys.",
            4.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Scout,
            "Move at midday. The heat slows them down. At dawn and dusk they are most active.",
            5.5f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Scout,
            "When you reach the ridge, look for the flat stones near the water. That is where I left a marker.",
            5.0f));
        Node.Lines.Add(FNarr_DialogueLine(ENarr_SpeakerType::Player,
            "I will find your marker.",
            2.5f));
        Node.ActivatesQuestID = "Quest_BeyondTheRidge";
        Node.NextNodeIDs.Add("END");
        Tree.Nodes.Add(Node);
    }

    RegisterDialogueTree(Tree);
}
