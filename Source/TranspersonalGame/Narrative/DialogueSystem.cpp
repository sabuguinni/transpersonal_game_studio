// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue Agent
// Prehistoric survival dialogue system — NO spiritual content

#include "DialogueSystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ============================================================
// UNarr_DialogueComponent
// ============================================================

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bConversationActive = false;
    CurrentNodeID = 0;
    bHasMetPlayer = false;
    InteractionRadius = 300.0f;

    EmptyNode.NodeID = -1;
    EmptyNode.bIsEndNode = true;
    EmptyNode.NPCSpeech = FText::FromString(TEXT("..."));
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    // Auto-load dialogue tree if NPCID is set
    if (!DialogueTree.NPCID.IsNone())
    {
        LoadDialogueTree(DialogueTree.NPCID);
    }
}

void UNarr_DialogueComponent::LoadDialogueTree(FName InNPCID)
{
    DialogueTree.NPCID = InNPCID;

    if (InNPCID == FName("Elder_Kael"))
    {
        BuildElderKaelTree();
    }
    else if (InNPCID == FName("Hunter_Dara"))
    {
        BuildHunterDaraTree();
    }
    else if (InNPCID == FName("Scout_Mira"))
    {
        BuildScoutMiraTree();
    }
    else if (InNPCID == FName("Scout_Renn"))
    {
        BuildScoutRennTree();
    }
}

FNarr_DialogueNode UNarr_DialogueComponent::StartConversation()
{
    bConversationActive = true;
    CurrentNodeID = DialogueTree.RootNodeID;

    FNarr_DialogueNode* Node = FindNode(CurrentNodeID);
    if (!Node)
    {
        return EmptyNode;
    }

    // First meeting uses a different opening if available
    if (!bHasMetPlayer)
    {
        bHasMetPlayer = true;
    }

    return *Node;
}

FNarr_DialogueNode UNarr_DialogueComponent::SelectChoice(int32 ChoiceIndex)
{
    if (!bConversationActive)
    {
        return EmptyNode;
    }

    FNarr_DialogueNode* CurrentNode = FindNode(CurrentNodeID);
    if (!CurrentNode)
    {
        return EmptyNode;
    }

    if (!CurrentNode->PlayerChoices.IsValidIndex(ChoiceIndex))
    {
        return EmptyNode;
    }

    const FNarr_DialogueChoice& Choice = CurrentNode->PlayerChoices[ChoiceIndex];

    // Handle outcome
    if (Choice.Outcome == ENarr_DialogueOutcome::EndConversation)
    {
        EndConversation();
        return EmptyNode;
    }

    // Advance to next node
    if (Choice.NextNodeID < 0)
    {
        EndConversation();
        return EmptyNode;
    }

    CurrentNodeID = Choice.NextNodeID;
    FNarr_DialogueNode* NextNode = FindNode(CurrentNodeID);
    if (!NextNode)
    {
        EndConversation();
        return EmptyNode;
    }

    if (NextNode->bIsEndNode)
    {
        bConversationActive = false;
    }

    return *NextNode;
}

void UNarr_DialogueComponent::EndConversation()
{
    bConversationActive = false;
    CurrentNodeID = DialogueTree.RootNodeID;
}

FNarr_DialogueNode UNarr_DialogueComponent::GetCurrentNode() const
{
    for (const FNarr_DialogueNode& Node : DialogueTree.Nodes)
    {
        if (Node.NodeID == CurrentNodeID)
        {
            return Node;
        }
    }
    return EmptyNode;
}

bool UNarr_DialogueComponent::IsPlayerInRange(AActor* Player) const
{
    if (!Player || !GetOwner())
    {
        return false;
    }
    float Distance = FVector::Dist(Player->GetActorLocation(), GetOwner()->GetActorLocation());
    return Distance <= InteractionRadius;
}

FNarr_DialogueNode* UNarr_DialogueComponent::FindNode(int32 NodeID)
{
    for (FNarr_DialogueNode& Node : DialogueTree.Nodes)
    {
        if (Node.NodeID == NodeID)
        {
            return &Node;
        }
    }
    return nullptr;
}

