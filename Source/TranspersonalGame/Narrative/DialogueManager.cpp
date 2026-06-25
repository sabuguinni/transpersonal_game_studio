#include "DialogueManager.h"
#include "Engine/World.h"

// ============================================================
// Constructor
// ============================================================

ADialogueManager::ADialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentState = ENarr_DialogueState::Idle;
    ActiveLineIndex = 0;
}

// ============================================================
// BeginPlay — register all built-in NPC dialogue trees
// ============================================================

void ADialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitialiseBuiltInDialogueTrees();
}

// ============================================================
// InitialiseBuiltInDialogueTrees
// ============================================================

void ADialogueManager::InitialiseBuiltInDialogueTrees()
{
    BuildElderKaelTree();
    BuildScoutMiraTree();
}

// ============================================================
// Elder Kael — Tribe Elder NPC
// Quest links: QUEST_EXPLORE_RIVER, QUEST_DEFEND_CAMP
// Audio: ElevenLabs TTS generated in PROD_CYCLE_AUTO_20260625_009
// ============================================================

void ADialogueManager::BuildElderKaelTree()
{
    FNarr_DialogueTree Tree;
    Tree.TreeID = TEXT("DIALOGUE_ELDER_KAEL");
    Tree.NPCID  = TEXT("TribeElder_QuestGiver");
    Tree.RootNodeID = TEXT("KAEL_GREETING");
    Tree.LinkedQuestID = TEXT("QUEST_EXPLORE_RIVER");

    // ---- Node: KAEL_GREETING ----
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("KAEL_GREETING");

        FNarr_DialogueLine L1;
        L1.SpeakerID = TEXT("Elder Kael");
        L1.SpeakerRole = ENarr_SpeakerRole::TribeElder;
        L1.LineText = TEXT("You survived the night. Good. Most do not, their first time alone in the dark.");
        L1.DisplayDuration = 4.5f;
        L1.AnimationHint = TEXT("nod_slow");
        Node.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.SpeakerID = TEXT("Elder Kael");
        L2.SpeakerRole = ENarr_SpeakerRole::TribeElder;
        L2.LineText = TEXT("The T-Rex does not hunt by sight alone. It hunts by sound. By smell. By the trembling of the earth beneath your feet. Stand still. Breathe slow.");
        L2.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782393319087_Elder_Kael_Lore.mp3");
        L2.DisplayDuration = 12.0f;
        L2.AnimationHint = TEXT("gesture_east");
        Node.Lines.Add(L2);

        FNarr_DialogueChoice C1;
        C1.ChoiceText = TEXT("Tell me about the river valley.");
        C1.NextNodeID = TEXT("KAEL_RIVER_QUEST");
        Node.Choices.Add(C1);

        FNarr_DialogueChoice C2;
        C2.ChoiceText = TEXT("What happened to the tribe that was here before?");
        C2.NextNodeID = TEXT("KAEL_LORE_VALLEY");
        Node.Choices.Add(C2);

        FNarr_DialogueChoice C3;
        C3.ChoiceText = TEXT("I need to go.");
        C3.NextNodeID = TEXT("KAEL_FAREWELL");
        Node.Choices.Add(C3);

        Tree.Nodes.Add(Node);
    }

    // ---- Node: KAEL_RIVER_QUEST ----
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("KAEL_RIVER_QUEST");

        FNarr_DialogueLine L1;
        L1.SpeakerID = TEXT("Elder Kael");
        L1.SpeakerRole = ENarr_SpeakerRole::TribeElder;
        L1.LineText = TEXT("The eastern hunting grounds are dangerous. A Tyrannosaurus has claimed the river valley as its territory. If you go, go at dusk — it feeds at dawn.");
        L1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782393159346_TribeElder_QuestGiver.mp3");
        L1.DisplayDuration = 10.0f;
        L1.AnimationHint = TEXT("point_east");
        Node.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.SpeakerID = TEXT("Elder Kael");
        L2.SpeakerRole = ENarr_SpeakerRole::TribeElder;
        L2.LineText = TEXT("You want to cross the river? Then you need to know when the Triceratops herd drinks. They come at dusk, from the north. While they drink, the T-Rex stays west. That is your window. Maybe ten minutes. Do not waste it.");
        L2.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782393343233_TribeElder_QuestGiver_Dialogue.mp3");
        L2.DisplayDuration = 15.0f;
        L2.AnimationHint = TEXT("crouch_point");
        Node.Lines.Add(L2);

        FNarr_DialogueChoice C1;
        C1.ChoiceText = TEXT("I will scout the valley.");
        C1.NextNodeID = TEXT("KAEL_QUEST_ACCEPT");
        C1.bUnlocksQuest = true;
        C1.UnlockedQuestID = TEXT("QUEST_EXPLORE_RIVER");
        Node.Choices.Add(C1);

        FNarr_DialogueChoice C2;
        C2.ChoiceText = TEXT("That sounds too dangerous.");
        C2.NextNodeID = TEXT("KAEL_QUEST_DECLINE");
        Node.Choices.Add(C2);

        Tree.Nodes.Add(Node);
    }

    // ---- Node: KAEL_LORE_VALLEY ----
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("KAEL_LORE_VALLEY");

        FNarr_DialogueLine L1;
        L1.SpeakerID = TEXT("Narrator");
        L1.SpeakerRole = ENarr_SpeakerRole::Narrator;
        L1.LineText = TEXT("We call this place the Valley of Teeth. Not because of the rocks. The first tribe that camped here — forty strong — was gone by morning. Only the fire remained. We do not camp here.");
        L1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782393328297_Narrator_GameBible.mp3");
        L1.DisplayDuration = 13.0f;
        L1.AnimationHint = TEXT("look_around_fearful");
        Node.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.SpeakerID = TEXT("Elder Kael");
        L2.SpeakerRole = ENarr_SpeakerRole::TribeElder;
        L2.LineText = TEXT("My father's father saw the great lizards bring down a mammoth at the river bend. Three of them. Working together. They are not animals. They think. They plan. Never forget that.");
        L2.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782393326193_Scout_Mira_Lore.mp3");
        L2.DisplayDuration = 12.0f;
        L2.AnimationHint = TEXT("gesture_wide");
        Node.Lines.Add(L2);

        Node.AutoNextNodeID = TEXT("KAEL_GREETING");
        Tree.Nodes.Add(Node);
    }

    // ---- Node: KAEL_QUEST_ACCEPT ----
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("KAEL_QUEST_ACCEPT");

        FNarr_DialogueLine L1;
        L1.SpeakerID = TEXT("Elder Kael");
        L1.SpeakerRole = ENarr_SpeakerRole::TribeElder;
        L1.LineText = TEXT("Then go. And come back alive. The tribe needs scouts more than it needs heroes.");
        L1.DisplayDuration = 5.0f;
        L1.AnimationHint = TEXT("nod_firm");
        Node.Lines.Add(L1);

        Node.bIsEndNode = true;
        Tree.Nodes.Add(Node);
    }

    // ---- Node: KAEL_QUEST_DECLINE ----
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("KAEL_QUEST_DECLINE");

        FNarr_DialogueLine L1;
        L1.SpeakerID = TEXT("Elder Kael");
        L1.SpeakerRole = ENarr_SpeakerRole::TribeElder;
        L1.LineText = TEXT("Caution is not cowardice. Come back when you are ready.");
        L1.DisplayDuration = 4.0f;
        L1.AnimationHint = TEXT("nod_slow");
        Node.Lines.Add(L1);

        Node.bIsEndNode = true;
        Tree.Nodes.Add(Node);
    }

    // ---- Node: KAEL_FAREWELL ----
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("KAEL_FAREWELL");

        FNarr_DialogueLine L1;
        L1.SpeakerID = TEXT("Elder Kael");
        L1.SpeakerRole = ENarr_SpeakerRole::TribeElder;
        L1.LineText = TEXT("Stay close to the tree line. And watch the sky — when the birds go silent, something big is moving.");
        L1.DisplayDuration = 5.0f;
        L1.AnimationHint = TEXT("look_up");
        Node.Lines.Add(L1);

        Node.bIsEndNode = true;
        Tree.Nodes.Add(Node);
    }

    DialogueTrees.Add(Tree);
}

