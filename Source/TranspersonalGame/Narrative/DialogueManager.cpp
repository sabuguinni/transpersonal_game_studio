// DialogueManager.cpp — Narrative & Dialogue Agent #15
// PROD_CYCLE_AUTO_20260620_006
// Full dialogue tree system for prehistoric survival NPCs.
// Integrates with QuestManager::StartQuest(FName) on dialogue completion events.

#include "DialogueManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────
//  Static dialogue database — all NPC trees defined here
// ─────────────────────────────────────────────────────────────

void UDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    BuildDialogueDatabase();
    ActiveDialogueID = NAME_None;
    ActiveNodeIndex = 0;
    bDialogueActive = false;
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Initialized with %d dialogue trees"), DialogueDatabase.Num());
}

void UDialogueManager::BuildDialogueDatabase()
{
    DialogueDatabase.Empty();

    // ── TRIBE ELDER KAEL ──────────────────────────────────────

    // Quest: Protect the Camp (offer)
    {
        FNarr_DialogueTree Tree;
        Tree.DialogueID = FName("Elder_ProtectCamp_Offer");
        Tree.NPCName = FText::FromString("Kael");
        Tree.QuestToStartOnComplete = FName("Quest_ProtectCamp");
        Tree.AudioURL_Offer = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781944414278_Tribe_Elder_Quest_Alert.mp3");
        Tree.AudioURL_InProgress = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781944502150_Tribe_Elder_Kael.mp3");

        // Node 0 — opening
        FNarr_DialogueNode N0;
        N0.NodeIndex = 0;
        N0.SpeakerName = FText::FromString("Kael");
        N0.DialogueText = FText::FromString("Danger! The great lizard approaches our camp! Grab your spears — we must drive it away before it destroys everything we have built!");
        N0.bIsPlayerChoice = false;
        FNarr_DialogueChoice C0A; C0A.ChoiceText = FText::FromString("I will defend the camp."); C0A.NextNodeIndex = 1;
        FNarr_DialogueChoice C0B; C0B.ChoiceText = FText::FromString("How close is it?"); C0B.NextNodeIndex = 2;
        N0.Choices.Add(C0A);
        N0.Choices.Add(C0B);
        Tree.Nodes.Add(N0);

        // Node 1 — accept
        FNarr_DialogueNode N1;
        N1.NodeIndex = 1;
        N1.SpeakerName = FText::FromString("Kael");
        N1.DialogueText = FText::FromString("Good. Drive it beyond the eastern ridge. Fire and noise — that is what it fears. Go now!");
        N1.bIsPlayerChoice = false;
        N1.bEndsDialogue = true;
        N1.bStartsQuest = true;
        Tree.Nodes.Add(N1);

        // Node 2 — info branch
        FNarr_DialogueNode N2;
        N2.NodeIndex = 2;
        N2.SpeakerName = FText::FromString("Kael");
        N2.DialogueText = FText::FromString("Listen well. The great lizard that stalks our camp — it is not just hunger that drives it. It remembers. Keep your spear close and your fire burning through the night.");
        N2.bIsPlayerChoice = false;
        FNarr_DialogueChoice C2A; C2A.ChoiceText = FText::FromString("Understood. I will drive it away."); C2A.NextNodeIndex = 1;
        N2.Choices.Add(C2A);
        Tree.Nodes.Add(N2);

        DialogueDatabase.Add(Tree.DialogueID, Tree);
    }

    // Quest: Sharper Than Claws — crafting (offer)
    {
        FNarr_DialogueTree Tree;
        Tree.DialogueID = FName("Elder_CraftingQuest_Offer");
        Tree.NPCName = FText::FromString("Kael");
        Tree.QuestToStartOnComplete = FName("Quest_SharperThanClaws");
        Tree.AudioURL_Offer = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781944523851_Tribe_Elder_Kael_CraftingQuest.mp3");

        FNarr_DialogueNode N0;
        N0.NodeIndex = 0;
        N0.SpeakerName = FText::FromString("Kael");
        N0.DialogueText = FText::FromString("Stone and wood. That is all we have between us and death. But in the right hands — shaped right, bound tight — they become something the great lizards fear. Show me you can make the tools.");
        N0.bIsPlayerChoice = false;
        FNarr_DialogueChoice C0A; C0A.ChoiceText = FText::FromString("What do I need to gather?"); C0A.NextNodeIndex = 1;
        FNarr_DialogueChoice C0B; C0B.ChoiceText = FText::FromString("I already know how to craft."); C0B.NextNodeIndex = 2;
        N0.Choices.Add(C0A);
        N0.Choices.Add(C0B);
        Tree.Nodes.Add(N0);

        FNarr_DialogueNode N1;
        N1.NodeIndex = 1;
        N1.SpeakerName = FText::FromString("Kael");
        N1.DialogueText = FText::FromString("Three flat stones from the river bed. Two straight branches from the dry forest to the north. Bring them back and I will show you how to bind them.");
        N1.bIsPlayerChoice = false;
        N1.bEndsDialogue = true;
        N1.bStartsQuest = true;
        Tree.Nodes.Add(N1);

        FNarr_DialogueNode N2;
        N2.NodeIndex = 2;
        N2.SpeakerName = FText::FromString("Kael");
        N2.DialogueText = FText::FromString("Then prove it. Gather the materials and craft an axe and a spear. Actions speak louder than words.");
        N2.bIsPlayerChoice = false;
        N2.bEndsDialogue = true;
        N2.bStartsQuest = true;
        Tree.Nodes.Add(N2);

        DialogueDatabase.Add(Tree.DialogueID, Tree);
    }

    // Elder — in-progress check-in
    {
        FNarr_DialogueTree Tree;
        Tree.DialogueID = FName("Elder_InProgress");
        Tree.NPCName = FText::FromString("Kael");
        Tree.AudioURL_InProgress = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781944502150_Tribe_Elder_Kael.mp3");

        FNarr_DialogueNode N0;
        N0.NodeIndex = 0;
        N0.SpeakerName = FText::FromString("Kael");
        N0.DialogueText = FText::FromString("Not finished yet. The tribe is counting on you. Do not come back until the task is done.");
        N0.bIsPlayerChoice = false;
        N0.bEndsDialogue = true;
        Tree.Nodes.Add(N0);

        DialogueDatabase.Add(Tree.DialogueID, Tree);
    }

    // ── TRIBE SCOUT ORRA ──────────────────────────────────────

    // Quest: The First Hunt (offer)
    {
        FNarr_DialogueTree Tree;
        Tree.DialogueID = FName("Scout_FirstHunt_Offer");
        Tree.NPCName = FText::FromString("Orra");
        Tree.QuestToStartOnComplete = FName("Quest_FirstHunt");
        Tree.AudioURL_Offer = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781944447299_Tribe_Scout_Hunt_Briefing.mp3");
        Tree.AudioURL_Complete = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781944520926_Tribe_Scout_Orra_QuestComplete.mp3");

        FNarr_DialogueNode N0;
        N0.NodeIndex = 0;
        N0.SpeakerName = FText::FromString("Orra");
        N0.DialogueText = FText::FromString("You there — hunter. The herd moves east at dawn. Three kills, maybe four. Enough meat to keep us alive through the cold nights. Do not come back empty handed.");
        N0.bIsPlayerChoice = false;
        FNarr_DialogueChoice C0A; C0A.ChoiceText = FText::FromString("I will bring back the meat."); C0A.NextNodeIndex = 1;
        FNarr_DialogueChoice C0B; C0B.ChoiceText = FText::FromString("Which animals are in the herd?"); C0B.NextNodeIndex = 2;
        N0.Choices.Add(C0A);
        N0.Choices.Add(C0B);
        Tree.Nodes.Add(N0);

        FNarr_DialogueNode N1;
        N1.NodeIndex = 1;
        N1.SpeakerName = FText::FromString("Orra");
        N1.DialogueText = FText::FromString("Move fast, stay downwind. The herd spooks easily. Three kills is all we need.");
        N1.bIsPlayerChoice = false;
        N1.bEndsDialogue = true;
        N1.bStartsQuest = true;
        Tree.Nodes.Add(N1);

        FNarr_DialogueNode N2;
        N2.NodeIndex = 2;
        N2.SpeakerName = FText::FromString("Orra");
        N2.DialogueText = FText::FromString("Long-necks mostly. Slow, but their legs can crush you if you get too close. Aim for the neck. One clean throw.");
        N2.bIsPlayerChoice = false;
        FNarr_DialogueChoice C2A; C2A.ChoiceText = FText::FromString("Understood. I will go now."); C2A.NextNodeIndex = 1;
        N2.Choices.Add(C2A);
        Tree.Nodes.Add(N2);

        DialogueDatabase.Add(Tree.DialogueID, Tree);
    }

    // Quest: Know Your Ground — exploration (offer)
    {
        FNarr_DialogueTree Tree;
        Tree.DialogueID = FName("Scout_KnowYourGround_Offer");
        Tree.NPCName = FText::FromString("Orra");
        Tree.QuestToStartOnComplete = FName("Quest_KnowYourGround");
        Tree.AudioURL_Offer = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781944509130_Tribe_Scout_Orra.mp3");

        FNarr_DialogueNode N0;
        N0.NodeIndex = 0;
        N0.SpeakerName = FText::FromString("Orra");
        N0.DialogueText = FText::FromString("You want to know this land? Then walk it. Every ridge, every river bend, every shadow where a predator might wait. I have mapped it in my head. You must do the same.");
        N0.bIsPlayerChoice = false;
        FNarr_DialogueChoice C0A; C0A.ChoiceText = FText::FromString("Where do I start?"); C0A.NextNodeIndex = 1;
        FNarr_DialogueChoice C0B; C0B.ChoiceText = FText::FromString("How many vantage points?"); C0B.NextNodeIndex = 2;
        N0.Choices.Add(C0A);
        N0.Choices.Add(C0B);
        Tree.Nodes.Add(N0);

        FNarr_DialogueNode N1;
        N1.NodeIndex = 1;
        N1.SpeakerName = FText::FromString("Orra");
        N1.DialogueText = FText::FromString("North ridge first. You can see three valleys from there. Then work your way south. Do not rush — a scout who misses a detail is a dead scout.");
        N1.bIsPlayerChoice = false;
        N1.bEndsDialogue = true;
        N1.bStartsQuest = true;
        Tree.Nodes.Add(N1);

        FNarr_DialogueNode N2;
        N2.NodeIndex = 2;
        N2.SpeakerName = FText::FromString("Orra");
        N2.DialogueText = FText::FromString("Four. North ridge, east bluff, river crossing, and the high rock above the valley. Each one tells you something different about the land.");
        N2.bIsPlayerChoice = false;
        FNarr_DialogueChoice C2A; C2A.ChoiceText = FText::FromString("I will scout all four."); C2A.NextNodeIndex = 1;
        N2.Choices.Add(C2A);
        Tree.Nodes.Add(N2);

        DialogueDatabase.Add(Tree.DialogueID, Tree);
    }

    // Scout — quest completion dialogue
    {
        FNarr_DialogueTree Tree;
        Tree.DialogueID = FName("Scout_HuntComplete");
        Tree.NPCName = FText::FromString("Orra");
        Tree.AudioURL_Complete = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781944520926_Tribe_Scout_Orra_QuestComplete.mp3");

        FNarr_DialogueNode N0;
        N0.NodeIndex = 0;
        N0.SpeakerName = FText::FromString("Orra");
        N0.DialogueText = FText::FromString("Good. You came back. Most do not, the first time they face the herd alone. The meat you carry will feed twelve mouths tonight. That is not nothing. That is everything.");
        N0.bIsPlayerChoice = false;
        N0.bEndsDialogue = true;
        Tree.Nodes.Add(N0);

        DialogueDatabase.Add(Tree.DialogueID, Tree);
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Built %d dialogue trees"), DialogueDatabase.Num());
}

// ─────────────────────────────────────────────────────────────
//  Runtime dialogue control
// ─────────────────────────────────────────────────────────────

bool UDialogueManager::StartDialogue(FName DialogueID)
{
    if (!DialogueDatabase.Contains(DialogueID))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Dialogue '%s' not found"), *DialogueID.ToString());
        return false;
    }

    ActiveDialogueID = DialogueID;
    ActiveNodeIndex = 0;
    bDialogueActive = true;

    const FNarr_DialogueTree& Tree = DialogueDatabase[DialogueID];
    OnDialogueStarted.Broadcast(DialogueID, Tree.NPCName);

    AdvanceToNode(0);
    return true;
}

