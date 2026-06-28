#include "DialogueSystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    ProximityTriggerRadius = 400.0f;
    bDialogueActive = false;
    LineTimer = 0.0f;
    LineDisplayDuration = 5.0f;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    LoadDefaultDialogueLines();
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bDialogueActive)
    {
        LineTimer += DeltaTime;
        if (LineTimer >= LineDisplayDuration)
        {
            LineTimer = 0.0f;
            AdvanceDialogue();
        }
    }
}

void UNarr_DialogueSystem::StartDialogueSequence(const FString& SequenceID)
{
    for (const FNarr_DialogueSequence& Seq : RegisteredSequences)
    {
        if (Seq.SequenceID == SequenceID)
        {
            ActiveSequence = Seq;
            ActiveSequence.CurrentLineIndex = 0;
            ActiveSequence.bIsActive = true;
            bDialogueActive = true;
            LineTimer = 0.0f;

            if (ActiveSequence.Lines.Num() > 0)
            {
                LineDisplayDuration = ActiveSequence.Lines[0].DisplayDuration;
            }
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("DialogueSystem: Sequence '%s' not found"), *SequenceID);
}

void UNarr_DialogueSystem::AdvanceDialogue()
{
    if (!bDialogueActive) return;

    ActiveSequence.CurrentLineIndex++;

    if (ActiveSequence.CurrentLineIndex >= ActiveSequence.Lines.Num())
    {
        EndDialogueSequence();
        return;
    }

    const FNarr_DialogueLine& NextLine = ActiveSequence.Lines[ActiveSequence.CurrentLineIndex];
    LineDisplayDuration = NextLine.DisplayDuration;
    LineTimer = 0.0f;
}

void UNarr_DialogueSystem::EndDialogueSequence()
{
    bDialogueActive = false;
    ActiveSequence.bIsActive = false;
    LineTimer = 0.0f;
}

bool UNarr_DialogueSystem::IsDialogueActive() const
{
    return bDialogueActive;
}

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentLine() const
{
    if (bDialogueActive && ActiveSequence.Lines.IsValidIndex(ActiveSequence.CurrentLineIndex))
    {
        return ActiveSequence.Lines[ActiveSequence.CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

void UNarr_DialogueSystem::TriggerContextualDialogue(ENarr_DialogueTrigger Trigger, const FString& ContextID)
{
    for (const FNarr_DialogueLine& Line : RegisteredLines)
    {
        if (Line.TriggerType == Trigger && !Line.bHasBeenPlayed)
        {
            // Build a one-shot sequence from this line
            FNarr_DialogueSequence OneShotSeq;
            OneShotSeq.SequenceID = TEXT("OneShot_") + Line.LineID;
            OneShotSeq.Lines.Add(Line);
            OneShotSeq.CurrentLineIndex = 0;
            OneShotSeq.bIsActive = true;

            ActiveSequence = OneShotSeq;
            bDialogueActive = true;
            LineTimer = 0.0f;
            LineDisplayDuration = Line.DisplayDuration;

            UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Triggered contextual line '%s' for context '%s'"),
                *Line.LineID, *ContextID);
            return;
        }
    }
}

void UNarr_DialogueSystem::RegisterDialogueLine(const FNarr_DialogueLine& Line)
{
    RegisteredLines.Add(Line);
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    // Remove existing sequence with same ID
    RegisteredSequences.RemoveAll([&](const FNarr_DialogueSequence& Seq)
    {
        return Seq.SequenceID == Sequence.SequenceID;
    });
    RegisteredSequences.Add(Sequence);
}

void UNarr_DialogueSystem::LoadDefaultDialogueLines()
{
    // TrailReader — Danger trigger: predator tracks spotted
    {
        FNarr_DialogueLine Line;
        Line.LineID = TEXT("trail_reader_tracks_001");
        Line.SpeakerName = TEXT("TrailReader");
        Line.SpeakerRole = ENarr_SpeakerRole::TrailReader;
        Line.DialogueText = TEXT("Listen. The ground tells us things. Three claws, deep in the mud, fresh from this morning. The big hunter was here. Moving north. We go east, find the high ground, and we wait.");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782625717061_TrailReader.mp3");
        Line.TriggerType = ENarr_DialogueTrigger::Danger;
        Line.DisplayDuration = 8.0f;
        Line.bHasBeenPlayed = false;
        RegisteredLines.Add(Line);
    }

    // HuntCaller — QuestStart trigger: hunt begins
    {
        FNarr_DialogueLine Line;
        Line.LineID = TEXT("hunt_caller_herd_001");
        Line.SpeakerName = TEXT("HuntCaller");
        Line.SpeakerRole = ENarr_SpeakerRole::HuntCaller;
        Line.DialogueText = TEXT("The tribe needs meat. Two days without food. We take the old one at the edge. Fast, clean, and we run before the noise draws attention. Are you with me?");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782625720437_HuntCaller.mp3");
        Line.TriggerType = ENarr_DialogueTrigger::QuestStart;
        Line.DisplayDuration = 7.0f;
        Line.bHasBeenPlayed = false;
        RegisteredLines.Add(Line);
    }

    // CampKeeper — Danger trigger: flood warning
    {
        FNarr_DialogueLine Line;
        Line.LineID = TEXT("camp_keeper_flood_001");
        Line.SpeakerName = TEXT("CampKeeper");
        Line.SpeakerRole = ENarr_SpeakerRole::CampKeeper;
        Line.DialogueText = TEXT("We cannot stay here. The cave floods when the rains come. The water rises to my chest. We move the camp before the dark clouds reach us. Take what you can carry.");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782625739826_CampKeeper.mp3");
        Line.TriggerType = ENarr_DialogueTrigger::Danger;
        Line.DisplayDuration = 7.0f;
        Line.bHasBeenPlayed = false;
        RegisteredLines.Add(Line);
    }

    // RiverGuide — Discovery trigger: river crossing
    {
        FNarr_DialogueLine Line;
        Line.LineID = TEXT("river_guide_crossing_001");
        Line.SpeakerName = TEXT("RiverGuide");
        Line.SpeakerRole = ENarr_SpeakerRole::RiverGuide;
        Line.DialogueText = TEXT("You want to cross the river? Look at the current. Last season, two of our strongest swimmers tried at this point. Neither came back. There is a shallow crossing, three bends east.");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782625742553_RiverGuide.mp3");
        Line.TriggerType = ENarr_DialogueTrigger::Discovery;
        Line.DisplayDuration = 8.0f;
        Line.bHasBeenPlayed = false;
        RegisteredLines.Add(Line);
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueSystem: Loaded %d default dialogue lines"), RegisteredLines.Num());
}

void UNarr_DialogueSystem::CheckProximityTriggers()
{
    // Proximity check stub — implemented via Blueprint overlap events
    // NPCs with this component fire TriggerContextualDialogue on player overlap
}