// ============================================================
// Scout Mira — Scout NPC
// Quest links: QUEST_HUNT_RAPTOR_01, QUEST_CRAFT_STONE_AXE
// ============================================================

void ADialogueManager::BuildScoutMiraTree()
{
    FNarr_DialogueTree Tree;
    Tree.TreeID = TEXT("DIALOGUE_SCOUT_MIRA");
    Tree.NPCID  = TEXT("Scout_NPC_QuestLine");
    Tree.RootNodeID = TEXT("MIRA_GREETING");
    Tree.LinkedQuestID = TEXT("QUEST_HUNT_RAPTOR_01");

    // ---- Node: MIRA_GREETING ----
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("MIRA_GREETING");

        FNarr_DialogueLine L1;
        L1.SpeakerID = TEXT("Scout Mira");
        L1.SpeakerRole = ENarr_SpeakerRole::Scout;
        L1.LineText = TEXT("You found the raptor nest. Three eggs remain. Take one — it will feed your tribe for days. But be swift. The mother is never far.");
        L1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782393175487_Scout_NPC_QuestLine.mp3");
        L1.DisplayDuration = 10.0f;
        L1.AnimationHint = TEXT("crouch_whisper");
        Node.Lines.Add(L1);

        FNarr_DialogueChoice C1;
        C1.ChoiceText = TEXT("Tell me about the raptors.");
        C1.NextNodeID = TEXT("MIRA_RAPTOR_LORE");
        Node.Choices.Add(C1);

        FNarr_DialogueChoice C2;
        C2.ChoiceText = TEXT("I need a stone axe. Where do I find the right rocks?");
        C2.NextNodeID = TEXT("MIRA_CRAFTING_QUEST");
        Node.Choices.Add(C2);

        FNarr_DialogueChoice C3;
        C3.ChoiceText = TEXT("I will hunt a raptor alone.");
        C3.NextNodeID = TEXT("MIRA_HUNT_QUEST");
        C3.bUnlocksQuest = true;
        C3.UnlockedQuestID = TEXT("QUEST_HUNT_RAPTOR_01");
        Node.Choices.Add(C3);

        Tree.Nodes.Add(Node);
    }

    // ---- Node: MIRA_RAPTOR_LORE ----
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("MIRA_RAPTOR_LORE");

        FNarr_DialogueLine L1;
        L1.SpeakerID = TEXT("Scout Mira");
        L1.SpeakerRole = ENarr_SpeakerRole::Scout;
        L1.LineText = TEXT("Velociraptors hunt in threes. One drives you toward the others. If you see one, assume two more are already flanking you. Stop moving. Back against a tree. Make yourself unpredictable.");
        L1.DisplayDuration = 11.0f;
        L1.AnimationHint = TEXT("gesture_surround");
        Node.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.SpeakerID = TEXT("Scout Mira");
        L2.SpeakerRole = ENarr_SpeakerRole::Scout;
        L2.LineText = TEXT("They are fast. Faster than you. But they are not patient. Make them wait. Make them come to you on your terms.");
        L2.DisplayDuration = 7.0f;
        L2.AnimationHint = TEXT("crouch_ready");
        Node.Lines.Add(L2);

        Node.AutoNextNodeID = TEXT("MIRA_GREETING");
        Tree.Nodes.Add(Node);
    }

    // ---- Node: MIRA_CRAFTING_QUEST ----
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("MIRA_CRAFTING_QUEST");

        FNarr_DialogueLine L1;
        L1.SpeakerID = TEXT("Scout Mira");
        L1.SpeakerRole = ENarr_SpeakerRole::Scout;
        L1.LineText = TEXT("The grey rocks near the river bend — flint. Hard and sharp when struck right. Gather three pieces and find a straight branch. I will show you how to bind them.");
        L1.DisplayDuration = 9.0f;
        L1.AnimationHint = TEXT("point_river");
        Node.Lines.Add(L1);

        FNarr_DialogueChoice C1;
        C1.ChoiceText = TEXT("I will gather the materials.");
        C1.NextNodeID = TEXT("MIRA_CRAFT_ACCEPT");
        C1.bUnlocksQuest = true;
        C1.UnlockedQuestID = TEXT("QUEST_CRAFT_STONE_AXE");
        Node.Choices.Add(C1);

        Tree.Nodes.Add(Node);
    }

    // ---- Node: MIRA_HUNT_QUEST ----
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("MIRA_HUNT_QUEST");

        FNarr_DialogueLine L1;
        L1.SpeakerID = TEXT("Scout Mira");
        L1.SpeakerRole = ENarr_SpeakerRole::Scout;
        L1.LineText = TEXT("Alone? You are either very brave or very stupid. I have not decided which. There is a lone raptor that patrols the eastern ridge at midday. Separated from its pack. That is your best chance.");
        L1.DisplayDuration = 11.0f;
        L1.AnimationHint = TEXT("arms_crossed_skeptical");
        Node.Lines.Add(L1);

        Node.bIsEndNode = true;
        Tree.Nodes.Add(Node);
    }

    // ---- Node: MIRA_CRAFT_ACCEPT ----
    {
        FNarr_DialogueNode Node;
        Node.NodeID = TEXT("MIRA_CRAFT_ACCEPT");

        FNarr_DialogueLine L1;
        L1.SpeakerID = TEXT("Scout Mira");
        L1.SpeakerRole = ENarr_SpeakerRole::Scout;
        L1.LineText = TEXT("Good. Come back before dark. I will be at the camp fire.");
        L1.DisplayDuration = 4.0f;
        L1.AnimationHint = TEXT("nod_firm");
        Node.Lines.Add(L1);

        Node.bIsEndNode = true;
        Tree.Nodes.Add(Node);
    }

    DialogueTrees.Add(Tree);
}

