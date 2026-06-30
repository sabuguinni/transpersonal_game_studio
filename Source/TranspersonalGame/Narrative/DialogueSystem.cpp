#include "DialogueSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;
    bDialogueActive = false;
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    bAutoAdvance = true;
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeVoiceLineRegistry();
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

void ANarr_DialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDialogueActive || !bAutoAdvance)
    {
        return;
    }

    DialogueTimer -= DeltaTime;
    if (DialogueTimer <= 0.0f)
    {
        AdvanceDialogue();
    }
}

// ─── StartDialogueSequence ────────────────────────────────────────────────────

bool ANarr_DialogueManager::StartDialogueSequence(FName SequenceID)
{
    int32 Idx = FindSequenceIndex(SequenceID);
    if (Idx == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("ANarr_DialogueManager: Sequence '%s' not found."), *SequenceID.ToString());
        return false;
    }

    FNarr_DialogueSequence& Seq = RegisteredSequences[Idx];

    // Don't replay non-repeatable sequences
    if (Seq.bHasBeenPlayed && !Seq.bIsRepeatable)
    {
        return false;
    }

    if (Seq.Lines.Num() == 0)
    {
        return false;
    }

    ActiveSequenceID = SequenceID;
    CurrentLineIndex = 0;
    bDialogueActive = true;

    // Set timer for first line
    DialogueTimer = Seq.Lines[0].DisplayDuration;

    UE_LOG(LogTemp, Log, TEXT("ANarr_DialogueManager: Starting sequence '%s' (%d lines)"),
        *SequenceID.ToString(), Seq.Lines.Num());

    return true;
}

// ─── AdvanceDialogue ──────────────────────────────────────────────────────────

bool ANarr_DialogueManager::AdvanceDialogue()
{
    if (!bDialogueActive)
    {
        return false;
    }

    int32 SeqIdx = FindSequenceIndex(ActiveSequenceID);
    if (SeqIdx == INDEX_NONE)
    {
        EndDialogue();
        return false;
    }

    FNarr_DialogueSequence& Seq = RegisteredSequences[SeqIdx];
    CurrentLineIndex++;

    if (CurrentLineIndex >= Seq.Lines.Num())
    {
        // Sequence complete
        Seq.bHasBeenPlayed = true;
        EndDialogue();
        return false;
    }

    // Check result of previous line
    const FNarr_DialogueLine& PrevLine = Seq.Lines[CurrentLineIndex - 1];
    if (PrevLine.Result == ENarr_DialogueResult::End)
    {
        Seq.bHasBeenPlayed = true;
        EndDialogue();
        return false;
    }

    // Set timer for next line
    DialogueTimer = Seq.Lines[CurrentLineIndex].DisplayDuration;
    return true;
}

// ─── EndDialogue ──────────────────────────────────────────────────────────────

void ANarr_DialogueManager::EndDialogue()
{
    bDialogueActive = false;
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    ActiveSequenceID = NAME_None;

    UE_LOG(LogTemp, Log, TEXT("ANarr_DialogueManager: Dialogue ended."));
}

// ─── GetCurrentLine ───────────────────────────────────────────────────────────

bool ANarr_DialogueManager::GetCurrentLine(FNarr_DialogueLine& OutLine) const
{
    if (!bDialogueActive)
    {
        return false;
    }

    int32 SeqIdx = FindSequenceIndex(ActiveSequenceID);
    if (SeqIdx == INDEX_NONE)
    {
        return false;
    }

    const FNarr_DialogueSequence& Seq = RegisteredSequences[SeqIdx];
    if (!Seq.Lines.IsValidIndex(CurrentLineIndex))
    {
        return false;
    }

    OutLine = Seq.Lines[CurrentLineIndex];
    return true;
}

// ─── RegisterVoiceLine ────────────────────────────────────────────────────────

