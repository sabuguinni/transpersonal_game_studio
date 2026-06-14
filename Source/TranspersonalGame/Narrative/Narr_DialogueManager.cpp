#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    CurrentDialogue = nullptr;
    CurrentNodeID = 0;
    bDialogueActive = false;
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default dialogue trees
    InitializeDefaultDialogues();
}

bool ANarr_DialogueManager::StartDialogue(const FString& TreeName)
{
    FNarr_DialogueTree* FoundTree = FindDialogueTree(TreeName);
    if (!FoundTree)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue tree not found: %s"), *TreeName);
        return false;
    }

    CurrentDialogue = FoundTree;
    CurrentNodeID = FoundTree->RootNodeID;
    bDialogueActive = true;

    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s"), *TreeName);
    return true;
}

bool ANarr_DialogueManager::AdvanceDialogue(int32 ResponseIndex)
{
    if (!bDialogueActive || !CurrentDialogue)
    {
        return false;
    }

    FNarr_DialogueNode* CurrentNode = FindNodeByID(CurrentNodeID);
    if (!CurrentNode)
    {
        EndDialogue();
        return false;
    }

    // Check if this is an end node
    if (CurrentNode->bIsEndNode || CurrentNode->NextNodeIDs.Num() == 0)
    {
        EndDialogue();
        return false;
    }

    // Validate response index
    if (ResponseIndex < 0 || ResponseIndex >= CurrentNode->NextNodeIDs.Num())
    {
        ResponseIndex = 0; // Default to first option
    }

    // Move to next node
    CurrentNodeID = CurrentNode->NextNodeIDs[ResponseIndex];
    
    UE_LOG(LogTemp, Log, TEXT("Advanced dialogue to node: %d"), CurrentNodeID);
    return true;
}

void ANarr_DialogueManager::EndDialogue()
{
    CurrentDialogue = nullptr;
    CurrentNodeID = 0;
    bDialogueActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

FNarr_DialogueLine ANarr_DialogueManager::GetCurrentDialogueLine()
{
    if (!bDialogueActive || !CurrentDialogue)
    {
        return FNarr_DialogueLine();
    }

    FNarr_DialogueNode* CurrentNode = FindNodeByID(CurrentNodeID);
    if (CurrentNode)
    {
        return CurrentNode->DialogueLine;
    }

    return FNarr_DialogueLine();
}

TArray<FString> ANarr_DialogueManager::GetCurrentResponseOptions()
{
    if (!bDialogueActive || !CurrentDialogue)
    {
        return TArray<FString>();
    }

    FNarr_DialogueNode* CurrentNode = FindNodeByID(CurrentNodeID);
    if (CurrentNode)
    {
        return CurrentNode->DialogueLine.ResponseOptions;
    }

    return TArray<FString>();
}

void ANarr_DialogueManager::AddDialogueTree(const FNarr_DialogueTree& NewTree)
{
    DialogueTrees.Add(NewTree);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue tree: %s"), *NewTree.TreeName);
}

FNarr_DialogueTree* ANarr_DialogueManager::FindDialogueTree(const FString& TreeName)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeName == TreeName)
        {
            return &Tree;
        }
    }
    return nullptr;
}

FNarr_DialogueNode* ANarr_DialogueManager::FindNodeByID(int32 NodeID)
{
    if (!CurrentDialogue)
    {
        return nullptr;
    }

    for (FNarr_DialogueNode& Node : CurrentDialogue->Nodes)
    {
        if (Node.NodeID == NodeID)
        {
            return &Node;
        }
    }
    return nullptr;
}

void ANarr_DialogueManager::InitializeDefaultDialogues()
{
    DialogueTrees.Empty();
    
    CreateTribalElderDialogue();
    CreateScoutDialogue();
    CreateHunterDialogue();
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d default dialogue trees"), DialogueTrees.Num());
}

