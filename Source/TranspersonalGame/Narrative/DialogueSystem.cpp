// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue Agent
// Prehistoric survival dialogue system — full implementation

#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// ANarr_DialogueNPC — Constructor
// ─────────────────────────────────────────────────────────────────────────────

ANarr_DialogueNPC::ANarr_DialogueNPC()
    : NPCName(TEXT("Tribal Elder"))
    , NPCRole(ENarr_NPCRole::TribalElder)
    , ActiveTreeID(NAME_None)
    , InteractionRadius(300.0f)
    , bIsInDialogue(false)
    , CurrentLineIndex(0)
    , CurrentTreeIndex(0)
{
    PrimaryActorTick.bCanEverTick = true;

    // Root mesh component
    NPCMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCMesh"));
    RootComponent = NPCMesh;

    // Assign basic sphere mesh as placeholder
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        NPCMesh->SetStaticMesh(SphereMeshAsset.Object);
        NPCMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 2.0f));
    }

    // Interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(InteractionRadius);
    InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));
}

// ─────────────────────────────────────────────────────────────────────────────
// ANarr_DialogueNPC — BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ANarr_DialogueNPC::BeginPlay()
{
    Super::BeginPlay();

    // Load default survival dialogue
    LoadDefaultDialogue();

    // Register with the dialogue manager
    if (UWorld* World = GetWorld())
    {
        if (UNarr_DialogueManager* Manager = World->GetSubsystem<UNarr_DialogueManager>())
        {
            Manager->RegisterNPC(this);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ANarr_DialogueNPC — Tick
// ─────────────────────────────────────────────────────────────────────────────

void ANarr_DialogueNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Future: patrol, idle animation triggers
}

// ─────────────────────────────────────────────────────────────────────────────
// ANarr_DialogueNPC — Dialogue Logic
// ─────────────────────────────────────────────────────────────────────────────

void ANarr_DialogueNPC::StartDialogue(AActor* Initiator)
{
    if (bIsInDialogue || DialogueTrees.Num() == 0)
    {
        return;
    }

    bIsInDialogue = true;
    CurrentTreeIndex = 0;
    CurrentLineIndex = 0;

    // Set active tree to first available
    if (DialogueTrees.IsValidIndex(0))
    {
        ActiveTreeID = DialogueTrees[0].TreeID;
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueNPC [%s]: Dialogue started with %s"),
        *NPCName, Initiator ? *Initiator->GetName() : TEXT("Unknown"));
}

void ANarr_DialogueNPC::AdvanceDialogue()
{
    if (!bIsInDialogue)
    {
        return;
    }

    FNarr_DialogueTree* ActiveTree = FindTree(ActiveTreeID);
    if (!ActiveTree)
    {
        EndDialogue();
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveTree->Lines.Num())
    {
        // Move to next tree if available
        if (!ActiveTree->NextTreeID.IsNone())
        {
            ActiveTreeID = ActiveTree->NextTreeID;
            CurrentLineIndex = 0;
        }
        else
        {
            // Mark complete and end
            ActiveTree->bIsCompleted = true;
            EndDialogue();
        }
    }
}

void ANarr_DialogueNPC::EndDialogue()
{
    bIsInDialogue = false;
    CurrentLineIndex = 0;
    ActiveTreeID = NAME_None;

    UE_LOG(LogTemp, Log, TEXT("DialogueNPC [%s]: Dialogue ended"), *NPCName);
}

FNarr_DialogueLine ANarr_DialogueNPC::GetCurrentLine() const
{
    FNarr_DialogueLine EmptyLine;

    for (const FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == ActiveTreeID)
        {
            if (Tree.Lines.IsValidIndex(CurrentLineIndex))
            {
                return Tree.Lines[CurrentLineIndex];
            }
        }
    }

    return EmptyLine;
}

bool ANarr_DialogueNPC::HasActiveDialogue() const
{
    return bIsInDialogue && !ActiveTreeID.IsNone();
}

FString ANarr_DialogueNPC::GetNPCDisplayName() const
{
    return NPCName;
}

// ─────────────────────────────────────────────────────────────────────────────
// ANarr_DialogueNPC — Private Helpers
// ─────────────────────────────────────────────────────────────────────────────

void ANarr_DialogueNPC::LoadDefaultDialogue()
{
    // Survival-focused dialogue — no spiritual content
    // Tree 1: Warning about raptor pack
    FNarr_DialogueTree WarningTree;
    WarningTree.TreeID = FName("Warning_RaptorPack");
    WarningTree.bIsCompleted = false;
    WarningTree.NextTreeID = FName("Quest_ScoutNorth");

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = NPCName;
    Line1.LineText = TEXT("The raptors are moving in packs now. Three days ago, one scout. Yesterday, five. Today I stopped counting.");
    Line1.DialogueType = ENarr_DialogueType::Warning;
    Line1.DisplayDuration = 5.0f;
    Line1.bRequiresPlayerResponse = false;
    WarningTree.Lines.Add(Line1);

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = NPCName;
    Line2.LineText = TEXT("We cannot move the wounded. But staying here means death when the pack returns at dusk.");
    Line2.DialogueType = ENarr_DialogueType::Warning;
    Line2.DisplayDuration = 5.0f;
    Line2.bRequiresPlayerResponse = false;
    WarningTree.Lines.Add(Line2);

    FNarr_DialogueLine Line3;
    Line3.SpeakerName = NPCName;
    Line3.LineText = TEXT("You are fast. Scout the northern ridge. Find us a defensible position. Come back before the sun touches the valley rim.");
    Line3.DialogueType = ENarr_DialogueType::QuestGive;
    Line3.DisplayDuration = 6.0f;
    Line3.bRequiresPlayerResponse = true;
    WarningTree.Lines.Add(Line3);

    DialogueTrees.Add(WarningTree);

    // Tree 2: Quest follow-up
    FNarr_DialogueTree QuestTree;
    QuestTree.TreeID = FName("Quest_ScoutNorth");
    QuestTree.bIsCompleted = false;
    QuestTree.NextTreeID = NAME_None;

    FNarr_DialogueLine QLine1;
    QLine1.SpeakerName = NPCName;
    QLine1.LineText = TEXT("The northern ridge — look for the two dead trees on the skyline. The caves below them held our people three seasons ago.");
    QLine1.DialogueType = ENarr_DialogueType::Lore;
    QLine1.DisplayDuration = 5.0f;
    QLine1.bRequiresPlayerResponse = false;
    QuestTree.Lines.Add(QLine1);

    FNarr_DialogueLine QLine2;
    QLine2.SpeakerName = NPCName;
    QLine2.LineText = TEXT("Watch for the big one. The TRex has been marking territory along the river. Stay high. Stay quiet.");
    QLine2.DialogueType = ENarr_DialogueType::Urgent;
    QLine2.DisplayDuration = 5.0f;
    QLine2.bRequiresPlayerResponse = false;
    QuestTree.Lines.Add(QLine2);

    DialogueTrees.Add(QuestTree);
}

FNarr_DialogueTree* ANarr_DialogueNPC::FindTree(FName TreeID)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID)
        {
            return &Tree;
        }
    }
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// UNarr_DialogueManager — World Subsystem
// ─────────────────────────────────────────────────────────────────────────────

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bAnyDialogueActive = false;
    RegisteredNPCs.Empty();
    UE_LOG(LogTemp, Log, TEXT("NarrDialogueManager: Initialized"));
}

