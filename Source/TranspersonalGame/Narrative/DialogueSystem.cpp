#include "DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../Quest/QuestManager.h"

// UDialogueDataAsset Implementation
FNarr_DialogueTree UDialogueDataAsset::GetDialogueTree(const FString& DialogueID) const
{
    for (const FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.DialogueID == DialogueID)
        {
            return Tree;
        }
    }
    return FNarr_DialogueTree();
}

bool UDialogueDataAsset::HasDialogueTree(const FString& DialogueID) const
{
    for (const FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.DialogueID == DialogueID)
        {
            return true;
        }
    }
    return false;
}

// UDialogueComponent Implementation
UDialogueComponent::UDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    DialogueData = nullptr;
    DefaultDialogueID = TEXT("default_greeting");
    NPCType = ENarr_NPCType::Tribal_Elder;
    InteractionRange = 300.0f;
    
    bIsDialogueActive = false;
    CurrentPlayer = nullptr;
    CurrentDialogueID = TEXT("");
    CurrentNodeID = TEXT("");
}

bool UDialogueComponent::StartDialogue(const FString& DialogueID, AActor* PlayerActor)
{
    if (!DialogueData || !PlayerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Cannot start dialogue - missing data or player"));
        return false;
    }

    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Dialogue already active"));
        return false;
    }

    FNarr_DialogueTree DialogueTree = DialogueData->GetDialogueTree(DialogueID);
    if (DialogueTree.DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Dialogue tree not found: %s"), *DialogueID);
        return false;
    }

    CurrentDialogueID = DialogueID;
    CurrentNodeID = DialogueTree.StartNodeID;
    CurrentPlayer = PlayerActor;
    bIsDialogueActive = true;

    // Notify dialogue manager
    if (UDialogueManager* DialogueManager = GetWorld()->GetGameInstance()->GetSubsystem<UDialogueManager>())
    {
        DialogueManager->OnDialogueStarted.Broadcast(DialogueID);
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Started dialogue %s"), *DialogueID);
    return true;
}

bool UDialogueComponent::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return false;
    }

    FString EndedDialogueID = CurrentDialogueID;
    
    CurrentDialogueID = TEXT("");
    CurrentNodeID = TEXT("");
    CurrentPlayer = nullptr;
    bIsDialogueActive = false;

    // Notify dialogue manager
    if (UDialogueManager* DialogueManager = GetWorld()->GetGameInstance()->GetSubsystem<UDialogueManager>())
    {
        DialogueManager->OnDialogueEnded.Broadcast(EndedDialogueID);
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Ended dialogue %s"), *EndedDialogueID);
    return true;
}

bool UDialogueComponent::SelectChoice(int32 ChoiceIndex)
{
    if (!bIsDialogueActive || !DialogueData)
    {
        return false;
    }

    FNarr_DialogueNode CurrentNode = GetCurrentNode();
    if (CurrentNode.NodeID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Current node not found"));
        return false;
    }

    if (!CurrentNode.Choices.IsValidIndex(ChoiceIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Invalid choice index %d"), ChoiceIndex);
        return false;
    }

    const FNarr_DialogueChoice& SelectedChoice = CurrentNode.Choices[ChoiceIndex];
    
    // Check if choice is available
    if (!IsChoiceAvailable(SelectedChoice))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Choice not available"));
        return false;
    }

    // Notify dialogue manager
    if (UDialogueManager* DialogueManager = GetWorld()->GetGameInstance()->GetSubsystem<UDialogueManager>())
    {
        DialogueManager->OnDialogueChoiceSelected.Broadcast(CurrentDialogueID, ChoiceIndex);
    }

    // Move to next node or end dialogue
    if (SelectedChoice.NextNodeID.IsEmpty())
    {
        return EndDialogue();
    }
    else
    {
        CurrentNodeID = SelectedChoice.NextNodeID;
        
        // Check if new node is an end node
        FNarr_DialogueNode NextNode = FindNodeByID(CurrentNodeID);
        if (NextNode.bIsEndNode)
        {
            return EndDialogue();
        }
    }

    return true;
}