void ANarr_DialogueManager::CreateTribalElderDialogue()
{
    FNarr_DialogueTree ElderTree;
    ElderTree.TreeName = TEXT("TribalElder");
    ElderTree.RootNodeID = 1;

    // Node 1: Elder greeting
    FNarr_DialogueNode Node1;
    Node1.NodeID = 1;
    Node1.DialogueLine.SpeakerName = TEXT("Tribal Elder");
    Node1.DialogueLine.DialogueText = TEXT("The great hunters move through these ancient lands. Stone tools and fire separate us from the beasts. What brings you to our territory?");
    Node1.DialogueLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781435730205_Tribal_Elder.mp3");
    Node1.DialogueLine.Duration = 10.0f;
    Node1.DialogueLine.ResponseOptions.Add(TEXT("I seek knowledge of survival"));
    Node1.DialogueLine.ResponseOptions.Add(TEXT("I need shelter for the night"));
    Node1.DialogueLine.ResponseOptions.Add(TEXT("I must leave immediately"));
    Node1.NextNodeIDs.Add(2);
    Node1.NextNodeIDs.Add(3);
    Node1.NextNodeIDs.Add(4);
    Node1.bIsEndNode = false;

    // Node 2: Knowledge path
    FNarr_DialogueNode Node2;
    Node2.NodeID = 2;
    Node2.DialogueLine.SpeakerName = TEXT("Tribal Elder");
    Node2.DialogueLine.DialogueText = TEXT("Wisdom comes through hardship. Watch the sky for storms, the ground for tracks, and always keep your fire burning. The beasts fear flame above all.");
    Node2.DialogueLine.Duration = 8.0f;
    Node2.bIsEndNode = true;

    // Node 3: Shelter path
    FNarr_DialogueNode Node3;
    Node3.NodeID = 3;
    Node3.DialogueLine.SpeakerName = TEXT("Tribal Elder");
    Node3.DialogueLine.DialogueText = TEXT("Rest by our fire tonight, but be gone by dawn. The pack hunters grow bold when strangers linger. Take this advice: never sleep without a spear in hand.");
    Node3.DialogueLine.Duration = 9.0f;
    Node3.bIsEndNode = true;

    // Node 4: Leave path
    FNarr_DialogueNode Node4;
    Node4.NodeID = 4;
    Node4.DialogueLine.SpeakerName = TEXT("Tribal Elder");
    Node4.DialogueLine.DialogueText = TEXT("Go then, but remember - the valley holds many dangers. Trust your instincts and may the spirits of the hunt protect you.");
    Node4.DialogueLine.Duration = 7.0f;
    Node4.bIsEndNode = true;

    ElderTree.Nodes.Add(Node1);
    ElderTree.Nodes.Add(Node2);
    ElderTree.Nodes.Add(Node3);
    ElderTree.Nodes.Add(Node4);

    DialogueTrees.Add(ElderTree);
}

void ANarr_DialogueManager::CreateScoutDialogue()
{
    FNarr_DialogueTree ScoutTree;
    ScoutTree.TreeName = TEXT("Scout");
    ScoutTree.RootNodeID = 1;

    // Node 1: Scout warning
    FNarr_DialogueNode Node1;
    Node1.NodeID = 1;
    Node1.DialogueLine.SpeakerName = TEXT("Scout");
    Node1.DialogueLine.DialogueText = TEXT("Danger approaches from the north! The pack hunters circle our territory. Have you seen any signs of the Velociraptors?");
    Node1.DialogueLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781435734326_Scout_Warning.mp3");
    Node1.DialogueLine.Duration = 8.0f;
    Node1.DialogueLine.ResponseOptions.Add(TEXT("Yes, I saw tracks near the river"));
    Node1.DialogueLine.ResponseOptions.Add(TEXT("No, but I'll help hunt them"));
    Node1.DialogueLine.ResponseOptions.Add(TEXT("I must warn the others"));
    Node1.NextNodeIDs.Add(2);
    Node1.NextNodeIDs.Add(3);
    Node1.NextNodeIDs.Add(4);
    Node1.bIsEndNode = false;

    // Node 2: Tracks information
    FNarr_DialogueNode Node2;
    Node2.NodeID = 2;
    Node2.DialogueLine.SpeakerName = TEXT("Scout");
    Node2.DialogueLine.DialogueText = TEXT("The river crossing! That's their hunting ground. We must move quickly before they strike our gathering parties. Take this spear and follow me!");
    Node2.DialogueLine.Duration = 7.0f;
    Node2.bIsEndNode = true;

    // Node 3: Join hunt
    FNarr_DialogueNode Node3;
    Node3.NodeID = 3;
    Node3.DialogueLine.SpeakerName = TEXT("Scout");
    Node3.DialogueLine.DialogueText = TEXT("Good! We need every hunter. They move in packs of three, strike fast and retreat. Stay low, use the rocks for cover, and aim for the leader first.");
    Node3.DialogueLine.Duration = 8.0f;
    Node3.bIsEndNode = true;

    // Node 4: Warn others
    FNarr_DialogueNode Node4;
    Node4.NodeID = 4;
    Node4.DialogueLine.SpeakerName = TEXT("Scout");
    Node4.DialogueLine.DialogueText = TEXT("Yes! Alert the camp immediately. Tell them to gather the children and elderly in the center. The hunters will form a protective circle. Go now!");
    Node4.DialogueLine.Duration = 8.0f;
    Node4.bIsEndNode = true;

    ScoutTree.Nodes.Add(Node1);
    ScoutTree.Nodes.Add(Node2);
    ScoutTree.Nodes.Add(Node3);
    ScoutTree.Nodes.Add(Node4);

    DialogueTrees.Add(ScoutTree);
}