// ============================================================
// ELDER KAEL — Quest: Herd Under Threat
// ============================================================
void UNarr_DialogueComponent::BuildElderKaelTree()
{
    DialogueTree.NPCRole = ENarr_NPCRole::Elder;
    DialogueTree.RootNodeID = 0;
    DialogueTree.Nodes.Empty();

    // Node 0 — Opening
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 0;
        Node.NPCSpeech = FText::FromString(
            TEXT("The herd moves south. Something hunts them. I have seen this before — when the great lizards flee, death follows close behind."));
        Node.AudioAssetPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782442701499_Elder_Kael.mp3");

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("What should I do?"));
        C1.NextNodeID = 1;
        C1.Outcome = ENarr_DialogueOutcome::None;

        FNarr_DialogueChoice C2;
        C2.ChoiceText = FText::FromString(TEXT("I will track the herd."));
        C2.NextNodeID = 2;
        C2.Outcome = ENarr_DialogueOutcome::StartQuest;
        C2.OutcomePayload = FName("Quest_HerdThreat");

        FNarr_DialogueChoice C3;
        C3.ChoiceText = FText::FromString(TEXT("Not now."));
        C3.NextNodeID = -1;
        C3.Outcome = ENarr_DialogueOutcome::EndConversation;

        Node.PlayerChoices.Add(C1);
        Node.PlayerChoices.Add(C2);
        Node.PlayerChoices.Add(C3);
        DialogueTree.Nodes.Add(Node);
    }

    // Node 1 — Elder advises
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 1;
        Node.NPCSpeech = FText::FromString(
            TEXT("Track the herd south. Find what drives them. If it is raptors, watch for the flanking pair — they always send two ahead. Come back with information. We cannot fight what we cannot see."));

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("I will track the herd."));
        C1.NextNodeID = 2;
        C1.Outcome = ENarr_DialogueOutcome::StartQuest;
        C1.OutcomePayload = FName("Quest_HerdThreat");

        FNarr_DialogueChoice C2;
        C2.ChoiceText = FText::FromString(TEXT("I need more time."));
        C2.NextNodeID = -1;
        C2.Outcome = ENarr_DialogueOutcome::EndConversation;

        Node.PlayerChoices.Add(C1);
        Node.PlayerChoices.Add(C2);
        DialogueTree.Nodes.Add(Node);
    }

    // Node 2 — Quest accepted
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 2;
        Node.bIsEndNode = true;
        Node.NPCSpeech = FText::FromString(
            TEXT("Take your spear. Find what drives them. Come back alive. The camp needs your eyes more than your courage."));

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("I will return."));
        C1.NextNodeID = -1;
        C1.Outcome = ENarr_DialogueOutcome::EndConversation;

        Node.PlayerChoices.Add(C1);
        DialogueTree.Nodes.Add(Node);
    }
}

