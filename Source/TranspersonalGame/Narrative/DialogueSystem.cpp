#include "DialogueSystem.h"
#include "Components/ActorComponent.h"

UDialogueSystem::UDialogueSystem()
    : bIsDialogueActive(false)
    , CurrentLineIndex(0)
    , ActiveSequence(nullptr)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
}

void UDialogueSystem::InitializeDefaultDialogues()
{
    // Canyon entrance warning — triggered when player approaches eastern ridge
    FNarr_DialogueSequence CanyonSeq;
    CanyonSeq.SequenceID = FName("SEQ_CanyonEntrance");
    CanyonSeq.bIsTriggered = false;

    FNarr_DialogueLine Line1;
    Line1.SpeakerID = TEXT("TribalElder");
    Line1.LineText = TEXT("Stay low. The canyon narrows ahead. Raptors nest on the eastern ridge.");
    Line1.Tone = ENarr_DialogueTone::Warning;
    Line1.DisplayDuration = 5.0f;

    FNarr_DialogueLine Line2;
    Line2.SpeakerID = TEXT("TribalElder");
    Line2.LineText = TEXT("We need flint from the riverbed. Move fast. Move quiet.");
    Line2.Tone = ENarr_DialogueTone::Urgent;
    Line2.DisplayDuration = 4.5f;

    CanyonSeq.Lines.Add(Line1);
    CanyonSeq.Lines.Add(Line2);
    DialogueLibrary.Add(CanyonSeq);

    // Riverbed crossing — resource gathering hint
    FNarr_DialogueSequence RiverSeq;
    RiverSeq.SequenceID = FName("SEQ_RiverbedCrossing");
    RiverSeq.bIsTriggered = false;

    FNarr_DialogueLine RLine1;
    RLine1.SpeakerID = TEXT("TribalElder");
    RLine1.LineText = TEXT("The river runs cold here. Flint and bone — everything we need is in the shallows.");
    RLine1.Tone = ENarr_DialogueTone::Informative;
    RLine1.DisplayDuration = 5.0f;

    FNarr_DialogueLine RLine2;
    RLine2.SpeakerID = TEXT("TribalElder");
    RLine2.LineText = TEXT("But the Brachiosaurus herd drinks at dusk. Do not startle them.");
    RLine2.Tone = ENarr_DialogueTone::Cautious;
    RLine2.DisplayDuration = 4.5f;

    RiverSeq.Lines.Add(RLine1);
    RiverSeq.Lines.Add(RLine2);
    DialogueLibrary.Add(RiverSeq);

    // Dino nest area — danger warning
    FNarr_DialogueSequence NestSeq;
    NestSeq.SequenceID = FName("SEQ_DinoNestArea");
    NestSeq.bIsTriggered = false;

    FNarr_DialogueLine NLine1;
    NLine1.SpeakerID = TEXT("TribalElder");
    NLine1.LineText = TEXT("Eggs. Fresh ones. The mother is close — I can smell her.");
    NLine1.Tone = ENarr_DialogueTone::Warning;
    NLine1.DisplayDuration = 4.0f;

    FNarr_DialogueLine NLine2;
    NLine2.SpeakerID = TEXT("TribalElder");
    NLine2.LineText = TEXT("Back away slowly. Do not run. Running is death.");
    NLine2.Tone = ENarr_DialogueTone::Urgent;
    NLine2.DisplayDuration = 5.0f;

    NestSeq.Lines.Add(NLine1);
    NestSeq.Lines.Add(NLine2);
    DialogueLibrary.Add(NestSeq);
}

void UDialogueSystem::StartDialogueSequence(FName SequenceID)
{
    for (FNarr_DialogueSequence& Seq : DialogueLibrary)
    {
        if (Seq.SequenceID == SequenceID && !Seq.bIsTriggered)
        {
            ActiveSequence = &Seq;
            CurrentLineIndex = 0;
            bIsDialogueActive = true;
            Seq.bIsTriggered = true;
            return;
        }
    }
    // Sequence not found or already triggered
    bIsDialogueActive = false;
    ActiveSequence = nullptr;
}

void UDialogueSystem::AdvanceLine()
{
    if (!bIsDialogueActive || !ActiveSequence)
    {
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveSequence->Lines.Num())
    {
        EndDialogue();
    }
}

void UDialogueSystem::EndDialogue()
{
    bIsDialogueActive = false;
    ActiveSequence = nullptr;
    CurrentLineIndex = 0;
}

FNarr_DialogueLine UDialogueSystem::GetCurrentLine() const
{
    if (bIsDialogueActive && ActiveSequence &&
        CurrentLineIndex < ActiveSequence->Lines.Num())
    {
        return ActiveSequence->Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

bool UDialogueSystem::HasMoreLines() const
{
    if (!bIsDialogueActive || !ActiveSequence)
    {
        return false;
    }
    return CurrentLineIndex < ActiveSequence->Lines.Num() - 1;
}

void UDialogueSystem::RegisterSequence(FNarr_DialogueSequence NewSequence)
{
    // Check for duplicate SequenceID
    for (const FNarr_DialogueSequence& Existing : DialogueLibrary)
    {
        if (Existing.SequenceID == NewSequence.SequenceID)
        {
            return; // Already registered
        }
    }
    DialogueLibrary.Add(NewSequence);
}
