#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Engine/World.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bDialogueActive = false;
    CurrentSequenceIndex = -1;
    CurrentEntryIndex = -1;
    DialogueSpeed = 1.0f;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UNarr_DialogueSystem::StartDialogueSequence(const FString& SequenceName)
{
    // Find the dialogue sequence by name
    for (int32 i = 0; i < DialogueSequences.Num(); i++)
    {
        if (DialogueSequences[i].SequenceName == SequenceName)
        {
            CurrentSequenceIndex = i;
            CurrentEntryIndex = 0;
            bDialogueActive = true;

            if (DialogueSequences[i].DialogueEntries.Num() > 0)
            {
                ProcessNextDialogueEntry();
            }
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence '%s' not found"), *SequenceName);
}

void UNarr_DialogueSystem::StopDialogue()
{
    bDialogueActive = false;
    CurrentSequenceIndex = -1;
    CurrentEntryIndex = -1;

    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DialogueTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
}

void UNarr_DialogueSystem::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    DialogueSequences.Add(NewSequence);
}

void UNarr_DialogueSystem::PlayNarration(const FString& NarrationText, float Duration)
{
    FNarr_DialogueEntry NarrationEntry;
    NarrationEntry.SpeakerName = TEXT("Narrator");
    NarrationEntry.DialogueText = NarrationText;
    NarrationEntry.DisplayDuration = Duration;
    NarrationEntry.bIsNarration = true;

    FNarr_DialogueSequence TempSequence;
    TempSequence.SequenceName = TEXT("TempNarration");
    TempSequence.DialogueEntries.Add(NarrationEntry);
    TempSequence.bAutoPlay = true;

    DialogueSequences.Add(TempSequence);
    StartDialogueSequence(TEXT("TempNarration"));
}

FNarr_DialogueEntry UNarr_DialogueSystem::GetCurrentDialogueEntry() const
{
    if (bDialogueActive && CurrentSequenceIndex >= 0 && CurrentEntryIndex >= 0)
    {
        if (DialogueSequences.IsValidIndex(CurrentSequenceIndex))
        {
            const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceIndex];
            if (CurrentSequence.DialogueEntries.IsValidIndex(CurrentEntryIndex))
            {
                return CurrentSequence.DialogueEntries[CurrentEntryIndex];
            }
        }
    }

    return FNarr_DialogueEntry();
}

void UNarr_DialogueSystem::ProcessNextDialogueEntry()
{
    if (!bDialogueActive || CurrentSequenceIndex < 0 || CurrentEntryIndex < 0)
    {
        return;
    }

    if (!DialogueSequences.IsValidIndex(CurrentSequenceIndex))
    {
        StopDialogue();
        return;
    }

    const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceIndex];
    
    if (!CurrentSequence.DialogueEntries.IsValidIndex(CurrentEntryIndex))
    {
        StopDialogue();
        return;
    }

    const FNarr_DialogueEntry& CurrentEntry = CurrentSequence.DialogueEntries[CurrentEntryIndex];

    // Display the dialogue entry
    FString DisplayText = FString::Printf(TEXT("%s: %s"), *CurrentEntry.SpeakerName, *CurrentEntry.DialogueText);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, CurrentEntry.DisplayDuration, FColor::Cyan, DisplayText);
    }

    UE_LOG(LogTemp, Log, TEXT("Dialogue: %s"), *DisplayText);

    // Set timer for next entry or completion
    if (GetWorld())
    {
        float TimerDuration = CurrentEntry.DisplayDuration + CurrentSequence.DelayBetweenEntries;
        GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, this, &UNarr_DialogueSystem::OnDialogueEntryComplete, TimerDuration, false);
    }
}

void UNarr_DialogueSystem::OnDialogueEntryComplete()
{
    if (!bDialogueActive || CurrentSequenceIndex < 0)
    {
        return;
    }

    CurrentEntryIndex++;

    if (DialogueSequences.IsValidIndex(CurrentSequenceIndex))
    {
        const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceIndex];
        
        if (CurrentEntryIndex < CurrentSequence.DialogueEntries.Num())
        {
            // Process next entry in sequence
            ProcessNextDialogueEntry();
        }
        else
        {
            // Sequence complete
            StopDialogue();
        }
    }
    else
    {
        StopDialogue();
    }
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Create survival tutorial dialogue
    FNarr_DialogueSequence TutorialSequence;
    TutorialSequence.SequenceName = TEXT("SurvivalTutorial");
    TutorialSequence.bAutoPlay = false;
    TutorialSequence.DelayBetweenEntries = 2.0f;

    FNarr_DialogueEntry Entry1;
    Entry1.SpeakerName = TEXT("Narrator");
    Entry1.DialogueText = TEXT("The ancient valley holds many secrets, survivor. Listen carefully to the wind.");
    Entry1.DisplayDuration = 4.0f;
    Entry1.bIsNarration = true;
    TutorialSequence.DialogueEntries.Add(Entry1);

    FNarr_DialogueEntry Entry2;
    Entry2.SpeakerName = TEXT("Scout");
    Entry2.DialogueText = TEXT("Warning! Pack hunters detected in the eastern ravines. Stay low and move quietly.");
    Entry2.DisplayDuration = 4.0f;
    Entry2.bIsNarration = false;
    TutorialSequence.DialogueEntries.Add(Entry2);

    DialogueSequences.Add(TutorialSequence);

    // Create danger warning dialogue
    FNarr_DialogueSequence DangerSequence;
    DangerSequence.SequenceName = TEXT("DangerWarning");
    DangerSequence.bAutoPlay = true;
    DangerSequence.DelayBetweenEntries = 1.5f;

    FNarr_DialogueEntry DangerEntry;
    DangerEntry.SpeakerName = TEXT("Elder");
    DangerEntry.DialogueText = TEXT("Thunder Lizard territory ahead! The massive predator's roar can be heard for miles.");
    DangerEntry.DisplayDuration = 5.0f;
    DangerEntry.bIsNarration = false;
    DangerSequence.DialogueEntries.Add(DangerEntry);

    DialogueSequences.Add(DangerSequence);

    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue System initialized with %d sequences"), DialogueSequences.Num());
}