// ============================================================
// Public API implementations
// ============================================================

bool ADialogueManager::StartDialogue(const FString& TreeID)
{
    FNarr_DialogueTree* Tree = FindTree(TreeID);
    if (!Tree) return false;

    CurrentState = ENarr_DialogueState::Active;
    ActiveTreeID = TreeID;
    ActiveNodeID = Tree->RootNodeID;
    ActiveLineIndex = 0;
    return true;
}

bool ADialogueManager::AdvanceLine()
{
    if (CurrentState != ENarr_DialogueState::Active) return false;

    FNarr_DialogueNode* Node = FindNode(ActiveTreeID, ActiveNodeID);
    if (!Node) return false;

    ActiveLineIndex++;

    if (ActiveLineIndex >= Node->Lines.Num())
    {
        // All lines in this node exhausted
        if (Node->Choices.Num() > 0)
        {
            CurrentState = ENarr_DialogueState::Waiting;
            return true;
        }
        else if (!Node->AutoNextNodeID.IsEmpty())
        {
            ActiveNodeID = Node->AutoNextNodeID;
            ActiveLineIndex = 0;
            return true;
        }
        else if (Node->bIsEndNode)
        {
            EndDialogue();
            return false;
        }
    }
    return true;
}

bool ADialogueManager::MakeChoice(int32 ChoiceIndex)
{
    if (CurrentState != ENarr_DialogueState::Waiting) return false;

    FNarr_DialogueNode* Node = FindNode(ActiveTreeID, ActiveNodeID);
    if (!Node || !Node->Choices.IsValidIndex(ChoiceIndex)) return false;

    const FNarr_DialogueChoice& Choice = Node->Choices[ChoiceIndex];
    ActiveNodeID = Choice.NextNodeID;
    ActiveLineIndex = 0;
    CurrentState = ENarr_DialogueState::Active;
    return true;
}

