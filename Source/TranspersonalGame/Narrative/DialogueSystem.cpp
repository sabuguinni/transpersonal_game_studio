#include "DialogueSystem.h"
#include "Engine/World.h"

// ─── Audio URLs from TTS generation (PROD_CYCLE_AUTO_20260701_004) ────────────
// Elder_Crafted:  https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782884624367_QuestNPC_Elder.mp3
// Tracker_Raptor: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782884626228_QuestNPC_Tracker.mp3
// Tracker_Mamm:   https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782884633594_QuestNPC_Tracker_Hunt.mp3
// Elder_Return:   https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782884635451_QuestNPC_Elder_Return.mp3

UDialogueSystem::UDialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentState = ENarr_DialogueState::Idle;
    ActiveTreeID = NAME_None;
    ActiveNodeID = NAME_None;

    // Bake TTS audio URLs into CDO
    ElderCraftedURL  = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782884624367_QuestNPC_Elder.mp3");
    TrackerRaptorURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782884626228_QuestNPC_Tracker.mp3");
    TrackerMammothURL= TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782884633594_QuestNPC_Tracker_Hunt.mp3");
    ElderReturnURL   = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782884635451_QuestNPC_Elder_Return.mp3");
}

void UDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    BuildDefaultTrees();
}

// ─── Build default dialogue trees for Elder + Tracker ─────────────────────────

