#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

// ============================================================
// ANarr_TribeElderNPC — Implementation
// ============================================================

ANarr_TribeElderNPC::ANarr_TribeElderNPC()
{
    PrimaryActorTick.bCanEverTick = true;

    // Body mesh
    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    RootComponent = BodyMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
        TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderMesh.Succeeded())
    {
        BodyMesh->SetStaticMesh(CylinderMesh.Object);
        BodyMesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 1.0f));
    }

    // Interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));

    // Defaults
    CurrentState = ENarr_DialogueState::Idle;
    NPCRole = ENarr_NPCRole::TribeElder;
    NPCName = TEXT("Elder Kara");
    InteractionRadius = 300.0f;
    bPlayerHasStoneAxe = false;
    bRaptorQuestActive = false;
    bRaptorQuestComplete = false;
    CurrentLineIndex = 0;
    bPlayerInRange = false;
}

void ANarr_TribeElderNPC::BeginPlay()
{
    Super::BeginPlay();

    InitDialogueTrees();

    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_TribeElderNPC::OnSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_TribeElderNPC::OnSphereEndOverlap);
}

void ANarr_TribeElderNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarr_TribeElderNPC::InitDialogueTrees()
{
    DialogueTrees.Empty();

    // --- Tree 0: Crafting Tutorial (pre-axe) ---
    FNarr_DialogueTree CraftingTree;
    CraftingTree.TreeID = TEXT("crafting_tutorial");
    CraftingTree.bHasBeenTriggered = false;

    FNarr_DialogueLine Line0;
    Line0.SpeakerName = TEXT("Elder Kara");
    Line0.LineText = TEXT("Before you can face the raptors, you must first prove you can survive.");
    Line0.TriggerState = ENarr_DialogueState::QuestGive;
    Line0.DisplayDuration = 5.0f;
    CraftingTree.Lines.Add(Line0);

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Elder Kara");
    Line1.LineText = TEXT("Take these stones. Find a dry branch near the river. Bring them to the crafting stone at camp.");
    Line1.TriggerState = ENarr_DialogueState::QuestGive;
    Line1.DisplayDuration = 6.0f;
    CraftingTree.Lines.Add(Line1);

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Elder Kara");
    Line2.LineText = TEXT("A stone axe is the difference between hunter and prey.");
    Line2.TriggerState = ENarr_DialogueState::QuestGive;
    Line2.DisplayDuration = 4.0f;
    CraftingTree.Lines.Add(Line2);

    DialogueTrees.Add(CraftingTree);

    // --- Tree 1: Raptor Warning ---
    FNarr_DialogueTree RaptorTree;
    RaptorTree.TreeID = TEXT("raptor_warning");
    RaptorTree.bHasBeenTriggered = false;

    FNarr_DialogueLine RLine0;
    RLine0.SpeakerName = TEXT("Elder Kara");
    RLine0.LineText = TEXT("Three moons ago, a pack moved through the eastern valley. Seven hunters went out. Two came back.");
    RLine0.TriggerState = ENarr_DialogueState::Warning;
    RLine0.DisplayDuration = 6.0f;
    RaptorTree.Lines.Add(RLine0);

    FNarr_DialogueLine RLine1;
    RLine1.SpeakerName = TEXT("Elder Kara");
    RLine1.LineText = TEXT("The raptors do not hunt alone. They circle, they wait, they test you. Do not let them choose the ground.");
    RLine1.TriggerState = ENarr_DialogueState::Warning;
    RLine1.DisplayDuration = 6.0f;
    RaptorTree.Lines.Add(RLine1);

    DialogueTrees.Add(RaptorTree);

    // --- Tree 2: Quest Accept (post-axe) ---
    FNarr_DialogueTree AcceptTree;
    AcceptTree.TreeID = TEXT("quest_accept");
    AcceptTree.bHasBeenTriggered = false;

    FNarr_DialogueLine ALine0;
    ALine0.SpeakerName = TEXT("Elder Kara");
    ALine0.LineText = TEXT("You made it. The axe is rough, but it will hold.");
    ALine0.TriggerState = ENarr_DialogueState::QuestComplete;
    ALine0.DisplayDuration = 4.0f;
    AcceptTree.Lines.Add(ALine0);

    FNarr_DialogueLine ALine1;
    ALine1.SpeakerName = TEXT("Elder Kara");
    ALine1.LineText = TEXT("The raptors have been seen near the eastern crossing. Three days walk. We move at dawn. Stay close to the river. Stay quiet.");
    ALine1.TriggerState = ENarr_DialogueState::QuestComplete;
    ALine1.DisplayDuration = 7.0f;
    AcceptTree.Lines.Add(ALine1);

    DialogueTrees.Add(AcceptTree);

    // --- Tree 3: Valley Lore ---
    FNarr_DialogueTree LoreTree;
    LoreTree.TreeID = TEXT("valley_lore");
    LoreTree.bHasBeenTriggered = false;

    FNarr_DialogueLine LLine0;
    LLine0.SpeakerName = TEXT("Elder Kara");
    LLine0.LineText = TEXT("This valley has fed us for ten generations. The great lizards drink from the same river we do.");
    LLine0.TriggerState = ENarr_DialogueState::Lore;
    LLine0.DisplayDuration = 5.0f;
    LoreTree.Lines.Add(LLine0);

    FNarr_DialogueLine LLine1;
    LLine1.SpeakerName = TEXT("Elder Kara");
    LLine1.LineText = TEXT("We do not hunt what we cannot carry. We do not kill what we do not need. Remember this when you face them.");
    LLine1.TriggerState = ENarr_DialogueState::Lore;
    LLine1.DisplayDuration = 6.0f;
    LoreTree.Lines.Add(LLine1);

    DialogueTrees.Add(LoreTree);
}

