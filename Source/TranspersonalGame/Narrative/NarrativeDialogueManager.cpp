#include "NarrativeDialogueManager.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ANarrativeDialogueManager::ANarrativeDialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;
    bDialogueActive = false;
    DialogueTimeRemaining = 0.0f;
    ActiveSequenceIndex = -1;
}

void ANarrativeDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogue();
}

void ANarrativeDialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDialogueActive && DialogueTimeRemaining > 0.0f)
    {
        DialogueTimeRemaining -= DeltaTime;
        if (DialogueTimeRemaining <= 0.0f)
        {
            OnDialogueLineComplete();
        }
    }
}

void ANarrativeDialogueManager::InitializeDefaultDialogue()
{
    // --- Sequence: Raptor Warning (Tribal Leader Kora) ---
    FNarr_DialogueSequence RaptorWarning;
    RaptorWarning.SequenceID = TEXT("SEQ_RaptorWarning");
    RaptorWarning.bIsActive = false;
    RaptorWarning.CurrentLineIndex = 0;

    FNarr_DialogueLine Line_Kora1;
    Line_Kora1.LineID = TEXT("KORA_001");
    Line_Kora1.SpeakerName = TEXT("Kora");
    Line_Kora1.SpeakerType = ENarr_DialogueSpeaker::TribalLeader;
    Line_Kora1.DialogueText = TEXT("The eastern ridge is raptor territory now. We lost two scouts last moon cycle. Anyone who crosses that valley alone does not come back.");
    Line_Kora1.TriggerCondition = ENarr_DialogueTrigger::OnQuestStart;
    Line_Kora1.DisplayDuration = 9.0f;
    Line_Kora1.bHasBeenPlayed = false;
    RaptorWarning.Lines.Add(Line_Kora1);

    DialogueSequences.Add(RaptorWarning);

    // --- Sequence: Triceratops Tracking (Scout Davan) ---
    FNarr_DialogueSequence TriceratopsTracking;
    TriceratopsTracking.SequenceID = TEXT("SEQ_TriceratopsTracking");
    TriceratopsTracking.bIsActive = false;
    TriceratopsTracking.CurrentLineIndex = 0;

    FNarr_DialogueLine Line_Davan1;
    Line_Davan1.LineID = TEXT("DAVAN_001");
    Line_Davan1.SpeakerName = TEXT("Davan");
    Line_Davan1.SpeakerType = ENarr_DialogueSpeaker::Scout;
    Line_Davan1.DialogueText = TEXT("We have tracked the Triceratops herd for three days. They know where the water is. Follow them, stay downwind, and we will survive the dry season.");
    Line_Davan1.TriggerCondition = ENarr_DialogueTrigger::OnDinosaurSeen;
    Line_Davan1.DisplayDuration = 10.0f;
    Line_Davan1.bHasBeenPlayed = false;
    TriceratopsTracking.Lines.Add(Line_Davan1);

    DialogueSequences.Add(TriceratopsTracking);

    // --- Sequence: TRex Knowledge (Elder Maren) ---
    FNarr_DialogueSequence TRexKnowledge;
    TRexKnowledge.SequenceID = TEXT("SEQ_TRexKnowledge");
    TRexKnowledge.bIsActive = false;
    TRexKnowledge.CurrentLineIndex = 0;

    FNarr_DialogueLine Line_Maren1;
    Line_Maren1.LineID = TEXT("MAREN_001");
    Line_Maren1.SpeakerName = TEXT("Maren");
    Line_Maren1.SpeakerType = ENarr_DialogueSpeaker::Elder;
    Line_Maren1.DialogueText = TEXT("The T-Rex does not hunt by sight alone. It reads the ground — your footprints, your warmth, the way the grass bends when you pass. You cannot hide from it. You can only make it choose a different meal.");
    Line_Maren1.TriggerCondition = ENarr_DialogueTrigger::OnDinosaurSeen;
    Line_Maren1.DisplayDuration = 14.0f;
    Line_Maren1.bHasBeenPlayed = false;
    TRexKnowledge.Lines.Add(Line_Maren1);

    DialogueSequences.Add(TRexKnowledge);

    // --- Sequence: Fire Warning (Hunter Brek) ---
    FNarr_DialogueSequence FireWarning;
    FireWarning.SequenceID = TEXT("SEQ_FireWarning");
    FireWarning.bIsActive = false;
    FireWarning.CurrentLineIndex = 0;

    FNarr_DialogueLine Line_Brek1;
    Line_Brek1.LineID = TEXT("BREK_001");
    Line_Brek1.SpeakerName = TEXT("Brek");
    Line_Brek1.SpeakerType = ENarr_DialogueSpeaker::Hunter;
    Line_Brek1.DialogueText = TEXT("Fire. That is the only thing they fear. Keep it burning through the night. Let it go out and you will not see morning.");
    Line_Brek1.TriggerCondition = ENarr_DialogueTrigger::OnNightfall;
    Line_Brek1.DisplayDuration = 8.0f;
    Line_Brek1.bHasBeenPlayed = false;
    FireWarning.Lines.Add(Line_Brek1);

    DialogueSequences.Add(FireWarning);

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Initialized %d dialogue sequences"), DialogueSequences.Num());
}

