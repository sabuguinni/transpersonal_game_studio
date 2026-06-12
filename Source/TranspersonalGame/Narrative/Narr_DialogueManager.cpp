#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    bIsPlaying = false;
    ProximityRange = 500.0f;
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadDefaultDialogues();
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue Manager initialized"));
}

void UNarr_DialogueManager::StartDialogueSequence(const FString& SequenceID)
{
    if (bIsPlaying)
    {
        StopCurrentDialogue();
    }

    if (DialogueDatabase.Contains(SequenceID))
    {
        CurrentSequenceID = SequenceID;
        CurrentLineIndex = 0;
        bIsPlaying = true;
        PlayNextLine();
        UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
    }
}

void UNarr_DialogueManager::StopCurrentDialogue()
{
    if (bIsPlaying)
    {
        bIsPlaying = false;
        CurrentSequenceID = TEXT("");
        CurrentLineIndex = 0;
        UE_LOG(LogTemp, Log, TEXT("Stopped current dialogue"));
    }
}

bool UNarr_DialogueManager::IsDialoguePlaying() const
{
    return bIsPlaying;
}

void UNarr_DialogueManager::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueDatabase.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s"), *Sequence.SequenceID);
}

void UNarr_DialogueManager::TriggerContextualDialogue(const FString& Context, const FVector& Location)
{
    // Context-based dialogue triggering for environmental storytelling
    FString ContextSequenceID = FString::Printf(TEXT("Context_%s"), *Context);
    
    if (DialogueDatabase.Contains(ContextSequenceID))
    {
        StartDialogueSequence(ContextSequenceID);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("No contextual dialogue found for: %s"), *Context);
    }
}

void UNarr_DialogueManager::PlayNextLine()
{
    if (!bIsPlaying || !DialogueDatabase.Contains(CurrentSequenceID))
    {
        return;
    }

    const FNarr_DialogueSequence& CurrentSequence = DialogueDatabase[CurrentSequenceID];
    
    if (CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        // Sequence completed
        StopCurrentDialogue();
        return;
    }

    const FNarr_DialogueLine& CurrentLine = CurrentSequence.DialogueLines[CurrentLineIndex];
    
    // Display dialogue text (would integrate with UI system)
    UE_LOG(LogTemp, Log, TEXT("[%s]: %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
    
    // Set timer for next line
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        this,
        &UNarr_DialogueManager::OnLineCompleted,
        CurrentLine.Duration,
        false
    );
}

void UNarr_DialogueManager::OnLineCompleted()
{
    CurrentLineIndex++;
    PlayNextLine();
}

void UNarr_DialogueManager::LoadDefaultDialogues()
{
    // Hunter Warning Sequence
    FNarr_DialogueSequence HunterWarning;
    HunterWarning.SequenceID = TEXT("HunterWarning");
    HunterWarning.bIsRepeatable = false;
    HunterWarning.bRequiresProximity = true;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Ancient Hunter");
    Line1.DialogueText = TEXT("The ancient hunter crouches by the riverbank, his weathered hands tracing fresh claw marks in the mud.");
    Line1.Duration = 4.0f;
    HunterWarning.DialogueLines.Add(Line1);

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Ancient Hunter");
    Line2.DialogueText = TEXT("Three claws, deep gouges... This is no ordinary predator. The great beast has awakened from its slumber.");
    Line2.Duration = 5.0f;
    HunterWarning.DialogueLines.Add(Line2);

    RegisterDialogueSequence(HunterWarning);

    // Tribe Warning Sequence
    FNarr_DialogueSequence TribeWarning;
    TribeWarning.SequenceID = TEXT("TribeWarning");
    TribeWarning.bIsRepeatable = true;
    TribeWarning.bRequiresProximity = false;

    FNarr_DialogueLine WarningLine;
    WarningLine.SpeakerName = TEXT("Tribe Lookout");
    WarningLine.DialogueText = TEXT("Danger approaches from the north! The ground trembles beneath massive footsteps. Seek high ground immediately!");
    WarningLine.Duration = 4.5f;
    TribeWarning.DialogueLines.Add(WarningLine);

    RegisterDialogueSequence(TribeWarning);

    // Contextual dialogues for environmental storytelling
    FNarr_DialogueSequence RiverContext;
    RiverContext.SequenceID = TEXT("Context_River");
    RiverContext.bIsRepeatable = true;
    RiverContext.bRequiresProximity = true;

    FNarr_DialogueLine RiverLine;
    RiverLine.SpeakerName = TEXT("Narrator");
    RiverLine.DialogueText = TEXT("The river runs red with the blood of recent kills. Something large has been hunting here.");
    RiverLine.Duration = 3.5f;
    RiverContext.DialogueLines.Add(RiverLine);

    RegisterDialogueSequence(RiverContext);
}