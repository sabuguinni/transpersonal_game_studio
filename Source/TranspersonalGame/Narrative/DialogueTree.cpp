
#include "DialogueTree.h"
#include "Engine/World.h"

UDialogueTree::UDialogueTree()
{
    PrimaryComponentTick.bCanEverTick = false;
    NPCID = NAME_None;
    CurrentNodeID = NAME_None;
    bConversationActive = false;
}

void UDialogueTree::BeginPlay()
{
    Super::BeginPlay();

    // Auto-register default dialogue based on NPC role
    switch (NPCRole)
    {
        case ENarr_NPCRole::Elder:
            RegisterDefaultDialogue_Elder();
            break;
        case ENarr_NPCRole::Scout:
            RegisterDefaultDialogue_Scout();
            break;
        case ENarr_NPCRole::CampLeader:
            RegisterDefaultDialogue_CampLeader();
            break;
        default:
            break;
    }
}

void UDialogueTree::StartConversation()
{
    if (bConversationActive || DialogueNodes.Num() == 0)
    {
        return;
    }

    // Find entry node
    for (const FNarr_DialogueNode& Node : DialogueNodes)
    {
        if (Node.bIsEntryNode && IsConditionMet(Node))
        {
            CurrentNodeID = Node.NodeID;
            bConversationActive = true;
            OnDialogueNodeReached.Broadcast(CurrentNodeID);
            return;
        }
    }
}

void UDialogueTree::SelectChoice(int32 ChoiceIndex)
{
    if (!bConversationActive)
    {
        return;
    }

    FNarr_DialogueNode* CurrentNode = FindNode(CurrentNodeID);
    if (!CurrentNode)
    {
        return;
    }

    if (!CurrentNode->Choices.IsValidIndex(ChoiceIndex))
    {
        return;
    }

    const FNarr_DialogueChoice& Choice = CurrentNode->Choices[ChoiceIndex];

    // Unlock quest if applicable
    if (!Choice.UnlocksQuestID.IsNone())
    {
        KnownQuestIDs.AddUnique(Choice.UnlocksQuestID);
        OnQuestUnlocked.Broadcast(Choice.UnlocksQuestID);
    }

    // End conversation or advance
    if (Choice.bEndsConversation || Choice.NextNodeID.IsNone())
    {
        EndConversation();
        return;
    }

    // Advance to next node
    FNarr_DialogueNode* NextNode = FindNode(Choice.NextNodeID);
    if (NextNode && IsConditionMet(*NextNode))
    {
        CurrentNodeID = Choice.NextNodeID;
        OnDialogueNodeReached.Broadcast(CurrentNodeID);
    }
    else
    {
        EndConversation();
    }
}

void UDialogueTree::EndConversation()
{
    bConversationActive = false;
    CurrentNodeID = NAME_None;
    OnConversationEnded.Broadcast();
}

FNarr_DialogueNode UDialogueTree::GetCurrentNode() const
{
    for (const FNarr_DialogueNode& Node : DialogueNodes)
    {
        if (Node.NodeID == CurrentNodeID)
        {
            return Node;
        }
    }
    return FNarr_DialogueNode();
}

bool UDialogueTree::IsConditionMet(const FNarr_DialogueNode& Node) const
{
    switch (Node.Condition)
    {
        case ENarr_DialogueConditionType::None:
            return true;

        case ENarr_DialogueConditionType::QuestActive:
            return KnownQuestIDs.Contains(Node.ConditionQuestID);

        case ENarr_DialogueConditionType::QuestComplete:
            return CompletedQuestIDs.Contains(Node.ConditionQuestID);

        case ENarr_DialogueConditionType::FirstMeeting:
            // Always true on first conversation — tracked externally
            return true;

        default:
            return true;
    }
}

FNarr_DialogueNode* UDialogueTree::FindNode(FName NodeID)
{
    for (FNarr_DialogueNode& Node : DialogueNodes)
    {
        if (Node.NodeID == NodeID)
        {
            return &Node;
        }
    }
    return nullptr;
}

