#include "NarrativeDialogueSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// Constructor
// ============================================================
UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms — not every frame
}

// ============================================================
// BeginPlay
// ============================================================
void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    bSequencePlaying = false;
    LineTimer = 0.0f;
}

// ============================================================
// TickComponent — auto-advance lines based on display duration
// ============================================================
void UNarr_DialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bSequencePlaying) return;

    LineTimer += DeltaTime;

    FNarr_DialogueSequence* Seq = FindSequence(ActiveSequenceID);
    if (!Seq || Seq->Lines.Num() == 0) return;

    int32 Idx = Seq->CurrentLineIndex;
    if (!Seq->Lines.IsValidIndex(Idx)) return;

    float Duration = Seq->Lines[Idx].DisplayDuration;
    if (LineTimer >= Duration)
    {
        LineTimer = 0.0f;
        AdvanceLine();
    }
}

// ============================================================
// PlaySequence
// ============================================================
void UNarr_DialogueComponent::PlaySequence(FName SequenceID)
{
    FNarr_DialogueSequence* Seq = FindSequence(SequenceID);
    if (!Seq)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogue: Sequence '%s' not found on %s"),
            *SequenceID.ToString(), *GetOwner()->GetName());
        return;
    }

    ActiveSequenceID = SequenceID;
    Seq->CurrentLineIndex = 0;
    bSequencePlaying = true;
    LineTimer = 0.0f;

    // Mark first line as played if one-shot
    if (Seq->Lines.IsValidIndex(0) && Seq->Lines[0].bPlayOnce)
    {
        Seq->Lines[0].bHasPlayed = true;
    }

    // Play voice audio if assigned
    if (Seq->Lines.IsValidIndex(0) && Seq->Lines[0].VoiceAudio)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Seq->Lines[0].VoiceAudio,
            GetOwner()->GetActorLocation());
    }

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Playing sequence '%s' on %s"),
        *SequenceID.ToString(), *GetOwner()->GetName());
}

// ============================================================
// AdvanceLine
// ============================================================
void UNarr_DialogueComponent::AdvanceLine()
{
    FNarr_DialogueSequence* Seq = FindSequence(ActiveSequenceID);
    if (!Seq) return;

    Seq->CurrentLineIndex++;

    if (Seq->CurrentLineIndex >= Seq->Lines.Num())
    {
        // Sequence complete
        if (Seq->bLoopSequence)
        {
            Seq->CurrentLineIndex = 0;
        }
        else
        {
            bSequencePlaying = false;
            ActiveSequenceID = NAME_None;
            UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Sequence complete"));
            return;
        }
    }

    int32 Idx = Seq->CurrentLineIndex;
    if (!Seq->Lines.IsValidIndex(Idx)) return;

    FNarr_DialogueLine& Line = Seq->Lines[Idx];

    // Skip one-shot lines already played
    if (Line.bPlayOnce && Line.bHasPlayed)
    {
        AdvanceLine();
        return;
    }

    if (Line.bPlayOnce)
    {
        Line.bHasPlayed = true;
    }

    // Play voice audio
    if (Line.VoiceAudio)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Line.VoiceAudio,
            GetOwner()->GetActorLocation());
    }
}

// ============================================================
// StopSequence
// ============================================================
void UNarr_DialogueComponent::StopSequence()
{
    bSequencePlaying = false;
    LineTimer = 0.0f;
    ActiveSequenceID = NAME_None;
}

// ============================================================
// GetCurrentLineText
// ============================================================
FText UNarr_DialogueComponent::GetCurrentLineText() const
{
    const FNarr_DialogueSequence* Seq = nullptr;
    for (const FNarr_DialogueSequence& S : DialogueSequences)
    {
        if (S.SequenceID == ActiveSequenceID)
        {
            Seq = &S;
            break;
        }
    }

    if (!Seq || !Seq->Lines.IsValidIndex(Seq->CurrentLineIndex))
    {
        return FText::GetEmpty();
    }

    return Seq->Lines[Seq->CurrentLineIndex].LineText;
}

// ============================================================
// IsSequenceComplete
// ============================================================
bool UNarr_DialogueComponent::IsSequenceComplete(FName SequenceID) const
{
    for (const FNarr_DialogueSequence& Seq : DialogueSequences)
    {
        if (Seq.SequenceID == SequenceID)
        {
            return !Seq.bLoopSequence && (Seq.CurrentLineIndex >= Seq.Lines.Num());
        }
    }
    return false;
}

// ============================================================
// TriggerByCondition
// ============================================================
void UNarr_DialogueComponent::TriggerByCondition(ENarr_DialogueCondition Condition)
{
    for (FNarr_DialogueSequence& Seq : DialogueSequences)
    {
        for (const FNarr_DialogueLine& Line : Seq.Lines)
        {
            if (Line.Condition == Condition && !(Line.bPlayOnce && Line.bHasPlayed))
            {
                PlaySequence(Seq.SequenceID);
                return;
            }
        }
    }
}

// ============================================================
// FindSequence (private)
// ============================================================
FNarr_DialogueSequence* UNarr_DialogueComponent::FindSequence(FName SequenceID)
{
    for (FNarr_DialogueSequence& Seq : DialogueSequences)
    {
        if (Seq.SequenceID == SequenceID)
        {
            return &Seq;
        }
    }
    return nullptr;
}