void ADialogueManager::EndDialogue()
{
    CurrentState = ENarr_DialogueState::Completed;
    ActiveTreeID.Empty();
    ActiveNodeID.Empty();
    ActiveLineIndex = 0;
}

FNarr_DialogueLine ADialogueManager::GetCurrentLine() const
{
    // const_cast is safe here — FindNode is logically const
    FNarr_DialogueNode* Node = const_cast<ADialogueManager*>(this)->FindNode(ActiveTreeID, ActiveNodeID);
    if (!Node || !Node->Lines.IsValidIndex(ActiveLineIndex))
    {
        return FNarr_DialogueLine();
    }
    return Node->Lines[ActiveLineIndex];
}

TArray<FNarr_DialogueChoice> ADialogueManager::GetCurrentChoices() const
{
    FNarr_DialogueNode* Node = const_cast<ADialogueManager*>(this)->FindNode(ActiveTreeID, ActiveNodeID);
    if (!Node) return TArray<FNarr_DialogueChoice>();
    return Node->Choices;
}

bool ADialogueManager::IsDialogueActive() const
{
    return CurrentState == ENarr_DialogueState::Active ||
           CurrentState == ENarr_DialogueState::Waiting;
}

void ADialogueManager::RegisterDialogueTree(const FNarr_DialogueTree& Tree)
{
    // Remove existing tree with same ID if present
    for (int32 i = DialogueTrees.Num() - 1; i >= 0; --i)
    {
        if (DialogueTrees[i].TreeID == Tree.TreeID)
        {
            DialogueTrees.RemoveAt(i);
            break;
        }
    }
    DialogueTrees.Add(Tree);
}

// ============================================================
// Private helpers
// ============================================================

FNarr_DialogueNode* ADialogueManager::FindNode(const FString& TreeID, const FString& NodeID)
{
    FNarr_DialogueTree* Tree = FindTree(TreeID);
    if (!Tree) return nullptr;

    for (FNarr_DialogueNode& Node : Tree->Nodes)
    {
        if (Node.NodeID == NodeID) return &Node;
    }
    return nullptr;
}

FNarr_DialogueTree* ADialogueManager::FindTree(const FString& TreeID)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID) return &Tree;
    }
    return nullptr;
}
