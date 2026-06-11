#include "Quest_DialogueManager.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UQuest_DialogueManager::UQuest_DialogueManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    bDialogueActive = false;
    CurrentDialogueTree = nullptr;
    DialogueAudioVolume = 1.0f;
    DialogueSpeed = 1.0f;
}

void UQuest_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
}

void UQuest_DialogueManager::InitializeDefaultDialogues()
{
    CreateSurvivalDialogues();
    UE_LOG(LogTemp, Log, TEXT("Quest_DialogueManager: Default dialogues initialized"));
}

bool UQuest_DialogueManager::StartDialogue(const FString& TreeID)
{
    if (bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already active, ending current dialogue"));
        EndDialogue();
    }

    for (FQuest_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID)
        {
            CurrentDialogueTree = &Tree;
            CurrentDialogueTree->CurrentNodeIndex = 0;
            CurrentDialogueTree->bIsActive = true;
            bDialogueActive = true;
            
            UE_LOG(LogTemp, Log, TEXT("Started dialogue tree: %s"), *TreeID);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Dialogue tree not found: %s"), *TreeID);
    return false;
}

void UQuest_DialogueManager::EndDialogue()
{
    if (CurrentDialogueTree)
    {
        CurrentDialogueTree->bIsActive = false;
        CurrentDialogueTree = nullptr;
    }
    bDialogueActive = false;
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

bool UQuest_DialogueManager::SelectPlayerResponse(int32 ResponseIndex)
{
    if (!bDialogueActive || !CurrentDialogueTree)
    {
        return false;
    }

    if (CurrentDialogueTree->CurrentNodeIndex >= CurrentDialogueTree->DialogueNodes.Num())
    {
        return false;
    }

    FQuest_DialogueNode& CurrentNode = CurrentDialogueTree->DialogueNodes[CurrentDialogueTree->CurrentNodeIndex];
    
    if (ResponseIndex < 0 || ResponseIndex >= CurrentNode.NextNodeIndices.Num())
    {
        return false;
    }

    int32 NextNodeIndex = CurrentNode.NextNodeIndices[ResponseIndex];
    return AdvanceToNextNode(NextNodeIndex);
}

bool UQuest_DialogueManager::AdvanceToNextNode(int32 NextNodeIndex)
{
    if (!CurrentDialogueTree)
    {
        return false;
    }

    if (NextNodeIndex < 0 || NextNodeIndex >= CurrentDialogueTree->DialogueNodes.Num())
    {
        EndDialogue();
        return false;
    }

    CurrentDialogueTree->CurrentNodeIndex = NextNodeIndex;
    
    // Check if this node triggers a quest
    FQuest_DialogueNode& NewNode = CurrentDialogueTree->DialogueNodes[NextNodeIndex];
    if (NewNode.bIsQuestDialogue && !NewNode.QuestID.IsEmpty())
    {
        TriggerQuestFromDialogue(NewNode.QuestID);
    }

    return true;
}

FQuest_DialogueNode UQuest_DialogueManager::GetCurrentDialogueNode() const
{
    if (!bDialogueActive || !CurrentDialogueTree)
    {
        return FQuest_DialogueNode();
    }

    if (CurrentDialogueTree->CurrentNodeIndex >= CurrentDialogueTree->DialogueNodes.Num())
    {
        return FQuest_DialogueNode();
    }

    return CurrentDialogueTree->DialogueNodes[CurrentDialogueTree->CurrentNodeIndex];
}

TArray<FString> UQuest_DialogueManager::GetCurrentPlayerResponses() const
{
    FQuest_DialogueNode CurrentNode = GetCurrentDialogueNode();
    return CurrentNode.PlayerResponses;
}

void UQuest_DialogueManager::CreateSurvivalDialogues()
{
    CreateHunterDialogue();
    CreateCrafterDialogue();
    CreateSurvivalGuideDialogue();
}

void UQuest_DialogueManager::CreateHunterDialogue()
{
    FQuest_DialogueTree HunterTree;
    HunterTree.TreeID = TEXT("Hunter_Survival");

    // Node 0: Initial greeting
    FQuest_DialogueNode Node0;
    Node0.SpeakerName = TEXT("Experienced Hunter");
    Node0.DialogueText = TEXT("Greetings, newcomer. I see you're struggling to survive in these dangerous lands. The great beasts won't show mercy.");
    Node0.PlayerResponses.Add(TEXT("Can you teach me to hunt?"));
    Node0.PlayerResponses.Add(TEXT("What's the biggest threat here?"));
    Node0.NextNodeIndices.Add(1);
    Node0.NextNodeIndices.Add(2);
    HunterTree.DialogueNodes.Add(Node0);

    // Node 1: Teaching to hunt
    FQuest_DialogueNode Node1;
    Node1.SpeakerName = TEXT("Experienced Hunter");
    Node1.DialogueText = TEXT("Hunting requires patience and skill. First, you need proper tools. Craft a stone spear, then track the smaller prey before attempting the giants.");
    Node1.PlayerResponses.Add(TEXT("I'll craft the tools first"));
    Node1.PlayerResponses.Add(TEXT("Where can I find small prey?"));
    Node1.NextNodeIndices.Add(3);
    Node1.NextNodeIndices.Add(4);
    Node1.bIsQuestDialogue = true;
    Node1.QuestID = TEXT("HUNT_SMALL_PREY");
    HunterTree.DialogueNodes.Add(Node1);

    // Node 2: Biggest threat
    FQuest_DialogueNode Node2;
    Node2.SpeakerName = TEXT("Experienced Hunter");
    Node2.DialogueText = TEXT("The Tyrannosaurus Rex rules these lands. But don't underestimate the pack hunters - Velociraptors hunt in groups and are cunning.");
    Node2.PlayerResponses.Add(TEXT("How do I defend against them?"));
    Node2.NextNodeIndices.Add(5);
    HunterTree.DialogueNodes.Add(Node2);

    // Node 3: Crafting focus
    FQuest_DialogueNode Node3;
    Node3.SpeakerName = TEXT("Experienced Hunter");
    Node3.DialogueText = TEXT("Smart choice. Gather stones and sturdy branches. The crafting area near the river has good materials.");
    Node3.PlayerResponses.Add(TEXT("Thank you for the advice"));
    Node3.NextNodeIndices.Add(-1); // End dialogue
    HunterTree.DialogueNodes.Add(Node3);

    // Node 4: Small prey location
    FQuest_DialogueNode Node4;
    Node4.SpeakerName = TEXT("Experienced Hunter");
    Node4.DialogueText = TEXT("The forest edge has small creatures. But be careful - even small prey can be dangerous if you're unprepared.");
    Node4.PlayerResponses.Add(TEXT("I'll be cautious"));
    Node4.NextNodeIndices.Add(-1);
    HunterTree.DialogueNodes.Add(Node4);

    // Node 5: Defense advice
    FQuest_DialogueNode Node5;
    Node5.SpeakerName = TEXT("Experienced Hunter");
    Node5.DialogueText = TEXT("Build shelter first. High ground gives you advantage. Fire keeps most beasts away at night. Never travel alone in raptor territory.");
    Node5.PlayerResponses.Add(TEXT("I'll remember that"));
    Node5.NextNodeIndices.Add(-1);
    HunterTree.DialogueNodes.Add(Node5);

    DialogueTrees.Add(HunterTree);
}

void UQuest_DialogueManager::CreateCrafterDialogue()
{
    FQuest_DialogueTree CrafterTree;
    CrafterTree.TreeID = TEXT("Crafter_Survival");

    FQuest_DialogueNode Node0;
    Node0.SpeakerName = TEXT("Skilled Crafter");
    Node0.DialogueText = TEXT("Ah, another survivor! Tools are life in this harsh world. Without proper equipment, you won't last a day against the great lizards.");
    Node0.PlayerResponses.Add(TEXT("What tools do I need first?"));
    Node0.PlayerResponses.Add(TEXT("Can you teach me to craft?"));
    Node0.NextNodeIndices.Add(1);
    Node0.NextNodeIndices.Add(2);
    CrafterTree.DialogueNodes.Add(Node0);

    FQuest_DialogueNode Node1;
    Node1.SpeakerName = TEXT("Skilled Crafter");
    Node1.DialogueText = TEXT("Start with basics: stone axe for cutting, spear for hunting, and fire-making tools. The axe requires two good stones and a strong branch.");
    Node1.PlayerResponses.Add(TEXT("Where do I find the best stones?"));
    Node1.NextNodeIndices.Add(3);
    Node1.bIsQuestDialogue = true;
    Node1.QuestID = TEXT("CRAFT_STONE_AXE");
    CrafterTree.DialogueNodes.Add(Node1);

    FQuest_DialogueNode Node2;
    Node2.SpeakerName = TEXT("Skilled Crafter");
    Node2.DialogueText = TEXT("Crafting is about understanding materials. Each stone, each branch has its purpose. Practice makes perfect, but mistakes can be deadly here.");
    Node2.PlayerResponses.Add(TEXT("I'll practice carefully"));
    Node2.NextNodeIndices.Add(-1);
    CrafterTree.DialogueNodes.Add(Node2);

    FQuest_DialogueNode Node3;
    Node3.SpeakerName = TEXT("Skilled Crafter");
    Node3.DialogueText = TEXT("The rocky outcrops near the water have the hardest stones. But beware - predators also drink there. Go armed or go with others.");
    Node3.PlayerResponses.Add(TEXT("Thanks for the warning"));
    Node3.NextNodeIndices.Add(-1);
    CrafterTree.DialogueNodes.Add(Node3);

    DialogueTrees.Add(CrafterTree);
}

void UQuest_DialogueManager::CreateSurvivalGuideDialogue()
{
    FQuest_DialogueTree GuideTree;
    GuideTree.TreeID = TEXT("Guide_Survival");

    FQuest_DialogueNode Node0;
    Node0.SpeakerName = TEXT("Survival Guide");
    Node0.DialogueText = TEXT("Welcome to the prehistoric world, survivor. Every day here is a test of will, skill, and luck. The ancient beasts show no mercy to the unprepared.");
    Node0.PlayerResponses.Add(TEXT("What's the key to survival?"));
    Node0.PlayerResponses.Add(TEXT("How do I avoid the big predators?"));
    Node0.NextNodeIndices.Add(1);
    Node0.NextNodeIndices.Add(2);
    GuideTree.DialogueNodes.Add(Node0);

    FQuest_DialogueNode Node1;
    Node1.SpeakerName = TEXT("Survival Guide");
    Node1.DialogueText = TEXT("Three pillars: Shelter, Tools, and Knowledge. Build high, craft smart, learn the patterns of the great beasts. Respect this world or it will claim you.");
    Node1.PlayerResponses.Add(TEXT("I'll build shelter first"));
    Node1.NextNodeIndices.Add(3);
    Node1.bIsQuestDialogue = true;
    Node1.QuestID = TEXT("BUILD_SHELTER");
    GuideTree.DialogueNodes.Add(Node1);

    FQuest_DialogueNode Node2;
    Node2.SpeakerName = TEXT("Survival Guide");
    Node2.DialogueText = TEXT("Stay downwind, move quietly, know their territories. T-Rex has poor eyesight but excellent smell. Raptors hunt in packs - if you see one, there are others.");
    Node2.PlayerResponses.Add(TEXT("Any safe areas?"));
    Node2.NextNodeIndices.Add(4);
    GuideTree.DialogueNodes.Add(Node2);

    FQuest_DialogueNode Node3;
    Node3.SpeakerName = TEXT("Survival Guide");
    Node3.DialogueText = TEXT("Wise choice. High ground with multiple escape routes. Near water but not too close - predators hunt at water sources. Build before nightfall.");
    Node3.PlayerResponses.Add(TEXT("I'll start immediately"));
    Node3.NextNodeIndices.Add(-1);
    GuideTree.DialogueNodes.Add(Node3);

    FQuest_DialogueNode Node4;
    Node4.SpeakerName = TEXT("Survival Guide");
    Node4.DialogueText = TEXT("Nowhere is truly safe, but the elevated areas with good visibility offer the best chance. Always have an escape plan. This world belongs to them, not us.");
    Node4.PlayerResponses.Add(TEXT("I understand"));
    Node4.NextNodeIndices.Add(-1);
    GuideTree.DialogueNodes.Add(Node4);

    DialogueTrees.Add(GuideTree);
}

void UQuest_DialogueManager::TriggerQuestFromDialogue(const FString& QuestID)
{
    UE_LOG(LogTemp, Log, TEXT("Triggering quest from dialogue: %s"), *QuestID);
    
    // This would integrate with the Quest Manager to start the specified quest
    // For now, we log the quest trigger
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Quest Triggered: %s"), *QuestID));
    }
}

void UQuest_DialogueManager::PlayDialogueAudio(const FString& DialogueText, const FString& SpeakerName)
{
    // This would integrate with the audio system to play TTS or recorded dialogue
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio for %s: %s"), *SpeakerName, *DialogueText);
}