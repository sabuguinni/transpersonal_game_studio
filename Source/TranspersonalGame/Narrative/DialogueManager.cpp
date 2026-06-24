#include "DialogueManager.h"
#include "Engine/World.h"

ADialogueManager::ADialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;
    ActiveTreeID = TEXT("");
    ActiveLineID = TEXT("");
}

void ADialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogueTrees();
}

void ADialogueManager::InitializeDefaultDialogueTrees()
{
    // ============================================================
    // TREE 1: Hunter NPC — Quest QUEST_HUNT_RAPTOR_001
    // Trigger zone: DialogueTrigger_Hunter_001 at (1500, 1000, 250)
    // Voice: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782328316758_QuestGiver_Hunter.mp3
    // ============================================================
    {
        FNarr_DialogueTree HunterTree;
        HunterTree.TreeID = TEXT("TREE_HUNTER_001");
        HunterTree.NPCName = TEXT("Kael — Hunter");
        HunterTree.FirstLineID = TEXT("H001_L1");
        HunterTree.State = ENarr_DialogueState::Idle;

        FNarr_DialogueLine L1;
        L1.LineID = TEXT("H001_L1");
        L1.SpeakerName = TEXT("Kael");
        L1.LineText = TEXT("The hunting ground is dangerous. Three raptors were spotted near the river crossing at dawn. Gather your spear and follow the eastern trail.");
        L1.LineType = ENarr_DialogueType::QuestGive;
        L1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782328316758_QuestGiver_Hunter.mp3");
        L1.DisplayDuration = 6.0f;
        L1.NextLineID = TEXT("H001_L2");
        L1.TriggersQuestID = TEXT("QUEST_HUNT_RAPTOR_001");
        HunterTree.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.LineID = TEXT("H001_L2");
        L2.SpeakerName = TEXT("Kael");
        L2.LineText = TEXT("Stay low and listen. The raptors hunt in packs. One circles wide, two come from the front. If you hear the clicking sound — run. Use the rocks for cover.");
        L2.LineType = ENarr_DialogueType::Warning;
        L2.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782328450652_HunterNPC_Dialogue.mp3");
        L2.DisplayDuration = 7.0f;
        L2.NextLineID = TEXT("");
        L2.TriggersQuestID = TEXT("");
        HunterTree.Lines.Add(L2);

        DialogueTrees.Add(HunterTree);
    }

    // ============================================================
    // TREE 2: Tutorial NPC — Stone Axe crafting tutorial
    // Trigger zone: DialogueTrigger_Gather_001 at (-800, 600, 250)
    // Voice: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782328472201_TutorialNPC_Crafting.mp3
    // ============================================================
    {
        FNarr_DialogueTree CraftTree;
        CraftTree.TreeID = TEXT("TREE_CRAFT_001");
        CraftTree.NPCName = TEXT("Mira — Elder");
        CraftTree.FirstLineID = TEXT("C001_L1");
        CraftTree.State = ENarr_DialogueState::Idle;

        FNarr_DialogueLine L1;
        L1.LineID = TEXT("C001_L1");
        L1.SpeakerName = TEXT("Mira");
        L1.LineText = TEXT("The stone axe. Two rocks, one stick. Bind them tight with vine. It will not last long — but long enough to crack a skull. Watch me. Now you try.");
        L1.LineType = ENarr_DialogueType::Tutorial;
        L1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782328472201_TutorialNPC_Crafting.mp3");
        L1.DisplayDuration = 6.0f;
        L1.NextLineID = TEXT("C001_L2");
        L1.TriggersQuestID = TEXT("QUEST_GATHER_AXEMAT_001");
        CraftTree.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.LineID = TEXT("C001_L2");
        L2.SpeakerName = TEXT("Mira");
        L2.LineText = TEXT("Rocks are everywhere near the river. Sticks from dead branches only — green wood bends, it will not hold. Bring me what you find.");
        L2.LineType = ENarr_DialogueType::Tutorial;
        L2.AudioURL = TEXT("");
        L2.DisplayDuration = 5.0f;
        L2.NextLineID = TEXT("");
        L2.TriggersQuestID = TEXT("");
        CraftTree.Lines.Add(L2);

        DialogueTrees.Add(CraftTree);
    }

    // ============================================================
    // TREE 3: Scout NPC — Night raid warning
    // Trigger zone: DialogueTrigger_Defend_001 at (200, -900, 250)
    // Voice: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782328337821_QuestGiver_Scout.mp3
    // ============================================================
    {
        FNarr_DialogueTree DefendTree;
        DefendTree.TreeID = TEXT("TREE_DEFEND_001");
        DefendTree.NPCName = TEXT("Ryn — Scout");
        DefendTree.FirstLineID = TEXT("D001_L1");
        DefendTree.State = ENarr_DialogueState::Idle;

        FNarr_DialogueLine L1;
        L1.LineID = TEXT("D001_L1");
        L1.SpeakerName = TEXT("Ryn");
        L1.LineText = TEXT("You found the nest. Five eggs, still warm. Take them carefully — the mother cannot be far. Move fast and stay low.");
        L1.LineType = ENarr_DialogueType::QuestGive;
        L1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782328337821_QuestGiver_Scout.mp3");
        L1.DisplayDuration = 5.0f;
        L1.NextLineID = TEXT("D001_L2");
        L1.TriggersQuestID = TEXT("QUEST_DEFEND_CAMP_001");
        DefendTree.Lines.Add(L1);

        FNarr_DialogueLine L2;
        L2.LineID = TEXT("D001_L2");
        L2.SpeakerName = TEXT("Ryn");
        L2.LineText = TEXT("They come at night when the fire dies. Five of them, maybe more. Keep the torches burning. If one gets inside the camp perimeter, we lose people. Hold the line.");
        L2.LineType = ENarr_DialogueType::Warning;
        L2.AudioURL = TEXT("");
        L2.DisplayDuration = 7.0f;
        L2.NextLineID = TEXT("");
        L2.TriggersQuestID = TEXT("");
        DefendTree.Lines.Add(L2);

        DialogueTrees.Add(DefendTree);
    }
}

