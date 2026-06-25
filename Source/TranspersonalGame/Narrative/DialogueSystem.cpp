// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue Agent
// Cycle: PROD_CYCLE_AUTO_20260625_010
// Quest NPC dialogue, branching conversations, proximity detection

#include "DialogueSystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// ANarr_DialogueNPC — Implementation
// ============================================================

ANarr_DialogueNPC::ANarr_DialogueNPC()
{
    PrimaryActorTick.bCanEverTick = true;

    NPCName = TEXT("Unknown NPC");
    NPCRole = ENarr_NPCRole::Survivor;
    CurrentState = ENarr_DialogueState::Idle;
    InteractionRadius = 300.0f;
    bPlayerInRange = false;
    CurrentLineIndex = 0;
    ActiveLineArray = nullptr;
}

void ANarr_DialogueNPC::BeginPlay()
{
    Super::BeginPlay();

    // Default: populate Elder Kael quest dialogue if no data set
    if (QuestDialogue.QuestID.IsEmpty())
    {
        QuestDialogue.QuestID = TEXT("QUEST_CRAFT_SPEAR");

        // Greeting
        FNarr_DialogueLine Greeting;
        Greeting.SpeakerName = NPCName;
        Greeting.LineText = TEXT("You survived the night. Good. Most do not.");
        Greeting.TriggerState = ENarr_DialogueState::Greeting;
        Greeting.DisplayDuration = 4.0f;
        QuestDialogue.GreetingLines.Add(Greeting);

        // Quest offer
        FNarr_DialogueLine Offer;
        Offer.SpeakerName = NPCName;
        Offer.LineText = TEXT("The hunting grounds to the north are rich with prey. Gather three flint stones and two branches. Craft yourself a spear before nightfall.");
        Offer.TriggerState = ENarr_DialogueState::QuestOffer;
        Offer.DisplayDuration = 6.0f;
        QuestDialogue.QuestOfferLines.Add(Offer);

        // Quest active
        FNarr_DialogueLine Active;
        Active.SpeakerName = NPCName;
        Active.LineText = TEXT("You still need the spear. Flint near the dry riverbed. Branches from fallen trees. Do not come back empty-handed.");
        Active.TriggerState = ENarr_DialogueState::QuestActive;
        Active.DisplayDuration = 5.0f;
        QuestDialogue.QuestActiveLines.Add(Active);

        // Quest complete
        FNarr_DialogueLine Complete;
        Complete.SpeakerName = NPCName;
        Complete.LineText = TEXT("You made it. The tribe sees you now. You are no longer a stranger — you are one of us.");
        Complete.TriggerState = ENarr_DialogueState::QuestComplete;
        Complete.DisplayDuration = 5.0f;
        QuestDialogue.QuestCompleteLines.Add(Complete);
    }
}

void ANarr_DialogueNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdatePlayerProximity();
}

void ANarr_DialogueNPC::BeginDialogue()
{
    CurrentLineIndex = 0;
    CurrentState = ENarr_DialogueState::Greeting;
    ActiveLineArray = &QuestDialogue.GreetingLines;
}

void ANarr_DialogueNPC::AdvanceDialogue()
{
    if (!ActiveLineArray || ActiveLineArray->Num() == 0)
    {
        EndDialogue();
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveLineArray->Num())
    {
        // Move to next state
        switch (CurrentState)
        {
        case ENarr_DialogueState::Greeting:
            CurrentState = ENarr_DialogueState::QuestOffer;
            ActiveLineArray = &QuestDialogue.QuestOfferLines;
            CurrentLineIndex = 0;
            break;

        case ENarr_DialogueState::QuestOffer:
            CurrentState = ENarr_DialogueState::QuestActive;
            ActiveLineArray = &QuestDialogue.QuestActiveLines;
            CurrentLineIndex = 0;
            break;

        case ENarr_DialogueState::QuestActive:
        case ENarr_DialogueState::QuestComplete:
        default:
            EndDialogue();
            break;
        }
    }
}