FNarr_DialogueLine ANarr_TribeElderNPC::GetCurrentLine() const
{
    // Determine which tree to use based on quest state
    int32 TreeIndex = 0;
    if (bPlayerHasStoneAxe && !bRaptorQuestActive)
    {
        TreeIndex = 2; // quest_accept
    }
    else if (bRaptorQuestActive)
    {
        TreeIndex = 1; // raptor_warning
    }
    else
    {
        TreeIndex = 0; // crafting_tutorial
    }

    if (DialogueTrees.IsValidIndex(TreeIndex))
    {
        const FNarr_DialogueTree& Tree = DialogueTrees[TreeIndex];
        if (Tree.Lines.IsValidIndex(CurrentLineIndex))
        {
            return Tree.Lines[CurrentLineIndex];
        }
    }

    FNarr_DialogueLine Empty;
    Empty.SpeakerName = NPCName;
    Empty.LineText = TEXT("...");
    return Empty;
}

void ANarr_TribeElderNPC::AdvanceDialogueState()
{
    int32 TreeIndex = 0;
    if (bPlayerHasStoneAxe && !bRaptorQuestActive)
    {
        TreeIndex = 2;
    }
    else if (bRaptorQuestActive)
    {
        TreeIndex = 1;
    }

    if (DialogueTrees.IsValidIndex(TreeIndex))
    {
        const FNarr_DialogueTree& Tree = DialogueTrees[TreeIndex];
        CurrentLineIndex = FMath::Min(CurrentLineIndex + 1, Tree.Lines.Num() - 1);
    }
}

void ANarr_TribeElderNPC::OnPlayerEnterRange()
{
    bPlayerInRange = true;
    CurrentLineIndex = 0;

    // Update state based on quest progress
    if (bRaptorQuestComplete)
    {
        CurrentState = ENarr_DialogueState::QuestComplete;
    }
    else if (bPlayerHasStoneAxe)
    {
        CurrentState = ENarr_DialogueState::QuestAccept;
        // Trigger raptor quest
        bRaptorQuestActive = true;
    }
    else
    {
        CurrentState = ENarr_DialogueState::QuestGive;
    }

    UE_LOG(LogTemp, Log, TEXT("ANarr_TribeElderNPC: Player entered range. State=%d"),
        (int32)CurrentState);
}

void ANarr_TribeElderNPC::OnPlayerExitRange()
{
    bPlayerInRange = false;
    CurrentState = ENarr_DialogueState::Idle;
    CurrentLineIndex = 0;
}

FString ANarr_TribeElderNPC::GetNPCDisplayName() const
{
    return NPCName;
}

void ANarr_TribeElderNPC::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        // Check if it's the player character
        if (OtherActor->ActorHasTag(TEXT("Player")) || OtherActor->IsA(APawn::StaticClass()))
        {
            OnPlayerEnterRange();
        }
    }
}

void ANarr_TribeElderNPC::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor != this)
    {
        if (OtherActor->ActorHasTag(TEXT("Player")) || OtherActor->IsA(APawn::StaticClass()))
        {
            OnPlayerExitRange();
        }
    }
}

// ============================================================
// ANarr_DialogueTrigger — Implementation
// ============================================================

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(400.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));

    TriggerID = TEXT("trigger_default");
    NarrationText = TEXT("");
    bOneShot = true;
    bHasTriggered = false;
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnTriggerOverlap);
}

void ANarr_DialogueTrigger::FireNarration()
{
    if (bOneShot && bHasTriggered) return;

    bHasTriggered = true;
    UE_LOG(LogTemp, Log, TEXT("ANarr_DialogueTrigger [%s]: %s"), *TriggerID, *NarrationText);
}

void ANarr_DialogueTrigger::OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && (OtherActor->ActorHasTag(TEXT("Player")) || OtherActor->IsA(APawn::StaticClass())))
    {
        FireNarration();
    }
}