// ============================================================
// ELDER DIALOGUE TREE
// Quest hooks: Quest_CraftFirstWeapon
// Audio: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782874950792_QuestNPC_Elder_DialogueTree.mp3
// ============================================================
void UDialogueTree::RegisterDefaultDialogue_Elder()
{
    DialogueNodes.Empty();

    // Node 1 — Entry: Elder greets player
    FNarr_DialogueNode Entry;
    Entry.NodeID = FName("Elder_Greet");
    Entry.NPCRole = ENarr_NPCRole::Elder;
    Entry.NPCLine = FText::FromString("When you craft your first axe, you become more than a survivor. You become a maker. The tribe remembers those who build, not just those who run. Bring me the stone and the stick.");
    Entry.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782874950792_QuestNPC_Elder_DialogueTree.mp3";
    Entry.Condition = ENarr_DialogueConditionType::None;
    Entry.bIsEntryNode = true;

    FNarr_DialogueChoice ChoiceAccept;
    ChoiceAccept.ChoiceText = FText::FromString("I will find the stone and the stick.");
    ChoiceAccept.NextNodeID = FName("Elder_QuestGiven");
    ChoiceAccept.UnlocksQuestID = FName("Quest_CraftFirstWeapon");
    ChoiceAccept.bEndsConversation = false;

    FNarr_DialogueChoice ChoiceAsk;
    ChoiceAsk.ChoiceText = FText::FromString("Why do we need an axe?");
    ChoiceAsk.NextNodeID = FName("Elder_Explain");
    ChoiceAsk.bEndsConversation = false;

    FNarr_DialogueChoice ChoiceLeave;
    ChoiceLeave.ChoiceText = FText::FromString("Not now.");
    ChoiceLeave.bEndsConversation = true;

    Entry.Choices.Add(ChoiceAccept);
    Entry.Choices.Add(ChoiceAsk);
    Entry.Choices.Add(ChoiceLeave);
    DialogueNodes.Add(Entry);

    // Node 2 — Elder explains why crafting matters
    FNarr_DialogueNode Explain;
    Explain.NodeID = FName("Elder_Explain");
    Explain.NPCRole = ENarr_NPCRole::Elder;
    Explain.NPCLine = FText::FromString("The raptors are faster than us. The T-Rex is stronger. Our only advantage is what we make with our hands. A sharp stone on a stick is the difference between prey and predator.");
    Explain.Condition = ENarr_DialogueConditionType::None;

    FNarr_DialogueChoice ChoiceUnderstood;
    ChoiceUnderstood.ChoiceText = FText::FromString("I understand. I will craft the axe.");
    ChoiceUnderstood.NextNodeID = FName("Elder_QuestGiven");
    ChoiceUnderstood.UnlocksQuestID = FName("Quest_CraftFirstWeapon");
    ChoiceUnderstood.bEndsConversation = false;
    Explain.Choices.Add(ChoiceUnderstood);
    DialogueNodes.Add(Explain);

    // Node 3 — Quest confirmed
    FNarr_DialogueNode QuestGiven;
    QuestGiven.NodeID = FName("Elder_QuestGiven");
    QuestGiven.NPCRole = ENarr_NPCRole::Elder;
    QuestGiven.NPCLine = FText::FromString("Good. Two stones, one stick. You will find them near the river bank. Come back when you have crafted the axe. The tribe is counting on you.");
    QuestGiven.Condition = ENarr_DialogueConditionType::None;

    FNarr_DialogueChoice ChoiceDismiss;
    ChoiceDismiss.ChoiceText = FText::FromString("I will return.");
    ChoiceDismiss.bEndsConversation = true;
    QuestGiven.Choices.Add(ChoiceDismiss);
    DialogueNodes.Add(QuestGiven);
}

// ============================================================
// SCOUT DIALOGUE TREE
// Quest hooks: Quest_RaptorTrack
// Audio: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782874973882_QuestNPC_Scout_DialogueTree.mp3
// ============================================================
void UDialogueTree::RegisterDefaultDialogue_Scout()
{
    DialogueNodes.Empty();

    // Node 1 — Entry: Scout urgent report
    FNarr_DialogueNode Entry;
    Entry.NodeID = FName("Scout_Report");
    Entry.NPCRole = ENarr_NPCRole::Scout;
    Entry.NPCLine = FText::FromString("I tracked the raptors to the northern canyon. Three of them, maybe four. They are hunting in a pack now. We need a defensive perimeter before nightfall.");
    Entry.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782874973882_QuestNPC_Scout_DialogueTree.mp3";
    Entry.Condition = ENarr_DialogueConditionType::None;
    Entry.bIsEntryNode = true;

    FNarr_DialogueChoice ChoiceTrack;
    ChoiceTrack.ChoiceText = FText::FromString("I will track them and mark the den location.");
    ChoiceTrack.NextNodeID = FName("Scout_QuestGiven");
    ChoiceTrack.UnlocksQuestID = FName("Quest_RaptorTrack");
    ChoiceTrack.bEndsConversation = false;

    FNarr_DialogueChoice ChoiceAskMore;
    ChoiceAskMore.ChoiceText = FText::FromString("How many raptors exactly?");
    ChoiceAskMore.NextNodeID = FName("Scout_Details");
    ChoiceAskMore.bEndsConversation = false;

    Entry.Choices.Add(ChoiceTrack);
    Entry.Choices.Add(ChoiceAskMore);
    DialogueNodes.Add(Entry);

    // Node 2 — Scout provides details
    FNarr_DialogueNode Details;
    Details.NodeID = FName("Scout_Details");
    Details.NPCRole = ENarr_NPCRole::Scout;
    Details.NPCLine = FText::FromString("I counted three for certain. There may be a fourth hiding in the canyon shadows. They move fast — faster than I expected. Do not engage them alone.");
    Details.Condition = ENarr_DialogueConditionType::None;

    FNarr_DialogueChoice ChoiceAccept;
    ChoiceAccept.ChoiceText = FText::FromString("Understood. I will track them carefully.");
    ChoiceAccept.NextNodeID = FName("Scout_QuestGiven");
    ChoiceAccept.UnlocksQuestID = FName("Quest_RaptorTrack");
    ChoiceAccept.bEndsConversation = false;
    Details.Choices.Add(ChoiceAccept);
    DialogueNodes.Add(Details);

    // Node 3 — Quest confirmed
    FNarr_DialogueNode QuestGiven;
    QuestGiven.NodeID = FName("Scout_QuestGiven");
    QuestGiven.NPCRole = ENarr_NPCRole::Scout;
    QuestGiven.NPCLine = FText::FromString("Head north along the ridge. Stay downwind. Mark the den location and return before dark. We cannot afford to lose another tracker.");
    QuestGiven.Condition = ENarr_DialogueConditionType::None;

    FNarr_DialogueChoice ChoiceDismiss;
    ChoiceDismiss.ChoiceText = FText::FromString("I will be careful.");
    ChoiceDismiss.bEndsConversation = true;
    QuestGiven.Choices.Add(ChoiceDismiss);
    DialogueNodes.Add(QuestGiven);
}