void ANarr_DialogueNPC::EndDialogue()
{
    CurrentState = ENarr_DialogueState::Farewell;
    CurrentLineIndex = 0;
    ActiveLineArray = nullptr;
}

void ANarr_DialogueNPC::SetQuestState(ENarr_DialogueState NewState)
{
    CurrentState = NewState;
    CurrentLineIndex = 0;

    switch (NewState)
    {
    case ENarr_DialogueState::Greeting:
        ActiveLineArray = &QuestDialogue.GreetingLines;
        break;
    case ENarr_DialogueState::QuestOffer:
        ActiveLineArray = &QuestDialogue.QuestOfferLines;
        break;
    case ENarr_DialogueState::QuestActive:
        ActiveLineArray = &QuestDialogue.QuestActiveLines;
        break;
    case ENarr_DialogueState::QuestComplete:
        ActiveLineArray = &QuestDialogue.QuestCompleteLines;
        break;
    default:
        ActiveLineArray = nullptr;
        break;
    }
}

FNarr_DialogueLine ANarr_DialogueNPC::GetCurrentLine() const
{
    if (ActiveLineArray && ActiveLineArray->IsValidIndex(CurrentLineIndex))
    {
        return (*ActiveLineArray)[CurrentLineIndex];
    }

    // Return empty line
    FNarr_DialogueLine Empty;
    Empty.SpeakerName = NPCName;
    Empty.LineText = TEXT("...");
    Empty.DisplayDuration = 2.0f;
    return Empty;
}

bool ANarr_DialogueNPC::IsPlayerInRange() const
{
    return bPlayerInRange;
}

void ANarr_DialogueNPC::UpdatePlayerProximity()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    bPlayerInRange = (Distance <= InteractionRadius);
}

// ============================================================
// UNarr_DialogueManagerComponent — Implementation
// ============================================================

UNarr_DialogueManagerComponent::UNarr_DialogueManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    ActiveNPC = nullptr;
    bDialogueActive = false;
    LineTimer = 0.0f;
}

void UNarr_DialogueManagerComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UNarr_DialogueManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bDialogueActive || !ActiveNPC) return;

    // Auto-advance dialogue after line duration
    LineTimer -= DeltaTime;
    if (LineTimer <= 0.0f)
    {
        FNarr_DialogueLine Line = ActiveNPC->GetCurrentLine();
        ActiveNPC->AdvanceDialogue();

        // Check if dialogue ended
        if (ActiveNPC->CurrentState == ENarr_DialogueState::Farewell)
        {
            StopDialogue();
        }
        else
        {
            CurrentDisplayLine = ActiveNPC->GetCurrentLine();
            LineTimer = CurrentDisplayLine.DisplayDuration;
        }
    }
}

void UNarr_DialogueManagerComponent::StartDialogueWithNPC(ANarr_DialogueNPC* NPC)
{
    if (!NPC) return;

    ActiveNPC = NPC;
    bDialogueActive = true;

    ActiveNPC->BeginDialogue();
    CurrentDisplayLine = ActiveNPC->GetCurrentLine();
    LineTimer = CurrentDisplayLine.DisplayDuration;
}

void UNarr_DialogueManagerComponent::AdvanceCurrentDialogue()
{
    if (!bDialogueActive || !ActiveNPC) return;

    ActiveNPC->AdvanceDialogue();

    if (ActiveNPC->CurrentState == ENarr_DialogueState::Farewell)
    {
        StopDialogue();
        return;
    }

    CurrentDisplayLine = ActiveNPC->GetCurrentLine();
    LineTimer = CurrentDisplayLine.DisplayDuration;
}

void UNarr_DialogueManagerComponent::StopDialogue()
{
    bDialogueActive = false;
    ActiveNPC = nullptr;
    LineTimer = 0.0f;
}

bool UNarr_DialogueManagerComponent::HasActiveDialogue() const
{
    return bDialogueActive && (ActiveNPC != nullptr);
}