void ANarrativeDialogueManager::TriggerDialogueSequence(const FString& SequenceID)
{
    for (int32 i = 0; i < DialogueSequences.Num(); i++)
    {
        if (DialogueSequences[i].SequenceID == SequenceID)
        {
            if (bDialogueActive)
            {
                SkipDialogue();
            }
            DialogueSequences[i].bIsActive = true;
            DialogueSequences[i].CurrentLineIndex = 0;
            ActiveSequenceIndex = i;

            if (DialogueSequences[i].Lines.Num() > 0)
            {
                PlayDialogueLine(DialogueSequences[i].Lines[0]);
            }
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Sequence '%s' not found"), *SequenceID);
}

void ANarrativeDialogueManager::TriggerDialogueByCondition(ENarr_DialogueTrigger Trigger)
{
    for (int32 i = 0; i < DialogueSequences.Num(); i++)
    {
        FNarr_DialogueSequence& Seq = DialogueSequences[i];
        if (!Seq.bIsActive && Seq.Lines.Num() > 0)
        {
            for (const FNarr_DialogueLine& Line : Seq.Lines)
            {
                if (Line.TriggerCondition == Trigger && !Line.bHasBeenPlayed)
                {
                    TriggerDialogueSequence(Seq.SequenceID);
                    return;
                }
            }
        }
    }
}

void ANarrativeDialogueManager::AdvanceDialogue()
{
    if (ActiveSequenceIndex < 0 || ActiveSequenceIndex >= DialogueSequences.Num())
    {
        return;
    }

    FNarr_DialogueSequence& ActiveSeq = DialogueSequences[ActiveSequenceIndex];
    ActiveSeq.CurrentLineIndex++;

    if (ActiveSeq.CurrentLineIndex < ActiveSeq.Lines.Num())
    {
        PlayDialogueLine(ActiveSeq.Lines[ActiveSeq.CurrentLineIndex]);
    }
    else
    {
        // Sequence complete
        ActiveSeq.bIsActive = false;
        bDialogueActive = false;
        ActiveSequenceIndex = -1;
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Sequence complete"));
    }
}

void ANarrativeDialogueManager::SkipDialogue()
{
    GetWorldTimerManager().ClearTimer(DialogueTimerHandle);
    bDialogueActive = false;
    DialogueTimeRemaining = 0.0f;
}

void ANarrativeDialogueManager::RegisterDialogueLine(const FString& SequenceID, const FNarr_DialogueLine& Line)
{
    for (FNarr_DialogueSequence& Seq : DialogueSequences)
    {
        if (Seq.SequenceID == SequenceID)
        {
            Seq.Lines.Add(Line);
            return;
        }
    }

    // Create new sequence if not found
    FNarr_DialogueSequence NewSeq;
    NewSeq.SequenceID = SequenceID;
    NewSeq.bIsActive = false;
    NewSeq.CurrentLineIndex = 0;
    NewSeq.Lines.Add(Line);
    DialogueSequences.Add(NewSeq);
}

TArray<FNarr_DialogueLine> ANarrativeDialogueManager::GetLinesForSpeaker(ENarr_DialogueSpeaker Speaker)
{
    TArray<FNarr_DialogueLine> Result;
    for (const FNarr_DialogueSequence& Seq : DialogueSequences)
    {
        for (const FNarr_DialogueLine& Line : Seq.Lines)
        {
            if (Line.SpeakerType == Speaker)
            {
                Result.Add(Line);
            }
        }
    }
    return Result;
}

bool ANarrativeDialogueManager::IsSequenceComplete(const FString& SequenceID) const
{
    for (const FNarr_DialogueSequence& Seq : DialogueSequences)
    {
        if (Seq.SequenceID == SequenceID)
        {
            return !Seq.bIsActive && Seq.CurrentLineIndex >= Seq.Lines.Num();
        }
    }
    return false;
}

void ANarrativeDialogueManager::PlayDialogueLine(const FNarr_DialogueLine& Line)
{
    ActiveDialogueLine = Line;
    bDialogueActive = true;
    DialogueTimeRemaining = Line.DisplayDuration;

    // Play voice audio if assigned
    if (Line.VoiceAudio && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Line.VoiceAudio);
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: [%s] %s"), *Line.SpeakerName, *Line.DialogueText);
}

void ANarrativeDialogueManager::OnDialogueLineComplete()
{
    if (ActiveSequenceIndex >= 0 && ActiveSequenceIndex < DialogueSequences.Num())
    {
        FNarr_DialogueSequence& ActiveSeq = DialogueSequences[ActiveSequenceIndex];
        if (ActiveSeq.CurrentLineIndex < ActiveSeq.Lines.Num())
        {
            ActiveSeq.Lines[ActiveSeq.CurrentLineIndex].bHasBeenPlayed = true;
        }
    }
    AdvanceDialogue();
}
