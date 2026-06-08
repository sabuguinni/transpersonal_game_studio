#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentLineIndex = 0;
    bIsDialogueActive = false;
    DefaultLineDuration = 3.0f;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultSequences();
}

bool UNarr_DialogueSystem::StartDialogueSequence(const FString& SequenceID)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already active. Cannot start new sequence."));
        return false;
    }

    FNarr_DialogueSequence* FoundSequence = FindSequenceByID(SequenceID);
    if (!FoundSequence)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
        return false;
    }

    if (FoundSequence->DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence has no lines: %s"), *SequenceID);
        return false;
    }

    CurrentSequence = *FoundSequence;
    CurrentLineIndex = 0;
    bIsDialogueActive = true;

    UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
    return true;
}

void UNarr_DialogueSystem::AdvanceDialogue()
{
    if (!bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("No active dialogue to advance."));
        return;
    }

    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        EndDialogue();
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Advanced to dialogue line %d"), CurrentLineIndex);
}

void UNarr_DialogueSystem::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    bIsDialogueActive = false;
    CurrentLineIndex = 0;
    
    // Mark sequence as completed if it's not repeatable
    if (!CurrentSequence.bCanRepeat)
    {
        FNarr_DialogueSequence* OriginalSequence = FindSequenceByID(CurrentSequence.SequenceID);
        if (OriginalSequence)
        {
            OriginalSequence->bIsCompleted = true;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Ended dialogue sequence: %s"), *CurrentSequence.SequenceID);
}

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentLine() const
{
    if (!bIsDialogueActive || CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        return FNarr_DialogueLine();
    }

    return CurrentSequence.DialogueLines[CurrentLineIndex];
}

bool UNarr_DialogueSystem::IsDialogueActive() const
{
    return bIsDialogueActive;
}

void UNarr_DialogueSystem::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    // Check if sequence already exists
    for (int32 i = 0; i < AvailableSequences.Num(); i++)
    {
        if (AvailableSequences[i].SequenceID == NewSequence.SequenceID)
        {
            AvailableSequences[i] = NewSequence;
            UE_LOG(LogTemp, Log, TEXT("Updated existing dialogue sequence: %s"), *NewSequence.SequenceID);
            return;
        }
    }

    // Add new sequence
    AvailableSequences.Add(NewSequence);
    UE_LOG(LogTemp, Log, TEXT("Added new dialogue sequence: %s"), *NewSequence.SequenceID);
}

TArray<FString> UNarr_DialogueSystem::GetAvailableSequenceIDs() const
{
    TArray<FString> SequenceIDs;
    for (const FNarr_DialogueSequence& Sequence : AvailableSequences)
    {
        if (!Sequence.bIsCompleted || Sequence.bCanRepeat)
        {
            SequenceIDs.Add(Sequence.SequenceID);
        }
    }
    return SequenceIDs;
}

void UNarr_DialogueSystem::InitializeDefaultSequences()
{
    // Create survival warning sequence
    FNarr_DialogueSequence SurvivalWarning;
    SurvivalWarning.SequenceID = TEXT("survival_warning_01");
    SurvivalWarning.bCanRepeat = true;
    SurvivalWarning.bIsCompleted = false;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Elder");
    Line1.DialogueText = FText::FromString(TEXT("The ancient hunting grounds echo with danger, survivor."));
    Line1.AudioAssetPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1780895462435_TribalElder.mp3");
    Line1.Duration = 14.0f;
    Line1.bIsPlayerChoice = false;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Elder");
    Line2.DialogueText = FText::FromString(TEXT("The great beasts have marked this territory as their own. Tread carefully."));
    Line2.AudioAssetPath = TEXT("");
    Line2.Duration = 4.0f;
    Line2.bIsPlayerChoice = false;

    SurvivalWarning.DialogueLines.Add(Line1);
    SurvivalWarning.DialogueLines.Add(Line2);
    AvailableSequences.Add(SurvivalWarning);

    // Create scout warning sequence
    FNarr_DialogueSequence ScoutWarning;
    ScoutWarning.SequenceID = TEXT("scout_warning_01");
    ScoutWarning.bCanRepeat = true;
    ScoutWarning.bIsCompleted = false;

    FNarr_DialogueLine ScoutLine1;
    ScoutLine1.SpeakerName = TEXT("Scout");
    ScoutLine1.DialogueText = FText::FromString(TEXT("Movement detected near the river crossing! Three large predators approach from the north."));
    ScoutLine1.AudioAssetPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1780895468045_Scout.mp3");
    ScoutLine1.Duration = 15.0f;
    ScoutLine1.bIsPlayerChoice = false;

    FNarr_DialogueLine ScoutLine2;
    ScoutLine2.SpeakerName = TEXT("Scout");
    ScoutLine2.DialogueText = FText::FromString(TEXT("Stay low, avoid the open ground. The pack hunters work together."));
    ScoutLine2.AudioAssetPath = TEXT("");
    ScoutLine2.Duration = 4.0f;
    ScoutLine2.bIsPlayerChoice = false;

    ScoutWarning.DialogueLines.Add(ScoutLine1);
    ScoutWarning.DialogueLines.Add(ScoutLine2);
    AvailableSequences.Add(ScoutWarning);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d default dialogue sequences"), AvailableSequences.Num());
}

FNarr_DialogueSequence* UNarr_DialogueSystem::FindSequenceByID(const FString& SequenceID)
{
    for (FNarr_DialogueSequence& Sequence : AvailableSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            return &Sequence;
        }
    }
    return nullptr;
}