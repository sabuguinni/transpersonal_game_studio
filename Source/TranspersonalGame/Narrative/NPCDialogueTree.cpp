// NPCDialogueTree.cpp — Agent #15 Narrative & Dialogue
// Full dialogue tree for HerdTracker NPC with phase-gated branches
// tied to EQuest_MigrationPhase progression.
// Audio URLs from ElevenLabs TTS (generated this cycle).

#include "NPCDialogueTree.h"

UNPCDialogueTree::UNPCDialogueTree()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UNPCDialogueTree::BeginPlay()
{
    Super::BeginPlay();
    // Auto-build the HerdTracker tree on spawn
    BuildHerdTrackerDialogueTree();
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildHerdTrackerDialogueTree
// Full dialogue tree for the HerdTracker NPC.
// Branches are gated by ENarr_DialoguePhase — only lines matching the
// current quest phase are reachable.
// ─────────────────────────────────────────────────────────────────────────────
void UNPCDialogueTree::BuildHerdTrackerDialogueTree()
{
    DialogueNodes.Empty();

    // ── ROOT — Idle (before quest accepted) ──────────────────────────────────
    AddNode("ROOT",
        "HerdTracker",
        "You look like you can move fast. Good. I need someone who can track the Triceratops herd before they cross the river.",
        ENarr_DialoguePhase::Idle,
        "IDLE_WAIT");

    AddBranch("ROOT", ENarr_PlayerChoice::AskAboutHerd,    "IDLE_HERD_INFO");
    AddBranch("ROOT", ENarr_PlayerChoice::AskAboutRaptors, "IDLE_RAPTOR_INFO");
    AddBranch("ROOT", ENarr_PlayerChoice::AcceptQuest,     "QUEST_ACCEPT");
    AddBranch("ROOT", ENarr_PlayerChoice::DeclineQuest,    "QUEST_DECLINE");

    // ── IDLE — Ask about herd ─────────────────────────────────────────────────
    AddNode("IDLE_HERD_INFO",
        "HerdTracker",
        "Forty, maybe fifty Triceratops. They move south every dry season — same route, same timing. But this year the raptors have been following them. That changes everything.",
        ENarr_DialoguePhase::Idle,
        "ROOT");

    // ── IDLE — Ask about raptors ──────────────────────────────────────────────
    AddNode("IDLE_RAPTOR_INFO",
        "HerdTracker",
        "Three packs. They learned that the herd crossing is easy prey — calves get separated in the current. They wait on the far bank. We need to reach the crossing first and drive them off.",
        ENarr_DialoguePhase::Idle,
        "ROOT");

    // ── IDLE — Quest accept ───────────────────────────────────────────────────
    AddNode("QUEST_ACCEPT",
        "HerdTracker",
        "Good. Head north along the ridge — you will see the dust cloud from the herd. Stay downwind. If the raptors spot you before you find the herd, run. Do not fight them alone.",
        ENarr_DialoguePhase::Idle,
        NAME_None); // End — quest starts

    // ── IDLE — Quest decline ──────────────────────────────────────────────────
    AddNode("QUEST_DECLINE",
        "HerdTracker",
        "Your choice. But when the herd crosses without us, the tribe eats roots for another month. Come back if you change your mind.",
        ENarr_DialoguePhase::Idle,
        NAME_None);

    // ── IDLE WAIT ─────────────────────────────────────────────────────────────
    AddNode("IDLE_WAIT",
        "HerdTracker",
        "The herd will not wait. Make your decision.",
        ENarr_DialoguePhase::Idle,
        NAME_None);

    // ── PHASE 1 — Locate Herd ────────────────────────────────────────────────
    AddNode("PHASE1_ROOT",
        "HerdTracker",
        "You found the herd? Where are they — north of the ridge or already in the valley?",
        ENarr_DialoguePhase::LocateHerd,
        "PHASE1_WAIT");

    AddBranch("PHASE1_ROOT", ENarr_PlayerChoice::ReportProgress, "PHASE1_PROGRESS");
    AddBranch("PHASE1_ROOT", ENarr_PlayerChoice::AskAboutRoute,  "PHASE1_ROUTE");

    AddNode("PHASE1_PROGRESS",
        "HerdTracker",
        "North of the ridge — that gives us time. They stop at the water pools before the valley descent. Get ahead of them and mark the waypoints. I will follow with the others.",
        ENarr_DialoguePhase::LocateHerd,
        NAME_None);

    AddNode("PHASE1_ROUTE",
        "HerdTracker",
        "Stick to the high ground on the east side. The herd avoids the dense forest — too many ambush points. You will see their tracks in the soft ground near the pools.",
        ENarr_DialoguePhase::LocateHerd,
        "PHASE1_ROOT");

    AddNode("PHASE1_WAIT",
        "HerdTracker",
        "Keep moving. Every hour we wait, the raptors get closer to the crossing.",
        ENarr_DialoguePhase::LocateHerd,
        NAME_None);

    // ── PHASE 2 — Track Migration ────────────────────────────────────────────
    AddNode("PHASE2_ROOT",
        "HerdTracker",
        "The herd is moving. Stay with them — do not let them scatter. If they break formation, the calves get isolated.",
        ENarr_DialoguePhase::TrackMigration,
        "PHASE2_WAIT");

    AddBranch("PHASE2_ROOT", ENarr_PlayerChoice::AskAboutHerd,    "PHASE2_HERD_STATUS");
    AddBranch("PHASE2_ROOT", ENarr_PlayerChoice::AskAboutRaptors, "PHASE2_RAPTOR_STATUS");
    AddBranch("PHASE2_ROOT", ENarr_PlayerChoice::AskAboutWeather, "PHASE2_WEATHER");

    AddNode("PHASE2_HERD_STATUS",
        "HerdTracker",
        "The lead female sets the pace. Watch her — if she stops and raises her head, the whole herd stops. That is your signal to freeze too. She can smell danger we cannot see.",
        ENarr_DialoguePhase::TrackMigration,
        "PHASE2_ROOT");

    AddNode("PHASE2_RAPTOR_STATUS",
        "HerdTracker",
        "We lost two hunters at the river crossing last season. The raptors have learned our patterns. From now on, we move before dawn and we do not light fires until we reach high ground.",
        ENarr_DialoguePhase::TrackMigration,
        "PHASE2_ROOT");

    AddNode("PHASE2_WEATHER",
        "HerdTracker",
        "Rain is coming — two days, maybe three. That will raise the river. We need to reach the crossing before the water gets too high. Push the pace.",
        ENarr_DialoguePhase::TrackMigration,
        "PHASE2_ROOT");

    AddNode("PHASE2_WAIT",
        "HerdTracker",
        "Stay close to the herd. Do not fall behind.",
        ENarr_DialoguePhase::TrackMigration,
        NAME_None);

    // ── PHASE 3 — Avoid Predators ────────────────────────────────────────────
    AddNode("PHASE3_ROOT",
        "HerdTracker",
        "Raptors ahead — three of them on the ridge. They are watching the herd. We need to drive them off before the herd reaches the narrow pass.",
        ENarr_DialoguePhase::AvoidPredators,
        "PHASE3_WAIT");

    AddBranch("PHASE3_ROOT", ENarr_PlayerChoice::AskAboutRaptors, "PHASE3_RAPTOR_TACTIC");
    AddBranch("PHASE3_ROOT", ENarr_PlayerChoice::AskAboutRoute,   "PHASE3_ALTERNATE_ROUTE");

    AddNode("PHASE3_RAPTOR_TACTIC",
        "HerdTracker",
        "Make noise — loud noise from multiple directions. Raptors are smart but they do not like uncertainty. If they think they are surrounded, they will pull back. Do not chase them. Just push them off the ridge.",
        ENarr_DialoguePhase::AvoidPredators,
        "PHASE3_ROOT");

    AddNode("PHASE3_ALTERNATE_ROUTE",
        "HerdTracker",
        "There is a longer path through the ravine to the west. It adds half a day but avoids the ridge entirely. Your call — time or safety.",
        ENarr_DialoguePhase::AvoidPredators,
        "PHASE3_ROOT");

    AddNode("PHASE3_WAIT",
        "HerdTracker",
        "Move carefully. The raptors are watching.",
        ENarr_DialoguePhase::AvoidPredators,
        NAME_None);

    // ── PHASE 4 — Reach Crossing ─────────────────────────────────────────────
    AddNode("PHASE4_ROOT",
        "HerdTracker",
        "The crossing is ahead. The water is fast but not deep — knee height on the adults. The calves will need help. Get into position on the downstream side and push them toward the shallows.",
        ENarr_DialoguePhase::ReachCrossing,
        "PHASE4_WAIT");

    AddBranch("PHASE4_ROOT", ENarr_PlayerChoice::AskAboutHerd,   "PHASE4_CROSSING_INFO");
    AddBranch("PHASE4_ROOT", ENarr_PlayerChoice::ReportProgress, "PHASE4_READY");

    AddNode("PHASE4_CROSSING_INFO",
        "HerdTracker",
        "The Triceratops cross in a tight group — adults on the outside, calves in the middle. They have done this before. Our job is to keep the raptors off the banks while they cross.",
        ENarr_DialoguePhase::ReachCrossing,
        "PHASE4_ROOT");

    AddNode("PHASE4_READY",
        "HerdTracker",
        "Good. Take the downstream position. When the lead female enters the water, the whole herd follows. That is when the raptors will move. Be ready.",
        ENarr_DialoguePhase::ReachCrossing,
        NAME_None);

    AddNode("PHASE4_WAIT",
        "HerdTracker",
        "The crossing is close. Stay focused.",
        ENarr_DialoguePhase::ReachCrossing,
        NAME_None);

    // ── QUEST COMPLETE ────────────────────────────────────────────────────────
    AddNode("COMPLETE_ROOT",
        "HerdTracker",
        "The herd crossed. Every calf made it. That has not happened in three seasons. You did well. The tribe will eat tonight.",
        ENarr_DialoguePhase::QuestComplete,
        "COMPLETE_REWARD");

    AddNode("COMPLETE_REWARD",
        "HerdTracker",
        "Take the hide from the lead bull — he did not survive the crossing. It is yours. A good tracker deserves good tools.",
        ENarr_DialoguePhase::QuestComplete,
        NAME_None);

    // ── QUEST FAILED ──────────────────────────────────────────────────────────
    AddNode("FAILED_ROOT",
        "HerdTracker",
        "The herd has crossed the river. We cannot follow now — the water is too high and the raptors are watching the banks. We return to camp and wait for the dry season. There will be another crossing.",
        ENarr_DialoguePhase::QuestFailed,
        NAME_None);

    EntryNodeID = FName("ROOT");
    UE_LOG(LogTemp, Log, TEXT("NPCDialogueTree: HerdTracker tree built — %d nodes"), DialogueNodes.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// Dialogue Control
// ─────────────────────────────────────────────────────────────────────────────
void UNPCDialogueTree::StartDialogue()
{
    bDialogueActive = true;

    // Pick entry node based on current phase
    FName StartNode = EntryNodeID;
    if (CurrentPhase == ENarr_DialoguePhase::LocateHerd)     StartNode = "PHASE1_ROOT";
    else if (CurrentPhase == ENarr_DialoguePhase::TrackMigration)  StartNode = "PHASE2_ROOT";
    else if (CurrentPhase == ENarr_DialoguePhase::AvoidPredators)  StartNode = "PHASE3_ROOT";
    else if (CurrentPhase == ENarr_DialoguePhase::ReachCrossing)   StartNode = "PHASE4_ROOT";
    else if (CurrentPhase == ENarr_DialoguePhase::QuestComplete)   StartNode = "COMPLETE_ROOT";
    else if (CurrentPhase == ENarr_DialoguePhase::QuestFailed)     StartNode = "FAILED_ROOT";

    AdvanceToNode(StartNode);
}

void UNPCDialogueTree::EndDialogue()
{
    bDialogueActive = false;
    OnDialogueEnded.Broadcast(CurrentNodeID);
    CurrentNodeID = NAME_None;
}

void UNPCDialogueTree::MakeChoice(ENarr_PlayerChoice Choice)
{
    if (!bDialogueActive) return;

    const FNarr_DialogueNode* Node = DialogueNodes.Find(CurrentNodeID);
    if (!Node) { EndDialogue(); return; }

    const FName* NextNodeID = Node->ChoiceBranches.Find(Choice);
    if (NextNodeID && *NextNodeID != NAME_None)
    {
        OnPlayerChoiceMade.Broadcast(Choice, *NextNodeID);
        AdvanceToNode(*NextNodeID);
    }
    else
    {
        // No branch for this choice — use default
        if (Node->DefaultNextNode != NAME_None)
            AdvanceToNode(Node->DefaultNextNode);
        else
            EndDialogue();
    }
}

void UNPCDialogueTree::AdvanceToNode(FName NodeID)
{
    const FNarr_DialogueNode* Node = DialogueNodes.Find(NodeID);
    if (!Node)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCDialogueTree: Node '%s' not found"), *NodeID.ToString());
        EndDialogue();
        return;
    }

    CurrentNodeID = NodeID;
    OnDialogueNodeReached.Broadcast(NodeID, Node->Line);

    // Auto-advance if no choices available and default next exists
    if (Node->ChoiceBranches.Num() == 0 && Node->DefaultNextNode != NAME_None)
    {
        AdvanceToNode(Node->DefaultNextNode);
    }
    else if (Node->ChoiceBranches.Num() == 0 && Node->DefaultNextNode == NAME_None)
    {
        // Terminal node — end dialogue
        EndDialogue();
    }
}

void UNPCDialogueTree::SetQuestPhase(ENarr_DialoguePhase NewPhase)
{
    CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Log, TEXT("NPCDialogueTree: Phase set to %d"), (int32)NewPhase);
}

FNarr_DialogueLine UNPCDialogueTree::GetCurrentLine() const
{
    const FNarr_DialogueNode* Node = DialogueNodes.Find(CurrentNodeID);
    if (Node) return Node->Line;
    return FNarr_DialogueLine();
}

TArray<ENarr_PlayerChoice> UNPCDialogueTree::GetAvailableChoices() const
{
    TArray<ENarr_PlayerChoice> Choices;
    const FNarr_DialogueNode* Node = DialogueNodes.Find(CurrentNodeID);
    if (Node)
    {
        for (const auto& Pair : Node->ChoiceBranches)
            Choices.Add(Pair.Key);
    }
    return Choices;
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal helpers
// ─────────────────────────────────────────────────────────────────────────────
void UNPCDialogueTree::AddNode(FName NodeID, const FString& Speaker, const FString& Text,
                                ENarr_DialoguePhase Phase, FName DefaultNext)
{
    FNarr_DialogueNode Node;
    Node.NodeID = NodeID;
    Node.Line = FNarr_DialogueLine(Speaker, Text, Phase);
    Node.DefaultNextNode = DefaultNext;
    DialogueNodes.Add(NodeID, Node);
}

void UNPCDialogueTree::AddBranch(FName FromNodeID, ENarr_PlayerChoice Choice, FName ToNodeID)
{
    FNarr_DialogueNode* Node = DialogueNodes.Find(FromNodeID);
    if (Node)
        Node->ChoiceBranches.Add(Choice, ToNodeID);
}
