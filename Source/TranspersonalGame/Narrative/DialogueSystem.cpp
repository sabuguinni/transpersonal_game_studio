#include "DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"

UDialogueSystem::UDialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentTreeIndex = -1;
    CurrentNodeIndex = -1;
    bDialogueActive = false;
}

void UDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
}

void UDialogueSystem::InitializeDefaultDialogues()
{
    // Create survival tutorial dialogue tree
    FNarr_DialogueTree SurvivalTutorial;
    SurvivalTutorial.TreeName = TEXT("SurvivalTutorial");
    SurvivalTutorial.StartNodeID = 0;

    // Node 0: Elder introduction
    FNarr_DialogueNode IntroNode;
    IntroNode.SpeakerName = TEXT("Tribal Elder");
    IntroNode.DialogueText = TEXT("Welcome, young survivor. The ancient valley is dangerous, but knowledge will keep you alive.");
    IntroNode.ResponseOptions.Add(TEXT("Tell me about the dangers"));
    IntroNode.ResponseOptions.Add(TEXT("How do I find food?"));
    IntroNode.NextNodeIDs.Add(1);
    IntroNode.NextNodeIDs.Add(2);
    IntroNode.bIsEndNode = false;
    SurvivalTutorial.DialogueNodes.Add(IntroNode);

    // Node 1: Danger warning
    FNarr_DialogueNode DangerNode;
    DangerNode.SpeakerName = TEXT("Tribal Elder");
    DangerNode.DialogueText = TEXT("The great predators hunt in packs. Listen for their calls and watch for movement in the grass.");
    DangerNode.ResponseOptions.Add(TEXT("What about the Thunder Lizard?"));
    DangerNode.ResponseOptions.Add(TEXT("I understand"));
    DangerNode.NextNodeIDs.Add(3);
    DangerNode.NextNodeIDs.Add(4);
    DangerNode.bIsEndNode = false;
    SurvivalTutorial.DialogueNodes.Add(DangerNode);

    // Node 2: Food advice
    FNarr_DialogueNode FoodNode;
    FoodNode.SpeakerName = TEXT("Tribal Elder");
    FoodNode.DialogueText = TEXT("Follow the herbivore herds to water sources. Gather berries and roots, but test small amounts first.");
    FoodNode.ResponseOptions.Add(TEXT("Thank you for the wisdom"));
    FoodNode.NextNodeIDs.Add(4);
    FoodNode.bIsEndNode = false;
    SurvivalTutorial.DialogueNodes.Add(FoodNode);

    // Node 3: T-Rex warning
    FNarr_DialogueNode TRexNode;
    TRexNode.SpeakerName = TEXT("Tribal Elder");
    TRexNode.DialogueText = TEXT("The Thunder Lizard is the apex predator. When you hear its roar, find shelter immediately. Do not attempt to fight it.");
    TRexNode.ResponseOptions.Add(TEXT("I will remember"));
    TRexNode.NextNodeIDs.Add(4);
    TRexNode.bIsEndNode = false;
    SurvivalTutorial.DialogueNodes.Add(TRexNode);

    // Node 4: Farewell
    FNarr_DialogueNode FarewellNode;
    FarewellNode.SpeakerName = TEXT("Tribal Elder");
    FarewellNode.DialogueText = TEXT("May the spirits of your ancestors guide your path. Survive, and perhaps we will meet again.");
    FarewellNode.bIsEndNode = true;
    SurvivalTutorial.DialogueNodes.Add(FarewellNode);

    DialogueTrees.Add(SurvivalTutorial);

    // Create hunting dialogue tree
    FNarr_DialogueTree HuntingDialogue;
    HuntingDialogue.TreeName = TEXT("HuntingTips");
    HuntingDialogue.StartNodeID = 0;

    // Hunting intro node
    FNarr_DialogueNode HuntIntroNode;
    HuntIntroNode.SpeakerName = TEXT("Wise Hunter");
    HuntIntroNode.DialogueText = TEXT("The hunt requires patience and cunning. What would you learn?");
    HuntIntroNode.ResponseOptions.Add(TEXT("How to track prey"));
    HuntIntroNode.ResponseOptions.Add(TEXT("Weapon crafting"));
    HuntIntroNode.NextNodeIDs.Add(1);
    HuntIntroNode.NextNodeIDs.Add(2);
    HuntIntroNode.bIsEndNode = false;
    HuntingDialogue.DialogueNodes.Add(HuntIntroNode);

    // Tracking node
    FNarr_DialogueNode TrackingNode;
    TrackingNode.SpeakerName = TEXT("Wise Hunter");
    TrackingNode.DialogueText = TEXT("Look for broken branches, footprints in mud, and disturbed vegetation. The wind carries scents - use it wisely.");
    TrackingNode.ResponseOptions.Add(TEXT("What about predator signs?"));
    TrackingNode.NextNodeIDs.Add(3);
    TrackingNode.bIsEndNode = false;
    HuntingDialogue.DialogueNodes.Add(TrackingNode);

    // Weapon crafting node
    FNarr_DialogueNode WeaponNode;
    WeaponNode.SpeakerName = TEXT("Wise Hunter");
    WeaponNode.DialogueText = TEXT("Sharp stones make good spear tips. Bind them with plant fibers. A well-crafted spear can save your life.");
    WeaponNode.ResponseOptions.Add(TEXT("Thank you for the knowledge"));
    WeaponNode.NextNodeIDs.Add(4);
    WeaponNode.bIsEndNode = false;
    HuntingDialogue.DialogueNodes.Add(WeaponNode);

    // Predator warning node
    FNarr_DialogueNode PredatorNode;
    PredatorNode.SpeakerName = TEXT("Wise Hunter");
    PredatorNode.DialogueText = TEXT("Claw marks on trees, territorial scent markings, and silence where birds should sing - these warn of nearby predators.");
    PredatorNode.ResponseOptions.Add(TEXT("I will watch for these signs"));
    PredatorNode.NextNodeIDs.Add(4);
    PredatorNode.bIsEndNode = false;
    HuntingDialogue.DialogueNodes.Add(PredatorNode);

    // Hunt farewell
    FNarr_DialogueNode HuntFarewellNode;
    HuntFarewellNode.SpeakerName = TEXT("Wise Hunter");
    HuntFarewellNode.DialogueText = TEXT("Hunt with honor, take only what you need, and respect the balance of nature.");
    HuntFarewellNode.bIsEndNode = true;
    HuntingDialogue.DialogueNodes.Add(HuntFarewellNode);

    DialogueTrees.Add(HuntingDialogue);
}

