// DialogueManager.cpp
// Narrative & Dialogue Agent #15 — PROD_CYCLE_AUTO_20260620_005
// Manages NPC dialogue trees, conversation state, and quest-linked dialogue triggers.

#include "DialogueManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// UDialogueManager — WorldSubsystem
// ============================================================

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterDefaultDialogueTrees();
    UE_LOG(LogTemp, Log, TEXT("NarrDialogueManager: Initialized with %d dialogue trees"), DialogueTrees.Num());
}

void UNarr_DialogueManager::Deinitialize()
{
    ActiveConversation.Reset();
    DialogueTrees.Empty();
    Super::Deinitialize();
}

void UNarr_DialogueManager::RegisterDefaultDialogueTrees()
{
    // --- Elder NPC: Camp dialogue tree ---
    {
        FNarr_DialogueTree ElderTree;
        ElderTree.TreeID = FName("Elder_Camp");
        ElderTree.NPCName = FText::FromString("Elder");
        ElderTree.LinkedQuestID = FName("FirstNight");

        // Node 0 — Greeting
        FNarr_DialogueNode Greeting;
        Greeting.NodeID = 0;
        Greeting.SpeakerTag = ENarr_Speaker::NPC;
        Greeting.DialogueText = FText::FromString(
            "You survived the first night. Most do not. The cold takes them, or the dark ones do. "
            "But you are still here. That means something. Now we teach you how to stay alive."
        );
        Greeting.AudioAssetPath = FString(
            "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781938629853_ElderNPC_SurvivalWelcome.mp3"
        );
        Greeting.bIsEndNode = false;
        Greeting.NextNodeIDs = {1, 2};

        FNarr_DialogueChoice Choice1;
        Choice1.ChoiceText = FText::FromString("What dangers should I know about?");
        Choice1.NextNodeID = 1;
        Choice1.bRequiresCondition = false;

        FNarr_DialogueChoice Choice2;
        Choice2.ChoiceText = FText::FromString("I need to find food.");
        Choice2.NextNodeID = 2;
        Choice2.bRequiresCondition = false;

        Greeting.PlayerChoices = {Choice1, Choice2};
        ElderTree.Nodes.Add(Greeting);

        // Node 1 — Danger warning
        FNarr_DialogueNode DangerNode;
        DangerNode.NodeID = 1;
        DangerNode.SpeakerTag = ENarr_Speaker::NPC;
        DangerNode.DialogueText = FText::FromString(
            "She did not come back from the river. We waited until the stars moved. "
            "In the morning, we found her spear — broken near the mudflats where the big ones drink. "
            "Do not go there alone. Not ever."
        );
        DangerNode.AudioAssetPath = FString(
            "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781938618146_ElderNPC_Warning.mp3"
        );
        DangerNode.bIsEndNode = true;
        DangerNode.bTriggersQuestUpdate = true;
        DangerNode.QuestUpdateID = FName("FirstNight");
        ElderTree.Nodes.Add(DangerNode);

        // Node 2 — Food advice
        FNarr_DialogueNode FoodNode;
        FoodNode.NodeID = 2;
        FoodNode.SpeakerTag = ENarr_Speaker::NPC;
        FoodNode.DialogueText = FText::FromString(
            "The herd moves north when the rains stop. Three days, maybe four. "
            "If we follow them, we eat. If we stay here, we starve. "
            "The choice is yours — but choose fast."
        );
        FoodNode.AudioAssetPath = FString(
            "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781938620049_ScoutNPC_HerdReport.mp3"
        );
        FoodNode.bIsEndNode = true;
        FoodNode.bTriggersQuestUpdate = true;
        FoodNode.QuestUpdateID = FName("FollowTheHerd");
        ElderTree.Nodes.Add(FoodNode);

        DialogueTrees.Add(ElderTree.TreeID, ElderTree);
    }

    // --- Tracker NPC: TRex advice tree ---
    {
        FNarr_DialogueTree TrackerTree;
        TrackerTree.TreeID = FName("Tracker_RiverPost");
        TrackerTree.NPCName = FText::FromString("Tracker");
        TrackerTree.LinkedQuestID = FName("FirstBlood");

        FNarr_DialogueNode TrackerGreeting;
        TrackerGreeting.NodeID = 0;
        TrackerGreeting.SpeakerTag = ENarr_Speaker::NPC;
        TrackerGreeting.DialogueText = FText::FromString(
            "I have tracked animals since I was small enough to hide under a rock. "
            "That creature — the big one with the tiny arms — it does not hunt by sight. "
            "It hunts by smell. Stay downwind. Always stay downwind."
        );
        TrackerGreeting.AudioAssetPath = FString(
            "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781938627753_TrackerNPC_TRexAdvice.mp3"
        );
        TrackerGreeting.bIsEndNode = true;
        TrackerGreeting.bTriggersQuestUpdate = true;
        TrackerGreeting.QuestUpdateID = FName("FirstBlood");
        TrackerTree.Nodes.Add(TrackerGreeting);

        DialogueTrees.Add(TrackerTree.TreeID, TrackerTree);
    }

    UE_LOG(LogTemp, Log, TEXT("NarrDialogueManager: Registered %d dialogue trees"), DialogueTrees.Num());
}

