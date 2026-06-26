// DialogueSystem.cpp — Agent #15 Narrative & Dialogue
// Implements UNarr_DialogueSystem: proximity-triggered, quest-linked dialogue
// for NPC survivors, scouts, elders and trackers in the prehistoric world.

#include "DialogueSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

// ============================================================
// Constructor
// ============================================================

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for dialogue timing

    ProximityTriggerRadius = 400.0f;
    bAutoTriggerOnProximity = false;
    CurrentLineIndex = 0;
    bIsPlaying = false;
    ActiveSequenceID = TEXT("");
    LineTimer = 0.0f;
    ActiveSequence = nullptr;
}

// ============================================================
// BeginPlay — seed default dialogue library for testing
// ============================================================

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();

    // Seed a default DinoSighted sequence if library is empty
    if (DialogueLibrary.Num() == 0)
    {
        FNarr_DialogueSequence DinoSeq;
        DinoSeq.SequenceID = TEXT("DINO_SIGHTED_DEFAULT");
        DinoSeq.TriggerType = ENarr_DialogueTrigger::DinoSighted;
        DinoSeq.bCanRepeat = false;
        DinoSeq.bPlayedThisSession = false;

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = TEXT("Kael");
        Line1.SpeakerRole = ENarr_CharacterRole::Survivor;
        Line1.LineText = TEXT("Stay low. Do not run. It tracks movement.");
        Line1.DisplayDuration = 4.0f;

        FNarr_DialogueLine Line2;
        Line2.SpeakerName = TEXT("Mira");
        Line2.SpeakerRole = ENarr_CharacterRole::Scout;
        Line2.LineText = TEXT("Wind is in our favour. Move north — slow.");
        Line2.DisplayDuration = 4.0f;

        DinoSeq.Lines.Add(Line1);
        DinoSeq.Lines.Add(Line2);
        DialogueLibrary.Add(DinoSeq);

        // Night fall sequence
        FNarr_DialogueSequence NightSeq;
        NightSeq.SequenceID = TEXT("NIGHTFALL_DEFAULT");
        NightSeq.TriggerType = ENarr_DialogueTrigger::NightFall;
        NightSeq.bCanRepeat = true;
        NightSeq.bPlayedThisSession = false;

        FNarr_DialogueLine NightLine1;
        NightLine1.SpeakerName = TEXT("Elder Oru");
        NightLine1.SpeakerRole = ENarr_CharacterRole::Elder;
        NightLine1.LineText = TEXT("Fire high. Noise low. The dark belongs to them.");
        NightLine1.DisplayDuration = 5.0f;

        NightSeq.Lines.Add(NightLine1);
        DialogueLibrary.Add(NightSeq);
    }
}

// ============================================================
// TickComponent — advance line timer when playing
// ============================================================

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsPlaying || !ActiveSequence)
    {
        return;
    }

    LineTimer -= DeltaTime;

    if (LineTimer <= 0.0f)
    {
        AdvanceLine();
    }
}

// ============================================================
// TriggerDialogue — start a named sequence
// ============================================================

bool UNarr_DialogueSystem::TriggerDialogue(const FString& SequenceID)
{
    FNarr_DialogueSequence* Seq = FindSequence(SequenceID);
    if (!Seq)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueSystem: Sequence '%s' not found."), *SequenceID);
        return false;
    }

    if (Seq->bPlayedThisSession && !Seq->bCanRepeat)
    {
        return false;
    }

    if (Seq->Lines.Num() == 0)
    {
        return false;
    }

    // Stop any current dialogue
    StopDialogue();

    ActiveSequence = Seq;
    ActiveSequenceID = SequenceID;
    CurrentLineIndex = 0;
    bIsPlaying = true;
    Seq->bPlayedThisSession = true;

    // Set timer for first line
    LineTimer = Seq->Lines[0].DisplayDuration;

    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Starting sequence '%s' (%d lines)"),
        *SequenceID, Seq->Lines.Num());

    return true;
}

// ============================================================
// TriggerByType — find first matching sequence by trigger type
// ============================================================

void UNarr_DialogueSystem::TriggerByType(ENarr_DialogueTrigger TriggerType)
{
    for (FNarr_DialogueSequence& Seq : DialogueLibrary)
    {
        if (Seq.TriggerType == TriggerType)
        {
            if (!Seq.bPlayedThisSession || Seq.bCanRepeat)
            {
                TriggerDialogue(Seq.SequenceID);
                return;
            }
        }
    }
}

// ============================================================
// AdvanceLine — move to next line or end sequence
// ============================================================

void UNarr_DialogueSystem::AdvanceLine()
{
    if (!bIsPlaying || !ActiveSequence)
    {
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveSequence->Lines.Num())
    {
        // Sequence complete
        StopDialogue();
        UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Sequence '%s' complete."), *ActiveSequenceID);
        return;
    }

    // Set timer for next line
    LineTimer = ActiveSequence->Lines[CurrentLineIndex].DisplayDuration;
}

// ============================================================
// StopDialogue
// ============================================================

void UNarr_DialogueSystem::StopDialogue()
{
    bIsPlaying = false;
    ActiveSequence = nullptr;
    ActiveSequenceID = TEXT("");
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
}

// ============================================================
// GetCurrentLine
// ============================================================

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentLine() const
{
    if (!bIsPlaying || !ActiveSequence)
    {
        return FNarr_DialogueLine();
    }

    if (!ActiveSequence->Lines.IsValidIndex(CurrentLineIndex))
    {
        return FNarr_DialogueLine();
    }

    return ActiveSequence->Lines[CurrentLineIndex];
}

// ============================================================
// IsDialoguePlaying
// ============================================================

bool UNarr_DialogueSystem::IsDialoguePlaying() const
{
    return bIsPlaying;
}

// ============================================================
// AddDialogueLine — runtime line injection
// ============================================================

void UNarr_DialogueSystem::AddDialogueLine(const FString& SequenceID, const FNarr_DialogueLine& Line)
{
    FNarr_DialogueSequence* Seq = FindSequence(SequenceID);
    if (Seq)
    {
        Seq->Lines.Add(Line);
        return;
    }

    // Create new sequence
    FNarr_DialogueSequence NewSeq;
    NewSeq.SequenceID = SequenceID;
    NewSeq.TriggerType = ENarr_DialogueTrigger::Manual;
    NewSeq.bCanRepeat = false;
    NewSeq.bPlayedThisSession = false;
    NewSeq.Lines.Add(Line);
    DialogueLibrary.Add(NewSeq);
}

// ============================================================
// GetTotalLinesInSequence
// ============================================================

int32 UNarr_DialogueSystem::GetTotalLinesInSequence(const FString& SequenceID) const
{
    const FNarr_DialogueSequence* Seq = FindSequenceConst(SequenceID);
    return Seq ? Seq->Lines.Num() : 0;
}

// ============================================================
// Private helpers
// ============================================================

FNarr_DialogueSequence* UNarr_DialogueSystem::FindSequence(const FString& SequenceID)
{
    for (FNarr_DialogueSequence& Seq : DialogueLibrary)
    {
        if (Seq.SequenceID == SequenceID)
        {
            return &Seq;
        }
    }
    return nullptr;
}

const FNarr_DialogueSequence* UNarr_DialogueSystem::FindSequenceConst(const FString& SequenceID) const
{
    for (const FNarr_DialogueSequence& Seq : DialogueLibrary)
    {
        if (Seq.SequenceID == SequenceID)
        {
            return &Seq;
        }
    }
    return nullptr;
}