FNarr_DialogueNode UDialogueComponent::GetCurrentNode() const
{
    if (!DialogueData || CurrentNodeID.IsEmpty())
    {
        return FNarr_DialogueNode();
    }

    return FindNodeByID(CurrentNodeID);
}

bool UDialogueComponent::IsInDialogue() const
{
    return bIsDialogueActive;
}

bool UDialogueComponent::CanInteract(AActor* PlayerActor) const
{
    if (!PlayerActor || !GetOwner())
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    return Distance <= InteractionRange;
}

bool UDialogueComponent::IsChoiceAvailable(const FNarr_DialogueChoice& Choice) const
{
    if (!CurrentPlayer)
    {
        return false;
    }

    // Check quest requirements
    if (Choice.RequiredQuests.Num() > 0)
    {
        if (UQuestManager* QuestManager = GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>())
        {
            for (const FString& QuestID : Choice.RequiredQuests)
            {
                if (!QuestManager->IsQuestCompleted(QuestID))
                {
                    return false;
                }
            }
        }
    }

    // Check item requirements
    if (Choice.bRequiresItem && !Choice.RequiredItemType.IsEmpty())
    {
        // TODO: Implement inventory system check
        // For now, assume item is available
    }

    return true;
}

FNarr_DialogueNode UDialogueComponent::FindNodeByID(const FString& NodeID) const
{
    if (!DialogueData)
    {
        return FNarr_DialogueNode();
    }

    FNarr_DialogueTree DialogueTree = DialogueData->GetDialogueTree(CurrentDialogueID);
    for (const FNarr_DialogueNode& Node : DialogueTree.Nodes)
    {
        if (Node.NodeID == NodeID)
        {
            return Node;
        }
    }

    return FNarr_DialogueNode();
}

// UDialogueManager Implementation
UDialogueManager::UDialogueManager()
{
}

void UDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Initialized with %d dialogue data assets"), DialogueDataAssets.Num());
}

void UDialogueManager::Deinitialize()
{
    DialogueDataAssets.Empty();
    
    Super::Deinitialize();
}

void UDialogueManager::RegisterDialogueData(UDialogueDataAsset* DialogueData)
{
    if (!DialogueData)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Cannot register null dialogue data"));
        return;
    }

    FString AssetName = DialogueData->GetName();
    DialogueDataAssets.Add(AssetName, DialogueData);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Registered dialogue data asset: %s"), *AssetName);
}

UDialogueDataAsset* UDialogueManager::GetDialogueData(const FString& DataAssetName) const
{
    if (UDialogueDataAsset* const* FoundData = DialogueDataAssets.Find(DataAssetName))
    {
        return *FoundData;
    }
    return nullptr;
}