// ============================================================
// HUNTER DARA — Quest: Raptor Alpha Hunt
// ============================================================
void UNarr_DialogueComponent::BuildHunterDaraTree()
{
    DialogueTree.NPCRole = ENarr_NPCRole::Hunter;
    DialogueTree.RootNodeID = 0;
    DialogueTree.Nodes.Empty();

    // Node 0 — Opening
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 0;
        Node.NPCSpeech = FText::FromString(
            TEXT("Three days ago I tracked the raptor pack to the ridge. Seven of them — maybe more. The alpha leaves deep claw marks in the mud."));
        Node.AudioAssetPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782442703760_Hunter_Dara.mp3");

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("How do we kill the alpha?"));
        C1.NextNodeID = 1;

        FNarr_DialogueChoice C2;
        C2.ChoiceText = FText::FromString(TEXT("I will hunt it alone."));
        C2.NextNodeID = 2;
        C2.Outcome = ENarr_DialogueOutcome::StartQuest;
        C2.OutcomePayload = FName("Quest_RaptorAlpha");

        FNarr_DialogueChoice C3;
        C3.ChoiceText = FText::FromString(TEXT("Leave me out of this."));
        C3.NextNodeID = -1;
        C3.Outcome = ENarr_DialogueOutcome::EndConversation;

        Node.PlayerChoices.Add(C1);
        Node.PlayerChoices.Add(C2);
        Node.PlayerChoices.Add(C3);
        DialogueTree.Nodes.Add(Node);
    }

    // Node 1 — Hunting strategy
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 1;
        Node.NPCSpeech = FText::FromString(
            TEXT("Do not face it head on. Raptors test you first — they circle, they watch. Wait for the alpha to separate from the pack. One spear to the neck. Fast and clean. Hesitate and it calls the others."));

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("I understand. I will hunt it."));
        C1.NextNodeID = 2;
        C1.Outcome = ENarr_DialogueOutcome::StartQuest;
        C1.OutcomePayload = FName("Quest_RaptorAlpha");

        FNarr_DialogueChoice C2;
        C2.ChoiceText = FText::FromString(TEXT("I need a better weapon first."));
        C2.NextNodeID = 3;
        C2.Outcome = ENarr_DialogueOutcome::UnlockRecipe;
        C2.OutcomePayload = FName("Recipe_FlintSpear");

        Node.PlayerChoices.Add(C1);
        Node.PlayerChoices.Add(C2);
        DialogueTree.Nodes.Add(Node);
    }

    // Node 2 — Quest accepted
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 2;
        Node.bIsEndNode = true;
        Node.NPCSpeech = FText::FromString(
            TEXT("Kill the alpha and the pack scatters. They will not raid the camp again for a season. Good hunting."));

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("I will bring back proof."));
        C1.NextNodeID = -1;
        C1.Outcome = ENarr_DialogueOutcome::EndConversation;

        Node.PlayerChoices.Add(C1);
        DialogueTree.Nodes.Add(Node);
    }

    // Node 3 — Recipe unlock
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 3;
        Node.bIsEndNode = true;
        Node.NPCSpeech = FText::FromString(
            TEXT("Find flint near the river rocks. Two sticks, one sharp stone. Bind them tight with hide strips. The flint spear will pierce raptor hide. Come back when you have made one."));

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("I will craft one and return."));
        C1.NextNodeID = -1;
        C1.Outcome = ENarr_DialogueOutcome::EndConversation;

        Node.PlayerChoices.Add(C1);
        DialogueTree.Nodes.Add(Node);
    }
}

// ============================================================
// SCOUT MIRA — Quest: Follow the Giants (Brachiosaurus Migration)
// ============================================================
void UNarr_DialogueComponent::BuildScoutMiraTree()
{
    DialogueTree.NPCRole = ENarr_NPCRole::Scout;
    DialogueTree.RootNodeID = 0;
    DialogueTree.Nodes.Empty();

    // Node 0 — Opening
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 0;
        Node.NPCSpeech = FText::FromString(
            TEXT("I followed the Brachiosaurus trail for two days. They cross the river at dawn — hundreds of them. If we move with the herd, we stay hidden from the predators."));
        Node.AudioAssetPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782442725445_Scout_Mira.mp3");

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("Where does the herd cross?"));
        C1.NextNodeID = 1;

        FNarr_DialogueChoice C2;
        C2.ChoiceText = FText::FromString(TEXT("I will scout ahead of the herd."));
        C2.NextNodeID = 2;
        C2.Outcome = ENarr_DialogueOutcome::StartQuest;
        C2.OutcomePayload = FName("Quest_Migration");

        FNarr_DialogueChoice C3;
        C3.ChoiceText = FText::FromString(TEXT("Too dangerous."));
        C3.NextNodeID = -1;
        C3.Outcome = ENarr_DialogueOutcome::EndConversation;

        Node.PlayerChoices.Add(C1);
        Node.PlayerChoices.Add(C2);
        Node.PlayerChoices.Add(C3);
        DialogueTree.Nodes.Add(Node);
    }

    // Node 1 — River crossing details
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 1;
        Node.NPCSpeech = FText::FromString(
            TEXT("The shallow ford, north-east of the tall rocks. The giants know the safe crossing — they have done it for generations. Stay on the eastern bank and watch. Do not get between them."));

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("I will go now."));
        C1.NextNodeID = 2;
        C1.Outcome = ENarr_DialogueOutcome::StartQuest;
        C1.OutcomePayload = FName("Quest_Migration");

        Node.PlayerChoices.Add(C1);
        DialogueTree.Nodes.Add(Node);
    }

    // Node 2 — Quest accepted
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 2;
        Node.bIsEndNode = true;
        Node.NPCSpeech = FText::FromString(
            TEXT("The giants are our shield. Stay close but not too close. Mark the route they take — we may need to follow it when the dry season comes."));

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("I will mark the route."));
        C1.NextNodeID = -1;
        C1.Outcome = ENarr_DialogueOutcome::EndConversation;

        Node.PlayerChoices.Add(C1);
        DialogueTree.Nodes.Add(Node);
    }
}