// ============================================================
// CAMP LEADER DIALOGUE TREE
// Quest hooks: Quest_FindWaterSource, Quest_SurviveFirstNight
// Audio: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782875002289_QuestNPC_CampLeader_DialogueTr.mp3
// ============================================================
void UDialogueTree::RegisterDefaultDialogue_CampLeader()
{
    DialogueNodes.Empty();

    // Node 1 — Entry: Camp Leader assigns water mission
    FNarr_DialogueNode Entry;
    Entry.NodeID = FName("Leader_WaterMission");
    Entry.NPCRole = ENarr_NPCRole::CampLeader;
    Entry.NPCLine = FText::FromString("The water container is not a luxury — it is survival. Without water, we cannot travel. Without travel, we cannot find the herd. Craft the container. Find the river. Bring back enough for three days.");
    Entry.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782875002289_QuestNPC_CampLeader_DialogueTr.mp3";
    Entry.Condition = ENarr_DialogueConditionType::None;
    Entry.bIsEntryNode = true;

    FNarr_DialogueChoice ChoiceWater;
    ChoiceWater.ChoiceText = FText::FromString("I will find the river and bring back water.");
    ChoiceWater.NextNodeID = FName("Leader_WaterQuestGiven");
    ChoiceWater.UnlocksQuestID = FName("Quest_FindWaterSource");
    ChoiceWater.bEndsConversation = false;

    FNarr_DialogueChoice ChoiceNight;
    ChoiceNight.ChoiceText = FText::FromString("What about tonight? How do we survive the night?");
    ChoiceNight.NextNodeID = FName("Leader_NightSurvival");
    ChoiceNight.bEndsConversation = false;

    Entry.Choices.Add(ChoiceWater);
    Entry.Choices.Add(ChoiceNight);
    DialogueNodes.Add(Entry);

    // Node 2 — Water quest confirmed
    FNarr_DialogueNode WaterGiven;
    WaterGiven.NodeID = FName("Leader_WaterQuestGiven");
    WaterGiven.NPCRole = ENarr_NPCRole::CampLeader;
    WaterGiven.NPCLine = FText::FromString("The river is two ridges east. Watch for crocodilians near the bank. Fill the container and get back before the predators start their evening hunt.");
    WaterGiven.Condition = ENarr_DialogueConditionType::None;

    FNarr_DialogueChoice ChoiceDismiss;
    ChoiceDismiss.ChoiceText = FText::FromString("I will be back before dark.");
    ChoiceDismiss.bEndsConversation = true;
    WaterGiven.Choices.Add(ChoiceDismiss);
    DialogueNodes.Add(WaterGiven);

    // Node 3 — Night survival briefing
    FNarr_DialogueNode NightSurvival;
    NightSurvival.NodeID = FName("Leader_NightSurvival");
    NightSurvival.NPCRole = ENarr_NPCRole::CampLeader;
    NightSurvival.NPCLine = FText::FromString("Build a campfire. The predators avoid fire — most of them. Three sticks is all you need. Keep it burning through the night and you will see morning.");
    NightSurvival.Condition = ENarr_DialogueConditionType::None;

    FNarr_DialogueChoice ChoiceNightAccept;
    ChoiceNightAccept.ChoiceText = FText::FromString("I will build the campfire and keep watch.");
    ChoiceNightAccept.NextNodeID = FName("Leader_WaterQuestGiven");
    ChoiceNightAccept.UnlocksQuestID = FName("Quest_SurviveFirstNight");
    ChoiceNightAccept.bEndsConversation = false;
    NightSurvival.Choices.Add(ChoiceNightAccept);
    DialogueNodes.Add(NightSurvival);
}
