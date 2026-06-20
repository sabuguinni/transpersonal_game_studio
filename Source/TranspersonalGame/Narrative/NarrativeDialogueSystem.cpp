#include "NarrativeDialogueSystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// UNarr_DialogueTriggerComponent
// ============================================================

UNarr_DialogueTriggerComponent::UNarr_DialogueTriggerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    TriggerRadius = 400.0f;
    TriggerType = ENarr_DialogueTriggerType::Proximity;
    bTriggered = false;
}

void UNarr_DialogueTriggerComponent::BeginPlay()
{
    Super::BeginPlay();
    bTriggered = false;
}

void UNarr_DialogueTriggerComponent::TriggerDialogue()
{
    if (!CanTrigger()) return;

    bTriggered = true;
    DialogueSequence.bHasPlayed = true;

    UE_LOG(LogTemp, Log, TEXT("[NarrDialogue] Triggered sequence: %s (%d lines)"),
        *DialogueSequence.SequenceID,
        DialogueSequence.Lines.Num());
}

bool UNarr_DialogueTriggerComponent::CanTrigger() const
{
    if (bTriggered && !DialogueSequence.bCanRepeat) return false;
    return true;
}

void UNarr_DialogueTriggerComponent::ResetTrigger()
{
    bTriggered = false;
    DialogueSequence.bHasPlayed = false;
}

// ============================================================
// ANarr_DialogueZoneActor
// ============================================================

ANarr_DialogueZoneActor::ANarr_DialogueZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    DialogueTrigger = CreateDefaultSubobject<UNarr_DialogueTriggerComponent>(TEXT("DialogueTrigger"));
    AssignedSpeaker = ENarr_DialogueSpeaker::Scout;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
    bIsPlaying = false;
}

void ANarr_DialogueZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Populate dialogue trigger with lines from DialogueLines array
    if (DialogueTrigger && DialogueLines.Num() > 0)
    {
        DialogueTrigger->DialogueSequence.Lines.Empty();
        for (const FString& LineText : DialogueLines)
        {
            FNarr_DialogueLine Line;
            Line.LineText = LineText;
            Line.Speaker = AssignedSpeaker;
            Line.DisplayDuration = 5.0f;
            DialogueTrigger->DialogueSequence.Lines.Add(Line);
        }
        UE_LOG(LogTemp, Log, TEXT("[NarrDialogue] Zone %s loaded %d lines for speaker %d"),
            *GetActorLabel(),
            DialogueLines.Num(),
            (int32)AssignedSpeaker);
    }
}

void ANarr_DialogueZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsPlaying) return;

    LineTimer -= DeltaTime;
    if (LineTimer <= 0.0f)
    {
        CurrentLineIndex++;
        if (CurrentLineIndex >= DialogueLines.Num())
        {
            bIsPlaying = false;
            CurrentLineIndex = 0;
            UE_LOG(LogTemp, Log, TEXT("[NarrDialogue] Sequence complete on %s"), *GetActorLabel());
        }
        else
        {
            LineTimer = 5.0f;
            UE_LOG(LogTemp, Log, TEXT("[NarrDialogue] Line %d: %s"),
                CurrentLineIndex,
                *DialogueLines[CurrentLineIndex]);
        }
    }
}

void ANarr_DialogueZoneActor::OnPlayerEnterZone()
{
    if (!DialogueTrigger || !DialogueTrigger->CanTrigger()) return;

    DialogueTrigger->TriggerDialogue();
    CurrentLineIndex = 0;
    LineTimer = 5.0f;
    bIsPlaying = true;

    if (DialogueLines.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[NarrDialogue] Playing: %s"), *DialogueLines[0]);
    }
}

FNarr_DialogueLine ANarr_DialogueZoneActor::GetNextLine()
{
    if (DialogueTrigger && CurrentLineIndex < DialogueTrigger->DialogueSequence.Lines.Num())
    {
        return DialogueTrigger->DialogueSequence.Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}
