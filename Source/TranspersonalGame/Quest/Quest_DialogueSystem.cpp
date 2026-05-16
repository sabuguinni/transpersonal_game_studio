#include "Quest_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"

UQuest_DialogueComponent::UQuest_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    InteractionRange = 300.0f;
    bCanInteract = true;
}

void UQuest_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogue();
}

void UQuest_DialogueComponent::InitializeDefaultDialogue()
{
    DialogueTree.TreeID = FString::Printf(TEXT("dialogue_%s"), *GetOwner()->GetName());
    DialogueTree.NPCName = TEXT("Tribal Hunter");
    DialogueTree.CurrentNodeID = TEXT("greeting");
    DialogueTree.bIsActive = false;

    // Create greeting node
    FQuest_DialogueNode GreetingNode;
    GreetingNode.NodeID = TEXT("greeting");
    GreetingNode.SpeakerName = DialogueTree.NPCName;
    GreetingNode.DialogueText = TEXT("Stranger, these lands are dangerous. The great beasts hunt at dusk. What brings you to our territory?");
    GreetingNode.PlayerChoices.Add(TEXT("I seek knowledge of the ancient ways"));
    GreetingNode.PlayerChoices.Add(TEXT("I need supplies for survival"));
    GreetingNode.PlayerChoices.Add(TEXT("I'm just passing through"));
    GreetingNode.NextNodeIDs.Add(TEXT("knowledge"));
    GreetingNode.NextNodeIDs.Add(TEXT("supplies"));
    GreetingNode.NextNodeIDs.Add(TEXT("farewell"));
    DialogueTree.DialogueNodes.Add(GreetingNode);

    // Create knowledge node
    FQuest_DialogueNode KnowledgeNode;
    KnowledgeNode.NodeID = TEXT("knowledge");
    KnowledgeNode.SpeakerName = DialogueTree.NPCName;
    KnowledgeNode.DialogueText = TEXT("The old ways teach us to read the signs. Watch the sky for the great winged ones. Listen for the ground-shakers. Bring me three sharp stones and I will teach you to craft the hunter's spear.");
    KnowledgeNode.PlayerChoices.Add(TEXT("I will find the stones"));
    KnowledgeNode.PlayerChoices.Add(TEXT("Tell me more about the beasts"));
    KnowledgeNode.NextNodeIDs.Add(TEXT("quest_stones"));
    KnowledgeNode.NextNodeIDs.Add(TEXT("beast_lore"));
    DialogueTree.DialogueNodes.Add(KnowledgeNode);

    // Create quest node
    FQuest_DialogueNode QuestNode;
    QuestNode.NodeID = TEXT("quest_stones");
    QuestNode.SpeakerName = DialogueTree.NPCName;
    QuestNode.DialogueText = TEXT("Good. Seek the stones near the water's edge where the earth is soft. Return when you have gathered them.");
    QuestNode.bIsQuestNode = true;
    QuestNode.QuestID = TEXT("gather_sharp_stones");
    QuestNode.PlayerChoices.Add(TEXT("I understand"));
    QuestNode.NextNodeIDs.Add(TEXT("farewell"));
    DialogueTree.DialogueNodes.Add(QuestNode);

    // Create beast lore node
    FQuest_DialogueNode BeastNode;
    BeastNode.NodeID = TEXT("beast_lore");
    BeastNode.SpeakerName = DialogueTree.NPCName;
    BeastNode.DialogueText = TEXT("The three-horns travel in herds but fight alone when cornered. The swift-claws hunt in packs and strike from shadows. The long-necks are gentle unless protecting young. Remember this, and you may survive another day.");
    BeastNode.PlayerChoices.Add(TEXT("Thank you for the wisdom"));
    BeastNode.NextNodeIDs.Add(TEXT("farewell"));
    DialogueTree.DialogueNodes.Add(BeastNode);

    // Create supplies node
    FQuest_DialogueNode SuppliesNode;
    SuppliesNode.NodeID = TEXT("supplies");
    SuppliesNode.SpeakerName = DialogueTree.NPCName;
    SuppliesNode.DialogueText = TEXT("Survival requires more than hope, stranger. I can share dried meat and water, but you must prove your worth. Hunt a small prey and bring me its hide.");
    SuppliesNode.bIsQuestNode = true;
    SuppliesNode.QuestID = TEXT("hunt_small_prey");
    SuppliesNode.PlayerChoices.Add(TEXT("I accept this challenge"));
    SuppliesNode.PlayerChoices.Add(TEXT("This seems too dangerous"));
    SuppliesNode.NextNodeIDs.Add(TEXT("farewell"));
    SuppliesNode.NextNodeIDs.Add(TEXT("farewell"));
    DialogueTree.DialogueNodes.Add(SuppliesNode);

    // Create farewell node
    FQuest_DialogueNode FarewellNode;
    FarewellNode.NodeID = TEXT("farewell");
    FarewellNode.SpeakerName = DialogueTree.NPCName;
    FarewellNode.DialogueText = TEXT("May the spirits guide your path, wanderer. Stay alert and trust your instincts.");
    FarewellNode.PlayerChoices.Add(TEXT("Farewell"));
    FarewellNode.NextNodeIDs.Add(TEXT("end"));
    DialogueTree.DialogueNodes.Add(FarewellNode);

    UE_LOG(LogTemp, Warning, TEXT("Dialogue initialized for %s with %d nodes"), *DialogueTree.NPCName, DialogueTree.DialogueNodes.Num());
}