void ANarr_DialogueManager::RegisterVoiceLine(FName LineID,
    ENarr_DialogueSpeaker Speaker,
    const FString& AudioURL,
    float Duration,
    ENarr_DialogueContext Context)
{
    // Check for duplicate
    for (FNarr_VoiceLineRegistry& Entry : VoiceLineRegistry)
    {
        if (Entry.LineID == LineID)
        {
            Entry.AudioURL = AudioURL;
            Entry.DurationSeconds = Duration;
            UE_LOG(LogTemp, Log, TEXT("ANarr_DialogueManager: Updated voice line '%s'"), *LineID.ToString());
            return;
        }
    }

    FNarr_VoiceLineRegistry NewEntry;
    NewEntry.LineID = LineID;
    NewEntry.Speaker = Speaker;
    NewEntry.AudioURL = AudioURL;
    NewEntry.DurationSeconds = Duration;
    NewEntry.Context = Context;
    VoiceLineRegistry.Add(NewEntry);

    UE_LOG(LogTemp, Log, TEXT("ANarr_DialogueManager: Registered voice line '%s' (%.1fs)"),
        *LineID.ToString(), Duration);
}

// ─── GetVoiceLineURL ──────────────────────────────────────────────────────────

FString ANarr_DialogueManager::GetVoiceLineURL(FName LineID) const
{
    for (const FNarr_VoiceLineRegistry& Entry : VoiceLineRegistry)
    {
        if (Entry.LineID == LineID)
        {
            return Entry.AudioURL;
        }
    }
    return FString();
}

// ─── TriggerContextDialogue ───────────────────────────────────────────────────

bool ANarr_DialogueManager::TriggerContextDialogue(ENarr_DialogueContext Context)
{
    if (bDialogueActive)
    {
        // Don't interrupt active dialogue
        return false;
    }

    // Find first matching non-played sequence for this context
    for (const FNarr_DialogueSequence& Seq : RegisteredSequences)
    {
        if (Seq.RequiredContext == Context)
        {
            if (!Seq.bHasBeenPlayed || Seq.bIsRepeatable)
            {
                return StartDialogueSequence(Seq.SequenceID);
            }
        }
    }

    return false;
}

// ─── HasSequenceBeenPlayed ────────────────────────────────────────────────────

bool ANarr_DialogueManager::HasSequenceBeenPlayed(FName SequenceID) const
{
    int32 Idx = FindSequenceIndex(SequenceID);
    if (Idx == INDEX_NONE)
    {
        return false;
    }
    return RegisteredSequences[Idx].bHasBeenPlayed;
}

// ─── FindSequenceIndex ────────────────────────────────────────────────────────

int32 ANarr_DialogueManager::FindSequenceIndex(FName SequenceID) const
{
    for (int32 i = 0; i < RegisteredSequences.Num(); i++)
    {
        if (RegisteredSequences[i].SequenceID == SequenceID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

// ─── InitializeVoiceLineRegistry ─────────────────────────────────────────────

void ANarr_DialogueManager::InitializeVoiceLineRegistry()
{
    // Cycle AUTO_20260630_002 — ElevenLabs TTS voice lines
    // These URLs are live audio from the production pipeline

    RegisterVoiceLine(
        FName("TL_MigrationOrder"),
        ENarr_DialogueSpeaker::TribalLeader,
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782785903459_TribalLeader.mp3"),
        9.0f,
        ENarr_DialogueContext::Migration
    );

    RegisterVoiceLine(
        FName("SS_RexWarning"),
        ENarr_DialogueSpeaker::SurvivorScout,
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782785921355_SurvivorScout.mp3"),
        8.0f,
        ENarr_DialogueContext::Danger
    );

    RegisterVoiceLine(
        FName("EH_RiverWarning"),
        ENarr_DialogueSpeaker::ElderHunter,
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782785927646_ElderHunter.mp3"),
        11.0f,
        ENarr_DialogueContext::Discovery
    );

    RegisterVoiceLine(
        FName("YT_BigTracks"),
        ENarr_DialogueSpeaker::YoungTracker,
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782785929604_YoungTracker.mp3"),
        12.0f,
        ENarr_DialogueContext::Discovery
    );

    UE_LOG(LogTemp, Log, TEXT("ANarr_DialogueManager: Initialized %d voice lines"), VoiceLineRegistry.Num());
}