void UDialogueManager::CreateSurvivalDialogue(const FString& DialogueID, ENarr_NPCType NPCType, const FString& QuestID)
{
    // Create a basic survival dialogue tree
    FNarr_DialogueTree NewDialogue;
    NewDialogue.DialogueID = DialogueID;
    NewDialogue.NPCType = NPCType;
    NewDialogue.bRepeatable = true;
    NewDialogue.StartNodeID = TEXT("greeting");

    // Create greeting node
    FNarr_DialogueNode GreetingNode;
    GreetingNode.NodeID = TEXT("greeting");
    GreetingNode.bIsEndNode = false;

    switch (NPCType)
    {
        case ENarr_NPCType::Tribal_Elder:
            GreetingNode.SpeakerName = FText::FromString(TEXT("Tribal Elder"));
            GreetingNode.DialogueText = FText::FromString(TEXT("Young one, the wilderness tests us all. What wisdom do you seek?"));
            NewDialogue.DialogueTitle = FText::FromString(TEXT("Elder's Wisdom"));
            break;
            
        case ENarr_NPCType::Experienced_Hunter:
            GreetingNode.SpeakerName = FText::FromString(TEXT("Hunter"));
            GreetingNode.DialogueText = FText::FromString(TEXT("The hunt calls, survivor. Are you ready to face the beasts?"));
            NewDialogue.DialogueTitle = FText::FromString(TEXT("Hunter's Challenge"));
            break;
            
        case ENarr_NPCType::Wise_Explorer:
            GreetingNode.SpeakerName = FText::FromString(TEXT("Explorer"));
            GreetingNode.DialogueText = FText::FromString(TEXT("The paths ahead are treacherous. Let me share what I have learned."));
            NewDialogue.DialogueTitle = FText::FromString(TEXT("Explorer's Knowledge"));
            break;
            
        default:
            GreetingNode.SpeakerName = FText::FromString(TEXT("Survivor"));
            GreetingNode.DialogueText = FText::FromString(TEXT("Stay strong. The world is harsh, but we endure."));
            NewDialogue.DialogueTitle = FText::FromString(TEXT("Survivor's Words"));
            break;
    }

    // Add choices
    FNarr_DialogueChoice LearnChoice;
    LearnChoice.ChoiceText = FText::FromString(TEXT("Tell me about the dangers here."));
    LearnChoice.NextNodeID = TEXT("dangers");

    FNarr_DialogueChoice QuestChoice;
    QuestChoice.ChoiceText = FText::FromString(TEXT("Do you have work for me?"));
    QuestChoice.NextNodeID = TEXT("quest_offer");

    FNarr_DialogueChoice FarewellChoice;
    FarewellChoice.ChoiceText = FText::FromString(TEXT("I must go."));
    FarewellChoice.NextNodeID = TEXT("");

    GreetingNode.Choices.Add(LearnChoice);
    GreetingNode.Choices.Add(QuestChoice);
    GreetingNode.Choices.Add(FarewellChoice);

    NewDialogue.Nodes.Add(GreetingNode);

    // Create dangers node
    FNarr_DialogueNode DangersNode;
    DangersNode.NodeID = TEXT("dangers");
    DangersNode.SpeakerName = GreetingNode.SpeakerName;
    DangersNode.DialogueText = FText::FromString(TEXT("The great beasts rule these lands. Thunder Walkers in the valleys, Swift Death in the forests, and the Gentle Giants on the plains. Each has their territory and their ways."));
    DangersNode.bIsEndNode = true;

    NewDialogue.Nodes.Add(DangersNode);

    // Create quest offer node
    FNarr_DialogueNode QuestNode;
    QuestNode.NodeID = TEXT("quest_offer");
    QuestNode.SpeakerName = GreetingNode.SpeakerName;
    QuestNode.DialogueText = FText::FromString(TEXT("There is always work for those brave enough. Prove yourself, and greater challenges await."));
    QuestNode.bIsEndNode = true;

    NewDialogue.Nodes.Add(QuestNode);

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Created survival dialogue %s for NPC type %d"), *DialogueID, (int32)NPCType);
}

void UDialogueManager::CreateQuestDialogue(const FString& DialogueID, const FString& QuestID, ENarr_NPCType NPCType)
{
    CreateSurvivalDialogue(DialogueID, NPCType, QuestID);
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Created quest dialogue %s for quest %s"), *DialogueID, *QuestID);
}

void UDialogueManager::InitializeDefaultDialogues()
{
    CreateElderDialogues();
    CreateHunterDialogues();
    CreateExplorerDialogues();
}

void UDialogueManager::CreateElderDialogues()
{
    CreateSurvivalDialogue(TEXT("elder_default"), ENarr_NPCType::Tribal_Elder);
    CreateQuestDialogue(TEXT("elder_gathering_quest"), TEXT("gather_basic_materials"), ENarr_NPCType::Tribal_Elder);
}

void UDialogueManager::CreateHunterDialogues()
{
    CreateSurvivalDialogue(TEXT("hunter_default"), ENarr_NPCType::Experienced_Hunter);
    CreateQuestDialogue(TEXT("hunter_combat_quest"), TEXT("hunt_small_prey"), ENarr_NPCType::Experienced_Hunter);
}

void UDialogueManager::CreateExplorerDialogues()
{
    CreateSurvivalDialogue(TEXT("explorer_default"), ENarr_NPCType::Wise_Explorer);
    CreateQuestDialogue(TEXT("explorer_territory_quest"), TEXT("explore_safe_territory"), ENarr_NPCType::Wise_Explorer);
}