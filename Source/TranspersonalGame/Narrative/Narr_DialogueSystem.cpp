#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bDialogueActive = false;
    CurrentSpeaker = nullptr;
    CurrentListener = nullptr;
    CurrentTreeID = TEXT("");

    // Create default dialogue trees
    CreateHunterDialogue();
    CreateElderDialogue();
    CreateScoutDialogue();

    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue System initialized with %d dialogue trees"), DialogueTrees.Num());
}

void UNarr_DialogueSystem::StartDialogue(const FString& TreeID, AActor* Speaker, AActor* Listener)
{
    if (!Speaker || !Listener)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue: Speaker or Listener is null"));
        return;
    }

    if (!DialogueTrees.Contains(TreeID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue tree %s not found"), *TreeID);
        return;
    }

    CurrentTreeID = TreeID;
    CurrentSpeaker = Speaker;
    CurrentListener = Listener;
    bDialogueActive = true;

    // Reset to first node
    DialogueTrees[TreeID].CurrentNodeIndex = 0;

    ProcessCurrentNode();

    UE_LOG(LogTemp, Log, TEXT("Started dialogue %s between %s and %s"), 
        *TreeID, 
        *Speaker->GetName(), 
        *Listener->GetName());
}

void UNarr_DialogueSystem::SelectPlayerResponse(int32 ResponseIndex)
{
    if (!bDialogueActive || CurrentTreeID.IsEmpty())
    {
        return;
    }

    FNarr_DialogueTree& CurrentTree = DialogueTrees[CurrentTreeID];
    if (!CurrentTree.DialogueNodes.IsValidIndex(CurrentTree.CurrentNodeIndex))
    {
        return;
    }

    const FNarr_DialogueNode& CurrentNode = CurrentTree.DialogueNodes[CurrentTree.CurrentNodeIndex];
    
    if (CurrentNode.NextNodeIndices.IsValidIndex(ResponseIndex))
    {
        int32 NextIndex = CurrentNode.NextNodeIndices[ResponseIndex];
        AdvanceToNextNode(NextIndex);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid response index %d for dialogue node"), ResponseIndex);
        EndDialogue();
    }
}