void UDialogueManager::SelectChoice(int32 ChoiceIndex)
{
    if (!bDialogueActive || !DialogueDatabase.Contains(ActiveDialogueID))
    {
        return;
    }

    const FNarr_DialogueTree& Tree = DialogueDatabase[ActiveDialogueID];
    if (!Tree.Nodes.IsValidIndex(ActiveNodeIndex))
    {
        return;
    }

    const FNarr_DialogueNode& Node = Tree.Nodes[ActiveNodeIndex];
    if (!Node.Choices.IsValidIndex(ChoiceIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Invalid choice index %d"), ChoiceIndex);
        return;
    }

    int32 NextNode = Node.Choices[ChoiceIndex].NextNodeIndex;
    AdvanceToNode(NextNode);
}

void UDialogueManager::AdvanceToNode(int32 NodeIndex)
{
    if (!DialogueDatabase.Contains(ActiveDialogueID))
    {
        return;
    }

    FNarr_DialogueTree& Tree = DialogueDatabase[ActiveDialogueID];

    // Find node by index (not array position)
    FNarr_DialogueNode* FoundNode = nullptr;
    for (FNarr_DialogueNode& N : Tree.Nodes)
    {
        if (N.NodeIndex == NodeIndex)
        {
            FoundNode = &N;
            break;
        }
    }

    if (!FoundNode)
    {
        EndDialogue();
        return;
    }

    ActiveNodeIndex = NodeIndex;
    OnDialogueNodeAdvanced.Broadcast(ActiveDialogueID, *FoundNode);

    // Auto-end if flagged
    if (FoundNode->bEndsDialogue)
    {
        if (FoundNode->bStartsQuest && Tree.QuestToStartOnComplete != NAME_None)
        {
            OnQuestShouldStart.Broadcast(Tree.QuestToStartOnComplete);
        }
        EndDialogue();
    }
}

void UDialogueManager::EndDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    FName EndedID = ActiveDialogueID;
    bDialogueActive = false;
    ActiveDialogueID = NAME_None;
    ActiveNodeIndex = 0;

    OnDialogueEnded.Broadcast(EndedID);
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Dialogue '%s' ended"), *EndedID.ToString());
}

bool UDialogueManager::IsDialogueActive() const
{
    return bDialogueActive;
}

FNarr_DialogueTree UDialogueManager::GetDialogueTree(FName DialogueID) const
{
    if (DialogueDatabase.Contains(DialogueID))
    {
        return DialogueDatabase[DialogueID];
    }
    return FNarr_DialogueTree();
}

TArray<FName> UDialogueManager::GetAllDialogueIDs() const
{
    TArray<FName> Keys;
    DialogueDatabase.GetKeys(Keys);
    return Keys;
}

FName UDialogueManager::GetActiveDialogueID() const
{
    return ActiveDialogueID;
}
