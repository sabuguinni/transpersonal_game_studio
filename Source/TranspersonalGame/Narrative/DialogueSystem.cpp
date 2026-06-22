// DialogueSystem.cpp
// Narrative & Dialogue Agent #15 — PROD_CYCLE_AUTO_20260622_009
// Implementation of NPC dialogue system for prehistoric survival game

#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"

// ============================================================
// UNarr_DialogueComponent
// ============================================================

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    NPCName = TEXT("Unknown");
    InteractionRadius = 300.0f;
    CurrentState = ENarr_DialogueState::Idle;
    CurrentLineIndex = 0;
    ActiveSequence = nullptr;

    // Default greeting sequence for Tribal Elder
    FNarr_DialogueSequence GreetingSeq;
    GreetingSeq.SequenceID = FName("ElderGreeting");
    GreetingSeq.bCanRepeat = true;
    GreetingSeq.NextSequenceID = FName("ElderInfo");

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Elder");
    Line1.LineText = TEXT("You survived the night. Good. The predators are moving south — tracks near the river.");
    Line1.DisplayDuration = 4.0f;
    Line1.bRequiresPlayerResponse = false;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Elder");
    Line2.LineText = TEXT("Stay close to camp until we know how many there are. Scouts have not returned.");
    Line2.DisplayDuration = 4.0f;
    Line2.bRequiresPlayerResponse = true;

    GreetingSeq.Lines.Add(Line1);
    GreetingSeq.Lines.Add(Line2);
    DialogueSequences.Add(GreetingSeq);

    // Scout warning sequence
    FNarr_DialogueSequence ScoutSeq;
    ScoutSeq.SequenceID = FName("ScoutWarning");
    ScoutSeq.bCanRepeat = false;
    ScoutSeq.NextSequenceID = NAME_None;

    FNarr_DialogueLine ScoutLine1;
    ScoutLine1.SpeakerName = TEXT("Scout");
    ScoutLine1.LineText = TEXT("Three raptors. North ridge. They circled the herd twice before moving on.");
    ScoutLine1.DisplayDuration = 3.5f;
    ScoutLine1.bRequiresPlayerResponse = false;

    FNarr_DialogueLine ScoutLine2;
    ScoutLine2.SpeakerName = TEXT("Scout");
    ScoutLine2.LineText = TEXT("The big one — the Rex — it was watching from the tree line. Waiting.");
    ScoutLine2.DisplayDuration = 4.0f;
    ScoutLine2.bRequiresPlayerResponse = true;

    ScoutSeq.Lines.Add(ScoutLine1);
    ScoutSeq.Lines.Add(ScoutLine2);
    DialogueSequences.Add(ScoutSeq);
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UNarr_DialogueComponent::StartDialogue(FName SequenceID)
{
    if (CurrentState == ENarr_DialogueState::InDialogue)
    {
        return;
    }

    for (FNarr_DialogueSequence& Seq : DialogueSequences)
    {
        if (Seq.SequenceID == SequenceID)
        {
            ActiveSequence = &Seq;
            ActiveSequenceID = SequenceID;
            CurrentLineIndex = 0;
            CurrentState = ENarr_DialogueState::Greeting;

            if (Seq.Lines.Num() > 0)
            {
                CurrentState = ENarr_DialogueState::InDialogue;
            }
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("DialogueSystem: Sequence '%s' not found on %s"),
        *SequenceID.ToString(), *NPCName);
}

void UNarr_DialogueComponent::AdvanceDialogue()
{
    if (CurrentState != ENarr_DialogueState::InDialogue || !ActiveSequence)
    {
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveSequence->Lines.Num())
    {
        // Sequence complete
        if (!ActiveSequence->NextSequenceID.IsNone())
        {
            StartDialogue(ActiveSequence->NextSequenceID);
        }
        else
        {
            EndDialogue();
        }
    }
}

void UNarr_DialogueComponent::EndDialogue()
{
    CurrentState = ENarr_DialogueState::Farewell;
    CurrentLineIndex = 0;
    ActiveSequence = nullptr;
    ActiveSequenceID = NAME_None;

    // Reset to idle after farewell
    CurrentState = ENarr_DialogueState::Idle;
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentLine() const
{
    if (!ActiveSequence || CurrentLineIndex >= ActiveSequence->Lines.Num())
    {
        return FNarr_DialogueLine();
    }
    return ActiveSequence->Lines[CurrentLineIndex];
}

bool UNarr_DialogueComponent::IsInDialogue() const
{
    return CurrentState == ENarr_DialogueState::InDialogue;
}

ENarr_DialogueState UNarr_DialogueComponent::GetDialogueState() const
{
    return CurrentState;
}

// ============================================================
// ANarr_DialogueTrigger
// ============================================================

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(300.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    DialogueSequenceToTrigger = FName("ElderGreeting");
    LinkedNPCActor = nullptr;
    bOneShot = true;
    bHasTriggered = false;
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
}

void ANarr_DialogueTrigger::OnPlayerEnterRange(AActor* PlayerActor)
{
    if (!PlayerActor)
    {
        return;
    }

    if (bOneShot && bHasTriggered)
    {
        return;
    }

    if (LinkedNPCActor)
    {
        UNarr_DialogueComponent* DialogueComp = LinkedNPCActor->FindComponentByClass<UNarr_DialogueComponent>();
        if (DialogueComp)
        {
            DialogueComp->StartDialogue(DialogueSequenceToTrigger);
            bHasTriggered = true;

            UE_LOG(LogTemp, Log, TEXT("DialogueTrigger: Started sequence '%s' for player '%s'"),
                *DialogueSequenceToTrigger.ToString(),
                *PlayerActor->GetName());
        }
    }
}