void UDialogueSystem::BuildDefaultTrees()
{
    // ── Tree 1: Elder — First Tools arc ──────────────────────────────────────
    {
        FNarr_DialogueTree ElderTree;
        ElderTree.TreeID      = FName("Elder_FirstTools");
        ElderTree.OwnerRole   = ENarr_SpeakerRole::Elder;
        ElderTree.RootNodeID  = FName("Elder_Node_01");
        ElderTree.RequiredArc = ENarr_QuestArc::None;

        // Node 01 — opening line (fires after Stone Axe crafted)
        FNarr_DialogueNode Node01;
        Node01.NodeID = FName("Elder_Node_01");
        Node01.Line.LineID        = FName("Elder_Crafted_01");
        Node01.Line.Speaker       = ENarr_SpeakerRole::Elder;
        Node01.Line.LineText      = FText::FromString(TEXT("Stone Axe crafted. You are no longer just a survivor — you are a maker. The tribe remembers those who build. Now unlock the Bone Spear and hunt what hunts you."));
        Node01.Line.AudioURL      = ElderCraftedURL;
        Node01.Line.DisplayDuration = 11.0f;
        Node01.Line.TriggerArc    = ENarr_QuestArc::FirstTools;
        Node01.bIsEndNode         = false;

        // Choice A — accept raptor nest quest
        FNarr_DialogueChoice ChoiceA;
        ChoiceA.ChoiceText   = FText::FromString(TEXT("Tell me about the raptor nest."));
        ChoiceA.NextNodeID   = FName("Elder_Node_02");
        ChoiceA.bUnlocksQuest = true;
        ChoiceA.UnlockedArc  = ENarr_QuestArc::RaptorNest;
        Node01.Choices.Add(ChoiceA);

        // Choice B — ask about mammoth hunt
        FNarr_DialogueChoice ChoiceB;
        ChoiceB.ChoiceText   = FText::FromString(TEXT("What about the mammoth herd?"));
        ChoiceB.NextNodeID   = FName("Elder_Node_03");
        ChoiceB.bUnlocksQuest = false;
        ChoiceB.UnlockedArc  = ENarr_QuestArc::None;
        Node01.Choices.Add(ChoiceB);

        ElderTree.Nodes.Add(Node01);

        // Node 02 — raptor nest briefing
        FNarr_DialogueNode Node02;
        Node02.NodeID = FName("Elder_Node_02");
        Node02.Line.LineID        = FName("Elder_Raptor_01");
        Node02.Line.Speaker       = ENarr_SpeakerRole::Elder;
        Node02.Line.LineText      = FText::FromString(TEXT("The nest is two ridges east. Three eggs — maybe four. Bring them back. The tribe needs the protein before the cold season."));
        Node02.Line.AudioURL      = TEXT("");
        Node02.Line.DisplayDuration = 8.0f;
        Node02.Line.TriggerArc    = ENarr_QuestArc::RaptorNest;
        Node02.bIsEndNode         = true;
        ElderTree.Nodes.Add(Node02);

        // Node 03 — mammoth briefing
        FNarr_DialogueNode Node03;
        Node03.NodeID = FName("Elder_Node_03");
        Node03.Line.LineID        = FName("Elder_Mammoth_01");
        Node03.Line.Speaker       = ENarr_SpeakerRole::Elder;
        Node03.Line.LineText      = FText::FromString(TEXT("You came back. Not everyone does. The tribe needs more than survivors — it needs hunters who remember the land. Tell me what you saw out there."));
        Node03.Line.AudioURL      = ElderReturnURL;
        Node03.Line.DisplayDuration = 10.0f;
        Node03.Line.TriggerArc    = ENarr_QuestArc::MammothHunt;
        Node03.bIsEndNode         = true;
        ElderTree.Nodes.Add(Node03);

        DialogueTrees.Add(ElderTree);
    }

    // ── Tree 2: Tracker — Hunt briefings ─────────────────────────────────────
    {
        FNarr_DialogueTree TrackerTree;
        TrackerTree.TreeID      = FName("Tracker_Hunt");
        TrackerTree.OwnerRole   = ENarr_SpeakerRole::Tracker;
        TrackerTree.RootNodeID  = FName("Tracker_Node_01");
        TrackerTree.RequiredArc = ENarr_QuestArc::FirstTools;

        // Node 01 — raptor nest raid briefing
        FNarr_DialogueNode TNode01;
        TNode01.NodeID = FName("Tracker_Node_01");
        TNode01.Line.LineID        = FName("Tracker_Raptor_01");
        TNode01.Line.Speaker       = ENarr_SpeakerRole::Tracker;
        TNode01.Line.LineText      = FText::FromString(TEXT("First tools are done. Now the real test begins. The raptors nest two ridges east — three of them, maybe four. We go at dawn, before they spread out. Bring the spear."));
        TNode01.Line.AudioURL      = TrackerRaptorURL;
        TNode01.Line.DisplayDuration = 11.0f;
        TNode01.Line.TriggerArc    = ENarr_QuestArc::RaptorNest;
        TNode01.bIsEndNode         = false;

        // Choice — track mammoth instead
        FNarr_DialogueChoice TChoice;
        TChoice.ChoiceText   = FText::FromString(TEXT("What about the mammoth herd moving south?"));
        TChoice.NextNodeID   = FName("Tracker_Node_02");
        TChoice.bUnlocksQuest = true;
        TChoice.UnlockedArc  = ENarr_QuestArc::MammothHunt;
        TNode01.Choices.Add(TChoice);

        TrackerTree.Nodes.Add(TNode01);

        // Node 02 — mammoth tracking briefing
        FNarr_DialogueNode TNode02;
        TNode02.NodeID = FName("Tracker_Node_02");
        TNode02.Line.LineID        = FName("Tracker_Mammoth_01");
        TNode02.Line.Speaker       = ENarr_SpeakerRole::Tracker;
        TNode02.Line.LineText      = FText::FromString(TEXT("The mammoth does not fear you yet. But it will. Track it south, past the river bend. Watch where it drinks. That is where you set the ambush. Patience is the first weapon."));
        TNode02.Line.AudioURL      = TrackerMammothURL;
        TNode02.Line.DisplayDuration = 12.0f;
        TNode02.Line.TriggerArc    = ENarr_QuestArc::MammothHunt;
        TNode02.bIsEndNode         = true;
        TrackerTree.Nodes.Add(TNode02);

        DialogueTrees.Add(TrackerTree);
    }
}

// ─── Core Dialogue API ────────────────────────────────────────────────────────

bool UDialogueSystem::StartDialogue(FName TreeID)
{
    if (CurrentState != ENarr_DialogueState::Idle)
    {
        return false;
    }

    FNarr_DialogueTree* Tree = FindTree(TreeID);
    if (!Tree)
    {
        return false;
    }

    // Check arc requirement
    if (Tree->RequiredArc != ENarr_QuestArc::None && !HasUnlockedArc(Tree->RequiredArc))
    {
        return false;
    }

    ActiveTreeID = TreeID;
    ActiveNodeID = Tree->RootNodeID;
    CurrentState = ENarr_DialogueState::Speaking;

    OnDialogueStarted.Broadcast(TreeID);

    FNarr_DialogueNode* RootNode = FindNode(Tree, Tree->RootNodeID);
    if (RootNode)
    {
        ShowNode(RootNode);
    }

    return true;
}