bool UNarr_DialogueManager::StartConversation(FName TreeID, AActor* InstigatorActor)
{
    if (!DialogueTrees.Contains(TreeID))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrDialogueManager: Tree '%s' not found"), *TreeID.ToString());
        return false;
    }

    if (bConversationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrDialogueManager: Conversation already active, ignoring '%s'"), *TreeID.ToString());
        return false;
    }

    ActiveConversation = DialogueTrees[TreeID];
    CurrentNodeIndex = 0;
    bConversationActive = true;
    ConversationInstigator = InstigatorActor;

    if (ActiveConversation.IsSet())
    {
        const FNarr_DialogueTree& Tree = ActiveConversation.GetValue();
        if (Tree.Nodes.IsValidIndex(0))
        {
            OnDialogueNodeReached.Broadcast(Tree.Nodes[0]);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("NarrDialogueManager: Started conversation '%s'"), *TreeID.ToString());
    return true;
}

void UNarr_DialogueManager::AdvanceConversation(int32 ChoiceIndex)
{
    if (!bConversationActive || !ActiveConversation.IsSet())
    {
        return;
    }

    const FNarr_DialogueTree& Tree = ActiveConversation.GetValue();
    if (!Tree.Nodes.IsValidIndex(CurrentNodeIndex))
    {
        EndConversation();
        return;
    }

    const FNarr_DialogueNode& CurrentNode = Tree.Nodes[CurrentNodeIndex];

    if (CurrentNode.bIsEndNode)
    {
        // Fire quest update if needed
        if (CurrentNode.bTriggersQuestUpdate)
        {
            OnQuestDialogueTrigger.Broadcast(CurrentNode.QuestUpdateID);
        }
        EndConversation();
        return;
    }

    // Validate choice
    if (!CurrentNode.PlayerChoices.IsValidIndex(ChoiceIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrDialogueManager: Invalid choice index %d"), ChoiceIndex);
        return;
    }

    int32 NextNodeID = CurrentNode.PlayerChoices[ChoiceIndex].NextNodeID;

    // Find next node by ID
    for (int32 i = 0; i < Tree.Nodes.Num(); ++i)
    {
        if (Tree.Nodes[i].NodeID == NextNodeID)
        {
            CurrentNodeIndex = i;
            OnDialogueNodeReached.Broadcast(Tree.Nodes[i]);

            if (Tree.Nodes[i].bIsEndNode)
            {
                if (Tree.Nodes[i].bTriggersQuestUpdate)
                {
                    OnQuestDialogueTrigger.Broadcast(Tree.Nodes[i].QuestUpdateID);
                }
                // Auto-end after brief delay
                GetWorld()->GetTimerManager().SetTimer(
                    EndConversationTimer,
                    this,
                    &UNarr_DialogueManager::EndConversation,
                    3.0f,
                    false
                );
            }
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("NarrDialogueManager: Node ID %d not found"), NextNodeID);
    EndConversation();
}

void UNarr_DialogueManager::EndConversation()
{
    if (!bConversationActive)
    {
        return;
    }

    FName EndedTreeID = NAME_None;
    if (ActiveConversation.IsSet())
    {
        EndedTreeID = ActiveConversation.GetValue().TreeID;
    }

    bConversationActive = false;
    ActiveConversation.Reset();
    CurrentNodeIndex = 0;
    ConversationInstigator = nullptr;

    OnConversationEnded.Broadcast(EndedTreeID);
    UE_LOG(LogTemp, Log, TEXT("NarrDialogueManager: Conversation '%s' ended"), *EndedTreeID.ToString());
}

bool UNarr_DialogueManager::HasDialogueTree(FName TreeID) const
{
    return DialogueTrees.Contains(TreeID);
}

FNarr_DialogueTree UNarr_DialogueManager::GetDialogueTree(FName TreeID) const
{
    if (DialogueTrees.Contains(TreeID))
    {
        return DialogueTrees[TreeID];
    }
    return FNarr_DialogueTree();
}

TArray<FName> UNarr_DialogueManager::GetAllTreeIDs() const
{
    TArray<FName> Keys;
    DialogueTrees.GetKeys(Keys);
    return Keys;
}