void UNarr_DialogueManager::Deinitialize()
{
    RegisteredNPCs.Empty();
    Super::Deinitialize();
}

void UNarr_DialogueManager::RegisterNPC(ANarr_DialogueNPC* NPC)
{
    if (NPC && !RegisteredNPCs.Contains(NPC))
    {
        RegisteredNPCs.Add(NPC);
        UE_LOG(LogTemp, Log, TEXT("NarrDialogueManager: Registered NPC [%s]"), *NPC->GetNPCDisplayName());
    }
}

void UNarr_DialogueManager::UnregisterNPC(ANarr_DialogueNPC* NPC)
{
    RegisteredNPCs.Remove(NPC);
}

ANarr_DialogueNPC* UNarr_DialogueManager::FindNearestNPC(FVector PlayerLocation, float MaxRadius) const
{
    ANarr_DialogueNPC* Nearest = nullptr;
    float NearestDistSq = MaxRadius * MaxRadius;

    for (ANarr_DialogueNPC* NPC : RegisteredNPCs)
    {
        if (!NPC || !IsValid(NPC))
        {
            continue;
        }

        float DistSq = FVector::DistSquared(PlayerLocation, NPC->GetActorLocation());
        if (DistSq < NearestDistSq)
        {
            NearestDistSq = DistSq;
            Nearest = NPC;
        }
    }

    return Nearest;
}

bool UNarr_DialogueManager::IsAnyDialogueActive() const
{
    for (const ANarr_DialogueNPC* NPC : RegisteredNPCs)
    {
        if (NPC && NPC->HasActiveDialogue())
        {
            return true;
        }
    }
    return false;
}

int32 UNarr_DialogueManager::GetRegisteredNPCCount() const
{
    return RegisteredNPCs.Num();
}