bool UDialogueSystem::StartDialogue(const FString& TreeName)
{
    int32 TreeIndex = FindTreeByName(TreeName);
    if (TreeIndex == -1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue tree not found: %s"), *TreeName);
        return false;
    }

    CurrentTreeIndex = TreeIndex;
    CurrentNodeIndex = DialogueTrees[TreeIndex].StartNodeID;
    bDialogueActive = true;

    UE_LOG(LogTemp, Log, TEXT("Started dialogue tree: %s"), *TreeName);
    return true;
}

void UDialogueSystem::EndDialogue()
{
    CurrentTreeIndex = -1;
    CurrentNodeIndex = -1;
    bDialogueActive = false;
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

FNarr_DialogueNode UDialogueSystem::GetCurrentNode()
{
    if (!bDialogueActive || CurrentTreeIndex == -1 || CurrentNodeIndex == -1)
    {
        return FNarr_DialogueNode();
    }

    if (DialogueTrees.IsValidIndex(CurrentTreeIndex) && 
        DialogueTrees[CurrentTreeIndex].DialogueNodes.IsValidIndex(CurrentNodeIndex))
    {
        return DialogueTrees[CurrentTreeIndex].DialogueNodes[CurrentNodeIndex];
    }

    return FNarr_DialogueNode();
}

bool UDialogueSystem::SelectResponse(int32 ResponseIndex)
{
    if (!bDialogueActive || CurrentTreeIndex == -1 || CurrentNodeIndex == -1)
    {
        return false;
    }

    FNarr_DialogueNode CurrentNode = GetCurrentNode();
    if (!CurrentNode.NextNodeIDs.IsValidIndex(ResponseIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid response index: %d"), ResponseIndex);
        return false;
    }

    CurrentNodeIndex = CurrentNode.NextNodeIDs[ResponseIndex];
    
    // Check if we've reached an end node
    FNarr_DialogueNode NextNode = GetCurrentNode();
    if (NextNode.bIsEndNode)
    {
        UE_LOG(LogTemp, Log, TEXT("Reached end node, dialogue will end"));
    }

    return true;
}

void UDialogueSystem::AddDialogueTree(const FNarr_DialogueTree& NewTree)
{
    DialogueTrees.Add(NewTree);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue tree: %s"), *NewTree.TreeName);
}

TArray<FString> UDialogueSystem::GetAvailableDialogueTrees()
{
    TArray<FString> TreeNames;
    for (const FNarr_DialogueTree& Tree : DialogueTrees)
    {
        TreeNames.Add(Tree.TreeName);
    }
    return TreeNames;
}

void UDialogueSystem::LoadDialogueFromDataTable(UDataTable* DialogueDataTable)
{
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueDataTable is null"));
        return;
    }

    // Implementation for loading from data table would go here
    UE_LOG(LogTemp, Log, TEXT("Loading dialogue from data table"));
}

int32 UDialogueSystem::FindTreeByName(const FString& TreeName)
{
    for (int32 i = 0; i < DialogueTrees.Num(); i++)
    {
        if (DialogueTrees[i].TreeName == TreeName)
        {
            return i;
        }
    }
    return -1;
}