bool UQuest_DialogueComponent::StartDialogue(AActor* Player)
{
    if (!Player || !bCanInteract)
    {
        return false;
    }

    if (!IsInRange(Player))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player too far for dialogue"));
        return false;
    }

    DialogueTree.bIsActive = true;
    DialogueTree.CurrentNodeID = TEXT("greeting");

    // Register with subsystem
    if (UQuest_DialogueSubsystem* DialogueSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuest_DialogueSubsystem>())
    {
        DialogueSubsystem->RegisterDialogueTree(DialogueTree);
        DialogueSubsystem->StartDialogueWithNPC(DialogueTree.TreeID, Player);
    }

    UE_LOG(LogTemp, Warning, TEXT("Started dialogue with %s"), *DialogueTree.NPCName);
    return true;
}

FQuest_DialogueNode UQuest_DialogueComponent::GetCurrentNode()
{
    FQuest_DialogueNode* Node = FindNodeByID(DialogueTree.CurrentNodeID);
    if (Node)
    {
        return *Node;
    }
    return FQuest_DialogueNode();
}

bool UQuest_DialogueComponent::SelectChoice(int32 ChoiceIndex)
{
    FQuest_DialogueNode* CurrentNode = FindNodeByID(DialogueTree.CurrentNodeID);
    if (!CurrentNode || ChoiceIndex < 0 || ChoiceIndex >= CurrentNode->NextNodeIDs.Num())
    {
        return false;
    }

    FString NextNodeID = CurrentNode->NextNodeIDs[ChoiceIndex];
    if (NextNodeID == TEXT("end"))
    {
        EndDialogue();
        return true;
    }

    DialogueTree.CurrentNodeID = NextNodeID;
    UE_LOG(LogTemp, Warning, TEXT("Dialogue moved to node: %s"), *NextNodeID);
    return true;
}

void UQuest_DialogueComponent::EndDialogue()
{
    DialogueTree.bIsActive = false;
    DialogueTree.CurrentNodeID = TEXT("greeting");

    if (UQuest_DialogueSubsystem* DialogueSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuest_DialogueSubsystem>())
    {
        DialogueSubsystem->EndCurrentDialogue();
    }

    UE_LOG(LogTemp, Warning, TEXT("Dialogue ended with %s"), *DialogueTree.NPCName);
}

bool UQuest_DialogueComponent::IsInRange(AActor* Player)
{
    if (!Player || !GetOwner())
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= InteractionRange;
}

FQuest_DialogueNode* UQuest_DialogueComponent::FindNodeByID(const FString& NodeID)
{
    for (FQuest_DialogueNode& Node : DialogueTree.DialogueNodes)
    {
        if (Node.NodeID == NodeID)
        {
            return &Node;
        }
    }
    return nullptr;
}

// Subsystem Implementation
void UQuest_DialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    CurrentDialogue = nullptr;
    CreateDefaultDialogues();
    UE_LOG(LogTemp, Warning, TEXT("Quest Dialogue Subsystem initialized"));
}

