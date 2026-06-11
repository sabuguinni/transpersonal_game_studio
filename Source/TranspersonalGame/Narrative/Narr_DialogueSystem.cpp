#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    bDialogueActive = false;
    DefaultDisplayDuration = 3.0f;
    
    InitializeDefaultDialogues();
}

void UNarr_DialogueSystem::StartDialogueSequence(const FString& SequenceID)
{
    if (DialogueSequences.Contains(SequenceID))
    {
        CurrentSequenceID = SequenceID;
        CurrentLineIndex = 0;
        bDialogueActive = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Started dialogue sequence: %s"), *SequenceID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue sequence not found: %s"), *SequenceID);
    }
}

void UNarr_DialogueSystem::StopCurrentDialogue()
{
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    bDialogueActive = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Stopped current dialogue"));
}

bool UNarr_DialogueSystem::IsDialogueActive() const
{
    return bDialogueActive;
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Warning, TEXT("Registered dialogue sequence: %s"), *Sequence.SequenceID);
}

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentDialogueLine() const
{
    if (bDialogueActive && DialogueSequences.Contains(CurrentSequenceID))
    {
        const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceID];
        if (CurrentSequence.DialogueLines.IsValidIndex(CurrentLineIndex))
        {
            return CurrentSequence.DialogueLines[CurrentLineIndex];
        }
    }
    
    return FNarr_DialogueLine();
}

void UNarr_DialogueSystem::AdvanceDialogue()
{
    if (!bDialogueActive || !DialogueSequences.Contains(CurrentSequenceID))
    {
        return;
    }
    
    const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceID];
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        // End of sequence
        if (CurrentSequence.bIsRepeatable)
        {
            CurrentLineIndex = 0;
        }
        else
        {
            StopCurrentDialogue();
        }
    }
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Survival Tutorial Dialogue
    FNarr_DialogueSequence TutorialSequence;
    TutorialSequence.SequenceID = TEXT("SurvivalTutorial");
    TutorialSequence.bIsRepeatable = false;
    
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Elder Hunter");
    Line1.DialogueText = TEXT("Stay low, newcomer. The great beasts hunt these grounds.");
    Line1.DisplayDuration = 4.0f;
    TutorialSequence.DialogueLines.Add(Line1);
    
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Elder Hunter");
    Line2.DialogueText = TEXT("Follow the river stones to shelter before the sun sets.");
    Line2.DisplayDuration = 4.0f;
    TutorialSequence.DialogueLines.Add(Line2);
    
    RegisterDialogueSequence(TutorialSequence);
    
    // Danger Warning Dialogue
    FNarr_DialogueSequence DangerSequence;
    DangerSequence.SequenceID = TEXT("DangerWarning");
    DangerSequence.bIsRepeatable = true;
    
    FNarr_DialogueLine Warning1;
    Warning1.SpeakerName = TEXT("Scout");
    Warning1.DialogueText = TEXT("Warning! Thunderfoot approaches from the eastern ridge.");
    Warning1.DisplayDuration = 3.5f;
    DangerSequence.DialogueLines.Add(Warning1);
    
    FNarr_DialogueLine Warning2;
    Warning2.SpeakerName = TEXT("Scout");
    Warning2.DialogueText = TEXT("Seek shelter in the caves until it passes.");
    Warning2.DisplayDuration = 3.0f;
    DangerSequence.DialogueLines.Add(Warning2);
    
    RegisterDialogueSequence(DangerSequence);
    
    // Discovery Dialogue
    FNarr_DialogueSequence DiscoverySequence;
    DiscoverySequence.SequenceID = TEXT("FirstDiscovery");
    DiscoverySequence.bIsRepeatable = false;
    
    FNarr_DialogueLine Discovery1;
    Discovery1.SpeakerName = TEXT("Explorer");
    Discovery1.DialogueText = TEXT("These ancient hunting grounds stretch beyond the horizon.");
    Discovery1.DisplayDuration = 4.0f;
    DiscoverySequence.DialogueLines.Add(Discovery1);
    
    FNarr_DialogueLine Discovery2;
    Discovery2.SpeakerName = TEXT("Explorer");
    Discovery2.DialogueText = TEXT("The river runs red with danger - massive predator tracks everywhere.");
    Discovery2.DisplayDuration = 4.5f;
    DiscoverySequence.DialogueLines.Add(Discovery2);
    
    RegisterDialogueSequence(DiscoverySequence);
}