void ANarr_DialogueManager::CreateHunterDialogue()
{
    FNarr_DialogueTree HunterTree;
    HunterTree.TreeName = TEXT("Hunter");
    HunterTree.RootNodeID = 1;

    // Node 1: Hunter encounter
    FNarr_DialogueNode Node1;
    Node1.NodeID = 1;
    Node1.DialogueLine.SpeakerName = TEXT("Hunter");
    Node1.DialogueLine.DialogueText = TEXT("You move quietly for an outsider. The great beasts have been restless lately. The Tyrant King stalks the eastern cliffs, and the long-necks flee south. What do you seek in these hunting grounds?");
    Node1.DialogueLine.Duration = 10.0f;
    Node1.DialogueLine.ResponseOptions.Add(TEXT("I seek to learn your hunting ways"));
    Node1.DialogueLine.ResponseOptions.Add(TEXT("I need meat for my journey"));
    Node1.DialogueLine.ResponseOptions.Add(TEXT("I bring warning of danger"));
    Node1.NextNodeIDs.Add(2);
    Node1.NextNodeIDs.Add(3);
    Node1.NextNodeIDs.Add(4);
    Node1.bIsEndNode = false;

    // Node 2: Learn hunting
    FNarr_DialogueNode Node2;
    Node2.NodeID = 2;
    Node2.DialogueLine.SpeakerName = TEXT("Hunter");
    Node2.DialogueLine.DialogueText = TEXT("Patience is the hunter's greatest weapon. Read the wind, follow the dung trails, and never hunt alone. The small ones travel in packs - take one, the others will come for revenge.");
    Node2.DialogueLine.Duration = 9.0f;
    Node2.bIsEndNode = true;

    // Node 3: Need meat
    FNarr_DialogueNode Node3;
    Node3.NodeID = 3;
    Node3.DialogueLine.SpeakerName = TEXT("Hunter");
    Node3.DialogueLine.DialogueText = TEXT("Meat must be earned, not given. But I have extra from yesterday's kill. Take this dried flesh, but remember - next time you must hunt for yourself or starve.");
    Node3.DialogueLine.Duration = 8.0f;
    Node3.bIsEndNode = true;

    // Node 4: Warning
    FNarr_DialogueNode Node4;
    Node4.NodeID = 4;
    Node4.DialogueLine.SpeakerName = TEXT("Hunter");
    Node4.DialogueLine.DialogueText = TEXT("Speak quickly! What danger? If it threatens the hunting grounds, every hunter must know. The survival of the tribe depends on these lands remaining safe.");
    Node4.DialogueLine.Duration = 8.0f;
    Node4.bIsEndNode = true;

    HunterTree.Nodes.Add(Node1);
    HunterTree.Nodes.Add(Node2);
    HunterTree.Nodes.Add(Node3);
    HunterTree.Nodes.Add(Node4);

    DialogueTrees.Add(HunterTree);
}