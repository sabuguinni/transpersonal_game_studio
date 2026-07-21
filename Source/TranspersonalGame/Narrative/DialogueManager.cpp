// DialogueManager.cpp
// Agent #15 — Narrative & Dialogue Agent
// Cycle: PROD_CYCLE_AUTO_20260629_008
// Implements the full dialogue system for NPC interactions in the prehistoric survival world.

#include "DialogueManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UDialogueManager::UDialogueManager()
{
    CurrentDialogueIndex = 0;
    bDialogueActive = false;
    bSubtitlesEnabled = true;
    DialogueCooldownSeconds = 30.0f;
    ProximityTriggerRadius = 400.0f;
}

void UDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ActiveLines.Empty();
    LineTimestamps.Empty();
    CurrentDialogueIndex = 0;
    bDialogueActive = false;
    unreal::log("DialogueManager initialized");
}

void UDialogueManager::Deinitialize()
{
    StopCurrentDialogue();
    Super::Deinitialize();
}

bool UDialogueManager::TriggerDialogue(const FNarr_DialogueSequence& Sequence, AActor* Instigator)
{
    if (!Instigator)
    {
        return false;
    }

    // Check cooldown — avoid repeating the same sequence too soon
    const FString& SeqID = Sequence.SequenceID;
    if (LineTimestamps.Contains(SeqID))
    {
        const float LastTime = LineTimestamps[SeqID];
        const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        if ((Now - LastTime) < DialogueCooldownSeconds)
        {
            return false;
        }
    }

    // Stop any currently playing dialogue
    if (bDialogueActive)
    {
        StopCurrentDialogue();
    }

    // Load the new sequence
    ActiveLines = Sequence.Lines;
    CurrentDialogueIndex = 0;
    bDialogueActive = true;
    CurrentSpeakerActor = Instigator;

    // Record timestamp
    if (GetWorld())
    {
        LineTimestamps.Add(SeqID, GetWorld()->GetTimeSeconds());
    }

    // Broadcast start event
    OnDialogueStarted.Broadcast(Sequence.SequenceID);

    // Play first line
    PlayNextLine();

    return true;
}

void UDialogueManager::PlayNextLine()
{
    if (!bDialogueActive || CurrentDialogueIndex >= ActiveLines.Num())
    {
        FinishDialogue();
        return;
    }

    const FNarr_DialogueLine& Line = ActiveLines[CurrentDialogueIndex];

    // Broadcast the line to UI / audio system
    OnDialogueLine.Broadcast(Line);

    // Advance index
    CurrentDialogueIndex++;

    // Schedule next line after display duration
    if (GetWorld())
    {
        FTimerHandle LineTimer;
        const float Delay = FMath::Max(Line.DisplayDurationSeconds, 1.5f);
        GetWorld()->GetTimerManager().SetTimer(
            LineTimer,
            this,
            &UDialogueManager::PlayNextLine,
            Delay,
            false
        );
    }
}

void UDialogueManager::StopCurrentDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    bDialogueActive = false;
    ActiveLines.Empty();
    CurrentDialogueIndex = 0;
    CurrentSpeakerActor = nullptr;

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    }

    OnDialogueStopped.Broadcast();
}

void UDialogueManager::FinishDialogue()
{
    bDialogueActive = false;
    ActiveLines.Empty();
    CurrentDialogueIndex = 0;
    CurrentSpeakerActor = nullptr;

    OnDialogueFinished.Broadcast();
}

bool UDialogueManager::IsDialogueActive() const
{
    return bDialogueActive;
}

void UDialogueManager::SetSubtitlesEnabled(bool bEnabled)
{
    bSubtitlesEnabled = bEnabled;
}

bool UDialogueManager::AreSubtitlesEnabled() const
{
    return bSubtitlesEnabled;
}

void UDialogueManager::SetDialogueCooldown(float Seconds)
{
    DialogueCooldownSeconds = FMath::Max(0.0f, Seconds);
}

void UDialogueManager::SetProximityRadius(float Radius)
{
    ProximityTriggerRadius = FMath::Max(50.0f, Radius);
}

FNarr_DialogueLine UDialogueManager::MakeDialogueLine(
    const FText& Text,
    ENarr_SpeakerRole Speaker,
    float Duration,
    ENarr_DialogueTrigger Trigger)
{
    FNarr_DialogueLine Line;
    Line.LineText = Text;
    Line.Speaker = Speaker;
    Line.DisplayDurationSeconds = FMath::Max(Duration, 1.0f);
    Line.Trigger = Trigger;
    Line.bHasVoiceAudio = false;
    return Line;
}

FNarr_DialogueSequence UDialogueManager::MakeSequenceFromLines(
    const FString& SequenceID,
    const TArray<FNarr_DialogueLine>& Lines,
    ENarr_DialogueTrigger TriggerType)
{
    FNarr_DialogueSequence Seq;
    Seq.SequenceID = SequenceID;
    Seq.Lines = Lines;
    Seq.TriggerType = TriggerType;
    Seq.bLooping = false;
    Seq.Priority = 1;
    return Seq;
}
