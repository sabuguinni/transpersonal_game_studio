// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue Agent
// Implementation of NPC dialogue system for prehistoric survival game

#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

ANarr_DialogueNPC::ANarr_DialogueNPC()
{
    PrimaryActorTick.bCanEverTick = true;

    // Interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    RootComponent = InteractionSphere;
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueNPC::OnPlayerEnterRange);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_DialogueNPC::OnPlayerExitRange);

    // Defaults
    NPCName = TEXT("Unknown");
    NPCRole = ENarr_NPCRole::Survivor;
    InteractionRadius = 300.0f;
    CurrentState = ENarr_DialogueState::Idle;
    bPlayerInRange = false;
    CurrentLineIndex = 0;
    bOffersQuest = false;
    QuestID = TEXT("");
    bQuestAccepted = false;
    ActiveTreeID = TEXT("default");
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void ANarr_DialogueNPC::BeginPlay()
{
    Super::BeginPlay();

    // Update sphere radius from property
    if (InteractionSphere)
    {
        InteractionSphere->SetSphereRadius(InteractionRadius);
    }
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

void ANarr_DialogueNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Future: auto-advance timed dialogue lines
}

// ─── Dialogue Control ─────────────────────────────────────────────────────────

void ANarr_DialogueNPC::StartDialogue()
{
    if (CurrentState == ENarr_DialogueState::InConversation)
    {
        return;
    }

    CurrentLineIndex = 0;
    CurrentState = ENarr_DialogueState::Greeting;

    FNarr_DialogueTree* Tree = FindActiveTree();
    if (Tree && Tree->Lines.Num() > 0)
    {
        CurrentState = ENarr_DialogueState::InConversation;
        UE_LOG(LogTemp, Log, TEXT("[Dialogue] %s: %s"), *NPCName, *Tree->Lines[0].LineText);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Dialogue] No dialogue tree found for NPC: %s"), *NPCName);
        EndDialogue();
    }
}

void ANarr_DialogueNPC::AdvanceDialogue()
{
    FNarr_DialogueTree* Tree = FindActiveTree();
    if (!Tree)
    {
        EndDialogue();
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= Tree->Lines.Num())
    {
        // All lines exhausted — check for quest offer
        if (bOffersQuest && !bQuestAccepted)
        {
            CurrentState = ENarr_DialogueState::QuestOffer;
            UE_LOG(LogTemp, Log, TEXT("[Dialogue] Quest offered: %s"), *QuestID);
        }
        else
        {
            EndDialogue();
        }
        return;
    }

    FNarr_DialogueLine& Line = Tree->Lines[CurrentLineIndex];
    UE_LOG(LogTemp, Log, TEXT("[Dialogue] %s: %s"), *NPCName, *Line.LineText);
}

void ANarr_DialogueNPC::EndDialogue()
{
    CurrentState = ENarr_DialogueState::Farewell;
    CurrentLineIndex = 0;

    // Reset to idle after farewell
    CurrentState = ENarr_DialogueState::Idle;
    UE_LOG(LogTemp, Log, TEXT("[Dialogue] Conversation ended with %s"), *NPCName);
}

FNarr_DialogueLine ANarr_DialogueNPC::GetCurrentLine() const
{
    FNarr_DialogueTree* Tree = const_cast<ANarr_DialogueNPC*>(this)->FindActiveTree();
    if (Tree && CurrentLineIndex < Tree->Lines.Num())
    {
        return Tree->Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

bool ANarr_DialogueNPC::HasMoreLines() const
{
    FNarr_DialogueTree* Tree = const_cast<ANarr_DialogueNPC*>(this)->FindActiveTree();
    if (!Tree) return false;
    return CurrentLineIndex < Tree->Lines.Num() - 1;
}

void ANarr_DialogueNPC::AcceptQuest()
{
    if (bOffersQuest && !bQuestAccepted)
    {
        bQuestAccepted = true;
        CurrentState = ENarr_DialogueState::QuestComplete;
        UE_LOG(LogTemp, Log, TEXT("[Dialogue] Quest accepted: %s from %s"), *QuestID, *NPCName);
        EndDialogue();
    }
}

// ─── Pure Getters ─────────────────────────────────────────────────────────────

FString ANarr_DialogueNPC::GetNPCDisplayName() const
{
    return NPCName;
}

ENarr_DialogueState ANarr_DialogueNPC::GetDialogueState() const
{
    return CurrentState;
}

// ─── Overlap Handlers ─────────────────────────────────────────────────────────

void ANarr_DialogueNPC::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Check if it's a character (player)
    if (OtherActor->IsA(ACharacter::StaticClass()))
    {
        bPlayerInRange = true;
        UE_LOG(LogTemp, Log, TEXT("[Dialogue] Player entered range of %s"), *NPCName);
        // Auto-greet
        if (CurrentState == ENarr_DialogueState::Idle)
        {
            StartDialogue();
        }
    }
}

void ANarr_DialogueNPC::OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    if (OtherActor->IsA(ACharacter::StaticClass()))
    {
        bPlayerInRange = false;
        if (CurrentState == ENarr_DialogueState::InConversation)
        {
            EndDialogue();
        }
        UE_LOG(LogTemp, Log, TEXT("[Dialogue] Player left range of %s"), *NPCName);
    }
}

// ─── Internal Helpers ─────────────────────────────────────────────────────────

FNarr_DialogueTree* ANarr_DialogueNPC::FindActiveTree()
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == ActiveTreeID)
        {
            return &Tree;
        }
    }
    // Fallback: return first tree if available
    if (DialogueTrees.Num() > 0)
    {
        return &DialogueTrees[0];
    }
    return nullptr;
}