void UNarr_DialogueSystem::EndDialogue()
{
    bDialogueActive = false;
    CurrentTreeID = TEXT("");
    CurrentSpeaker = nullptr;
    CurrentListener = nullptr;

    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

FNarr_DialogueNode UNarr_DialogueSystem::GetCurrentDialogueNode() const
{
    if (!bDialogueActive || CurrentTreeID.IsEmpty())
    {
        return FNarr_DialogueNode();
    }

    const FNarr_DialogueTree& CurrentTree = DialogueTrees[CurrentTreeID];
    if (CurrentTree.DialogueNodes.IsValidIndex(CurrentTree.CurrentNodeIndex))
    {
        return CurrentTree.DialogueNodes[CurrentTree.CurrentNodeIndex];
    }

    return FNarr_DialogueNode();
}

bool UNarr_DialogueSystem::IsDialogueActive() const
{
    return bDialogueActive;
}

void UNarr_DialogueSystem::RegisterDialogueTree(const FNarr_DialogueTree& DialogueTree)
{
    DialogueTrees.Add(DialogueTree.TreeID, DialogueTree);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue tree: %s"), *DialogueTree.TreeID);
}

void UNarr_DialogueSystem::CreateHunterDialogue()
{
    FNarr_DialogueTree HunterTree;
    HunterTree.TreeID = TEXT("HunterDialogue");

    // Node 0: Initial greeting
    FNarr_DialogueNode Node0;
    Node0.SpeakerName = TEXT("Veteran Hunter");
    Node0.DialogueText = TEXT("Greetings, newcomer. I see the hunger for the hunt in your eyes. These lands are dangerous, but rich with prey.");
    Node0.PlayerResponses.Add(TEXT("Tell me about the dangers here."));
    Node0.PlayerResponses.Add(TEXT("What prey can I find?"));
    Node0.PlayerResponses.Add(TEXT("I need to go."));
    Node0.NextNodeIndices.Add(1); // Dangers
    Node0.NextNodeIndices.Add(2); // Prey
    Node0.NextNodeIndices.Add(-1); // End dialogue
    HunterTree.DialogueNodes.Add(Node0);

    // Node 1: About dangers
    FNarr_DialogueNode Node1;
    Node1.SpeakerName = TEXT("Veteran Hunter");
    Node1.DialogueText = TEXT("The great predators rule these lands. T-Rex in the eastern cliffs, raptor packs in the forests. Never hunt alone after dark.");
    Node1.PlayerResponses.Add(TEXT("How do I survive against them?"));
    Node1.PlayerResponses.Add(TEXT("What about the herbivores?"));
    Node1.NextNodeIndices.Add(3); // Survival tips
    Node1.NextNodeIndices.Add(2); // Prey info
    HunterTree.DialogueNodes.Add(Node1);

    // Node 2: About prey
    FNarr_DialogueNode Node2;
    Node2.SpeakerName = TEXT("Veteran Hunter");
    Node2.DialogueText = TEXT("Triceratops graze in the plains - their meat feeds many, but their horns are deadly. Smaller prey hides in the underbrush.");
    Node2.PlayerResponses.Add(TEXT("Any hunting tips?"));
    Node2.PlayerResponses.Add(TEXT("Thank you for the advice."));
    Node2.NextNodeIndices.Add(3); // Tips
    Node2.NextNodeIndices.Add(-1); // End
    HunterTree.DialogueNodes.Add(Node2);

    // Node 3: Survival tips
    FNarr_DialogueNode Node3;
    Node3.SpeakerName = TEXT("Veteran Hunter");
    Node3.DialogueText = TEXT("Stay downwind, move quietly, and always have an escape route. The beasts have keen senses - respect them or become their meal.");
    Node3.PlayerResponses.Add(TEXT("I understand. Thank you."));
    Node3.NextNodeIndices.Add(-1); // End
    HunterTree.DialogueNodes.Add(Node3);

    DialogueTrees.Add(HunterTree.TreeID, HunterTree);
}

void UNarr_DialogueSystem::CreateElderDialogue()
{
    FNarr_DialogueTree ElderTree;
    ElderTree.TreeID = TEXT("ElderDialogue");

    // Node 0: Wise greeting
    FNarr_DialogueNode Node0;
    Node0.SpeakerName = TEXT("Tribal Elder");
    Node0.DialogueText = TEXT("Young one, I have seen many seasons pass in these ancient lands. The spirits of our ancestors watch over those who show wisdom and courage.");
    Node0.PlayerResponses.Add(TEXT("Tell me about the ancestors."));
    Node0.PlayerResponses.Add(TEXT("What wisdom can you share?"));
    Node0.PlayerResponses.Add(TEXT("I must continue my journey."));
    Node0.NextNodeIndices.Add(1); // Ancestors
    Node0.NextNodeIndices.Add(2); // Wisdom
    Node0.NextNodeIndices.Add(-1); // End
    ElderTree.DialogueNodes.Add(Node0);

    // Node 1: About ancestors
    FNarr_DialogueNode Node1;
    Node1.SpeakerName = TEXT("Tribal Elder");
    Node1.DialogueText = TEXT("Our people have walked these paths for countless generations. They learned to read the signs - the wind, the tracks, the behavior of beasts.");
    Node1.PlayerResponses.Add(TEXT("How do I read these signs?"));
    Node1.PlayerResponses.Add(TEXT("What happened to them?"));
    Node1.NextNodeIndices.Add(2); // Wisdom
    Node1.NextNodeIndices.Add(3); // History
    ElderTree.DialogueNodes.Add(Node1);

    // Node 2: Wisdom
    FNarr_DialogueNode Node2;
    Node2.SpeakerName = TEXT("Tribal Elder");
    Node2.DialogueText = TEXT("Listen to the land, child. When birds fall silent, predators are near. When the ground trembles, the great beasts walk. Nature speaks to those who listen.");
    Node2.PlayerResponses.Add(TEXT("I will remember this wisdom."));
    Node2.NextNodeIndices.Add(-1); // End
    ElderTree.DialogueNodes.Add(Node2);

    // Node 3: History
    FNarr_DialogueNode Node3;
    Node3.SpeakerName = TEXT("Tribal Elder");
    Node3.DialogueText = TEXT("Some were taken by the great beasts, others by the changing seasons. But their knowledge lives on in those who remember their teachings.");
    Node3.PlayerResponses.Add(TEXT("Their memory honors them."));
    Node3.NextNodeIndices.Add(-1); // End
    ElderTree.DialogueNodes.Add(Node3);

    DialogueTrees.Add(ElderTree.TreeID, ElderTree);
}

void UNarr_DialogueSystem::CreateScoutDialogue()
{
    FNarr_DialogueTree ScoutTree;
    ScoutTree.TreeID = TEXT("ScoutDialogue");

    // Node 0: Urgent warning
    FNarr_DialogueNode Node0;
    Node0.SpeakerName = TEXT("Tribal Scout");
    Node0.DialogueText = TEXT("Halt! I bring urgent news from the frontier. The migration patterns have changed - predators are moving into new territories.");
    Node0.PlayerResponses.Add(TEXT("What kind of predators?"));
    Node0.PlayerResponses.Add(TEXT("Where are they heading?"));
    Node0.PlayerResponses.Add(TEXT("How recent is this news?"));
    Node0.NextNodeIndices.Add(1); // Predator types
    Node0.NextNodeIndices.Add(2); // Locations
    Node0.NextNodeIndices.Add(3); // Timing
    ScoutTree.DialogueNodes.Add(Node0);

    // Node 1: Predator types
    FNarr_DialogueNode Node1;
    Node1.SpeakerName = TEXT("Tribal Scout");
    Node1.DialogueText = TEXT("Raptor packs, mainly. Fast, intelligent, and hunting in larger groups than usual. I've also spotted lone T-Rex tracks near the river crossing.");
    Node1.PlayerResponses.Add(TEXT("Why are they moving?"));
    Node1.PlayerResponses.Add(TEXT("Are the usual paths safe?"));
    Node1.NextNodeIndices.Add(4); // Migration reasons
    Node1.NextNodeIndices.Add(2); // Safe paths
    ScoutTree.DialogueNodes.Add(Node1);

    // Node 2: Locations/Safe paths
    FNarr_DialogueNode Node2;
    Node2.SpeakerName = TEXT("Tribal Scout");
    Node2.DialogueText = TEXT("The northern routes are compromised. Stick to the southern ridges if you must travel, but avoid the valley floors entirely.");
    Node2.PlayerResponses.Add(TEXT("Thank you for the warning."));
    Node2.NextNodeIndices.Add(-1); // End
    ScoutTree.DialogueNodes.Add(Node2);

    // Node 3: Timing
    FNarr_DialogueNode Node3;
    Node3.SpeakerName = TEXT("Tribal Scout");
    Node3.DialogueText = TEXT("I witnessed this movement just two days past. The situation changes quickly - what's safe today may be deadly tomorrow.");
    Node3.PlayerResponses.Add(TEXT("I'll be careful. Any other advice?"));
    Node3.NextNodeIndices.Add(4); // More advice
    ScoutTree.DialogueNodes.Add(Node3);

    // Node 4: Migration reasons/More advice
    FNarr_DialogueNode Node4;
    Node4.SpeakerName = TEXT("Tribal Scout");
    Node4.DialogueText = TEXT("The great herds are moving south for the season. Where prey goes, predators follow. Travel light, stay alert, and trust your instincts above all.");
    Node4.PlayerResponses.Add(TEXT("Your counsel is valuable. Stay safe."));
    Node4.NextNodeIndices.Add(-1); // End
    ScoutTree.DialogueNodes.Add(Node4);

    DialogueTrees.Add(ScoutTree.TreeID, ScoutTree);
}

void UNarr_DialogueSystem::ProcessCurrentNode()
{
    if (!bDialogueActive || CurrentTreeID.IsEmpty())
    {
        return;
    }

    const FNarr_DialogueTree& CurrentTree = DialogueTrees[CurrentTreeID];
    if (CurrentTree.DialogueNodes.IsValidIndex(CurrentTree.CurrentNodeIndex))
    {
        const FNarr_DialogueNode& CurrentNode = CurrentTree.DialogueNodes[CurrentTree.CurrentNodeIndex];
        
        // Log current dialogue for debugging
        UE_LOG(LogTemp, Log, TEXT("Dialogue - %s: %s"), *CurrentNode.SpeakerName, *CurrentNode.DialogueText);
        
        // Here you would trigger UI updates, audio playback, etc.
        // For now, we just log the dialogue content
    }
}

void UNarr_DialogueSystem::AdvanceToNextNode(int32 NextIndex)
{
    if (NextIndex == -1)
    {
        EndDialogue();
        return;
    }

    if (!CurrentTreeID.IsEmpty() && DialogueTrees.Contains(CurrentTreeID))
    {
        FNarr_DialogueTree& CurrentTree = DialogueTrees[CurrentTreeID];
        if (CurrentTree.DialogueNodes.IsValidIndex(NextIndex))
        {
            CurrentTree.CurrentNodeIndex = NextIndex;
            ProcessCurrentNode();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid next node index %d"), NextIndex);
            EndDialogue();
        }
    }
}