bool ADialogueManager::StartDialogue(const FString& TreeID)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID)
        {
            if (Tree.State == ENarr_DialogueState::Completed || Tree.State == ENarr_DialogueState::Locked)
            {
                return false;
            }
            ActiveTreeID = TreeID;
            ActiveLineID = Tree.FirstLineID;
            Tree.State = ENarr_DialogueState::Active;
            return true;
        }
    }
    return false;
}

bool ADialogueManager::AdvanceDialogue()
{
    if (ActiveTreeID.IsEmpty() || ActiveLineID.IsEmpty())
    {
        return false;
    }

    FNarr_DialogueLine* CurrentLine = FindLine(ActiveTreeID, ActiveLineID);
    if (!CurrentLine)
    {
        EndDialogue();
        return false;
    }

    if (CurrentLine->NextLineID.IsEmpty())
    {
        EndDialogue();
        return false;
    }

    ActiveLineID = CurrentLine->NextLineID;
    return true;
}

void ADialogueManager::EndDialogue()
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == ActiveTreeID)
        {
            Tree.State = ENarr_DialogueState::Completed;
            break;
        }
    }
    ActiveTreeID = TEXT("");
    ActiveLineID = TEXT("");
}

FNarr_DialogueLine ADialogueManager::GetCurrentLine() const
{
    if (ActiveTreeID.IsEmpty() || ActiveLineID.IsEmpty())
    {
        return FNarr_DialogueLine();
    }

    for (const FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == ActiveTreeID)
        {
            for (const FNarr_DialogueLine& Line : Tree.Lines)
            {
                if (Line.LineID == ActiveLineID)
                {
                    return Line;
                }
            }
        }
    }
    return FNarr_DialogueLine();
}

bool ADialogueManager::GetDialogueTree(const FString& TreeID, FNarr_DialogueTree& OutTree) const
{
    for (const FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID)
        {
            OutTree = Tree;
            return true;
        }
    }
    return false;
}

void ADialogueManager::RegisterDialogueTree(const FNarr_DialogueTree& Tree)
{
    // Remove existing tree with same ID if present
    DialogueTrees.RemoveAll([&Tree](const FNarr_DialogueTree& Existing)
    {
        return Existing.TreeID == Tree.TreeID;
    });
    DialogueTrees.Add(Tree);
}

void ADialogueManager::MarkTreeCompleted(const FString& TreeID)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID)
        {
            Tree.State = ENarr_DialogueState::Completed;
            return;
        }
    }
}

bool ADialogueManager::IsDialogueActive() const
{
    return !ActiveTreeID.IsEmpty();
}

FNarr_DialogueLine* ADialogueManager::FindLine(const FString& TreeID, const FString& LineID)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID)
        {
            for (FNarr_DialogueLine& Line : Tree.Lines)
            {
                if (Line.LineID == LineID)
                {
                    return &Line;
                }
            }
        }
    }
    return nullptr;
}