void UDialogueSystem::SelectChoice(int32 ChoiceIndex)
{
    if (CurrentState != ENarr_DialogueState::Waiting)
    {
        return;
    }

    FNarr_DialogueTree* Tree = FindTree(ActiveTreeID);
    if (!Tree) { return; }

    FNarr_DialogueNode* CurrentNode = FindNode(Tree, ActiveNodeID);
    if (!CurrentNode) { return; }

    if (!CurrentNode->Choices.IsValidIndex(ChoiceIndex))
    {
        return;
    }

    const FNarr_DialogueChoice& Choice = CurrentNode->Choices[ChoiceIndex];

    // Unlock arc if this choice grants one
    if (Choice.bUnlocksQuest && Choice.UnlockedArc != ENarr_QuestArc::None)
    {
        UnlockArc(Choice.UnlockedArc);
    }

    if (Choice.NextNodeID == NAME_None)
    {
        EndDialogue();
        return;
    }

    ActiveNodeID = Choice.NextNodeID;
    FNarr_DialogueNode* NextNode = FindNode(Tree, Choice.NextNodeID);
    if (NextNode)
    {
        CurrentState = ENarr_DialogueState::Speaking;
        ShowNode(NextNode);
    }
    else
    {
        EndDialogue();
    }
}

void UDialogueSystem::EndDialogue()
{
    ActiveTreeID = NAME_None;
    ActiveNodeID = NAME_None;
    CurrentState = ENarr_DialogueState::Idle;
    OnDialogueEnded.Broadcast();
}

bool UDialogueSystem::IsDialogueActive() const
{
    return CurrentState != ENarr_DialogueState::Idle;
}

FNarr_DialogueNode UDialogueSystem::GetCurrentNode() const
{
    FNarr_DialogueTree* Tree = const_cast<UDialogueSystem*>(this)->FindTree(ActiveTreeID);
    if (!Tree) { return FNarr_DialogueNode(); }

    FNarr_DialogueNode* Node = const_cast<UDialogueSystem*>(this)->FindNode(Tree, ActiveNodeID);
    if (!Node) { return FNarr_DialogueNode(); }

    return *Node;
}

bool UDialogueSystem::HasUnlockedArc(ENarr_QuestArc Arc) const
{
    return UnlockedArcs.Contains(Arc);
}

void UDialogueSystem::UnlockArc(ENarr_QuestArc Arc)
{
    if (!UnlockedArcs.Contains(Arc))
    {
        UnlockedArcs.Add(Arc);
        OnQuestArcUnlocked.Broadcast(Arc);
    }
}

void UDialogueSystem::RegisterDialogueTree(const FNarr_DialogueTree& Tree)
{
    // Remove existing tree with same ID if present
    DialogueTrees.RemoveAll([&Tree](const FNarr_DialogueTree& Existing)
    {
        return Existing.TreeID == Tree.TreeID;
    });
    DialogueTrees.Add(Tree);
}

TArray<FNarr_DialogueChoice> UDialogueSystem::GetCurrentChoices() const
{
    FNarr_DialogueTree* Tree = const_cast<UDialogueSystem*>(this)->FindTree(ActiveTreeID);
    if (!Tree) { return TArray<FNarr_DialogueChoice>(); }

    FNarr_DialogueNode* Node = const_cast<UDialogueSystem*>(this)->FindNode(Tree, ActiveNodeID);
    if (!Node) { return TArray<FNarr_DialogueChoice>(); }

    return Node->Choices;
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

FNarr_DialogueTree* UDialogueSystem::FindTree(FName TreeID)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID)
        {
            return &Tree;
        }
    }
    return nullptr;
}

FNarr_DialogueNode* UDialogueSystem::FindNode(FNarr_DialogueTree* Tree, FName NodeID)
{
    if (!Tree) { return nullptr; }
    for (FNarr_DialogueNode& Node : Tree->Nodes)
    {
        if (Node.NodeID == NodeID)
        {
            return &Node;
        }
    }
    return nullptr;
}

void UDialogueSystem::ShowNode(FNarr_DialogueNode* Node)
{
    if (!Node) { return; }

    OnDialogueLineShown.Broadcast(Node->Line);

    if (Node->bIsEndNode || Node->Choices.Num() == 0)
    {
        CurrentState = ENarr_DialogueState::Finished;
        EndDialogue();
    }
    else
    {
        CurrentState = ENarr_DialogueState::Waiting;
    }
}