// ============================================================
// SCOUT RENN — Quest: Water Trail (Parasaurolophus)
// ============================================================
void UNarr_DialogueComponent::BuildScoutRennTree()
{
    DialogueTree.NPCRole = ENarr_NPCRole::Scout;
    DialogueTree.RootNodeID = 0;
    DialogueTree.Nodes.Empty();

    // Node 0 — Opening
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 0;
        Node.NPCSpeech = FText::FromString(
            TEXT("The water hole is drying up. The Parasaurolophus know where the deep springs are — I have watched them. Follow their trail before the dry season takes everything."));
        Node.AudioAssetPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782442727609_Scout_Renn.mp3");

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("How long do we have?"));
        C1.NextNodeID = 1;

        FNarr_DialogueChoice C2;
        C2.ChoiceText = FText::FromString(TEXT("I will follow the herd."));
        C2.NextNodeID = 2;
        C2.Outcome = ENarr_DialogueOutcome::StartQuest;
        C2.OutcomePayload = FName("Quest_WaterTrail");

        FNarr_DialogueChoice C3;
        C3.ChoiceText = FText::FromString(TEXT("We will find another way."));
        C3.NextNodeID = -1;
        C3.Outcome = ENarr_DialogueOutcome::EndConversation;

        Node.PlayerChoices.Add(C1);
        Node.PlayerChoices.Add(C2);
        Node.PlayerChoices.Add(C3);
        DialogueTree.Nodes.Add(Node);
    }

    // Node 1 — Urgency
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 1;
        Node.NPCSpeech = FText::FromString(
            TEXT("Without water, the camp dies in four days. The Parasaurolophus move at dusk — they are calmer then. Follow them east. They will lead you to the spring. Do not startle the herd."));

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("I will go at dusk."));
        C1.NextNodeID = 2;
        C1.Outcome = ENarr_DialogueOutcome::StartQuest;
        C1.OutcomePayload = FName("Quest_WaterTrail");

        Node.PlayerChoices.Add(C1);
        DialogueTree.Nodes.Add(Node);
    }

    // Node 2 — Quest accepted
    {
        FNarr_DialogueNode Node;
        Node.NodeID = 2;
        Node.bIsEndNode = true;
        Node.NPCSpeech = FText::FromString(
            TEXT("Take a water container if you have one. Mark the spring location. If the water is clean, we move camp closer. The camp's survival depends on this."));

        FNarr_DialogueChoice C1;
        C1.ChoiceText = FText::FromString(TEXT("I will find the spring."));
        C1.NextNodeID = -1;
        C1.Outcome = ENarr_DialogueOutcome::EndConversation;

        Node.PlayerChoices.Add(C1);
        DialogueTree.Nodes.Add(Node);
    }
}

// ============================================================
// ANarr_NPCActor
// ============================================================

ANarr_NPCActor::ANarr_NPCActor()
{
    PrimaryActorTick.bCanEverTick = true;

    DialogueComponent = CreateDefaultSubobject<UNarr_DialogueComponent>(TEXT("DialogueComponent"));
}

void ANarr_NPCActor::BeginPlay()
{
    Super::BeginPlay();
    if (DialogueComponent && !NPCID.IsNone())
    {
        DialogueComponent->LoadDialogueTree(NPCID);
    }
}

void ANarr_NPCActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Proximity check handled by game mode or player controller
}

void ANarr_NPCActor::OnPlayerApproach(AActor* Player)
{
    if (!bPlayerNearby)
    {
        bPlayerNearby = true;
        // Broadcast event for UI to show interaction prompt
        UE_LOG(LogTemp, Log, TEXT("Narr_NPCActor: Player approached %s"), *GetActorLabel());
    }
}

void ANarr_NPCActor::OnPlayerLeave()
{
    bPlayerNearby = false;
    if (DialogueComponent && DialogueComponent->bConversationActive)
    {
        DialogueComponent->EndConversation();
    }
}