void UQuest_DialogueSubsystem::CreateDefaultDialogues()
{
    // Create elder dialogue
    FQuest_DialogueTree ElderTree;
    ElderTree.TreeID = TEXT("tribal_elder_main");
    ElderTree.NPCName = TEXT("Tribal Elder");
    ElderTree.CurrentNodeID = TEXT("wisdom");

    FQuest_DialogueNode WisdomNode;
    WisdomNode.NodeID = TEXT("wisdom");
    WisdomNode.SpeakerName = TEXT("Tribal Elder");
    WisdomNode.DialogueText = TEXT("Young one, I have walked these lands for many seasons. The great lizards were here before us, and they will remain after. We must learn to live in harmony with their power.");
    WisdomNode.PlayerChoices.Add(TEXT("Teach me the old ways"));
    WisdomNode.PlayerChoices.Add(TEXT("How do we survive the great beasts?"));
    WisdomNode.NextNodeIDs.Add(TEXT("teaching"));
    WisdomNode.NextNodeIDs.Add(TEXT("survival"));
    ElderTree.DialogueNodes.Add(WisdomNode);

    FQuest_DialogueNode TeachingNode;
    TeachingNode.NodeID = TEXT("teaching");
    TeachingNode.SpeakerName = TEXT("Tribal Elder");
    TeachingNode.DialogueText = TEXT("The old ways speak of patience and respect. Observe before you act. Learn the patterns of the hunt. Gather the sacred plants that grow near the ancient stones.");
    TeachingNode.bIsQuestNode = true;
    TeachingNode.QuestID = TEXT("gather_sacred_plants");
    TeachingNode.PlayerChoices.Add(TEXT("I will seek the sacred plants"));
    TeachingNode.NextNodeIDs.Add(TEXT("blessing"));
    ElderTree.DialogueNodes.Add(TeachingNode);

    FQuest_DialogueNode BlessingNode;
    BlessingNode.NodeID = TEXT("blessing");
    BlessingNode.SpeakerName = TEXT("Tribal Elder");
    BlessingNode.DialogueText = TEXT("Go with the blessing of the ancestors. May your path be clear and your spirit strong.");
    BlessingNode.PlayerChoices.Add(TEXT("Thank you, Elder"));
    BlessingNode.NextNodeIDs.Add(TEXT("end"));
    ElderTree.DialogueNodes.Add(BlessingNode);

    ActiveDialogues.Add(ElderTree);
    UE_LOG(LogTemp, Warning, TEXT("Created default dialogue for Tribal Elder"));
}

bool UQuest_DialogueSubsystem::RegisterDialogueTree(const FQuest_DialogueTree& NewTree)
{
    // Check if tree already exists
    for (FQuest_DialogueTree& ExistingTree : ActiveDialogues)
    {
        if (ExistingTree.TreeID == NewTree.TreeID)
        {
            ExistingTree = NewTree; // Update existing
            return true;
        }
    }

    ActiveDialogues.Add(NewTree);
    UE_LOG(LogTemp, Warning, TEXT("Registered dialogue tree: %s"), *NewTree.TreeID);
    return true;
}

FQuest_DialogueTree* UQuest_DialogueSubsystem::GetDialogueByID(const FString& TreeID)
{
    for (FQuest_DialogueTree& Tree : ActiveDialogues)
    {
        if (Tree.TreeID == TreeID)
        {
            return &Tree;
        }
    }
    return nullptr;
}

bool UQuest_DialogueSubsystem::StartDialogueWithNPC(const FString& NPCID, AActor* Player)
{
    FQuest_DialogueTree* Tree = GetDialogueByID(NPCID);
    if (!Tree || !Player)
    {
        return false;
    }

    CurrentDialogue = Tree;
    CurrentDialogue->bIsActive = true;
    UE_LOG(LogTemp, Warning, TEXT("Started dialogue with NPC: %s"), *NPCID);
    return true;
}

void UQuest_DialogueSubsystem::EndCurrentDialogue()
{
    if (CurrentDialogue)
    {
        CurrentDialogue->bIsActive = false;
        CurrentDialogue = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("Ended current dialogue"));
    }
}

bool UQuest_DialogueSubsystem::IsDialogueActive()
{
    return CurrentDialogue != nullptr && CurrentDialogue->bIsActive;
}