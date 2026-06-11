#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsPlaying = false;
    CurrentLineIndex = 0;
    CurrentSequenceID = TEXT("");
    
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Dialogue Manager initialized"));
}

void UNarr_DialogueManager::PlayDialogueSequence(const FString& SequenceID)
{
    if (bIsPlaying)
    {
        StopCurrentDialogue();
    }
    
    if (!DialogueSequences.Contains(SequenceID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
        return;
    }
    
    CurrentSequenceID = SequenceID;
    CurrentLineIndex = 0;
    bIsPlaying = true;
    
    AdvanceDialogue();
}

void UNarr_DialogueManager::StopCurrentDialogue()
{
    bIsPlaying = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }
}

void UNarr_DialogueManager::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Warning, TEXT("Registered dialogue sequence: %s"), *Sequence.SequenceID);
}

bool UNarr_DialogueManager::IsDialoguePlaying() const
{
    return bIsPlaying;
}

void UNarr_DialogueManager::AdvanceDialogue()
{
    if (!bIsPlaying || !DialogueSequences.Contains(CurrentSequenceID))
    {
        return;
    }
    
    const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceID];
    
    if (CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        StopCurrentDialogue();
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = CurrentSequence.DialogueLines[CurrentLineIndex];
    
    // Display dialogue to player
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, CurrentLine.Duration, 
            CurrentLine.bIsUrgent ? FColor::Red : FColor::White, DisplayText);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dialogue: %s - %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
    
    CurrentLineIndex++;
    
    // Schedule next line
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, this, &UNarr_DialogueManager::AdvanceDialogue, 
            CurrentLine.Duration, false);
    }
}

void UNarr_DialogueManager::InitializeDefaultDialogues()
{
    // Tribal Elder Introduction
    FNarr_DialogueSequence ElderIntro;
    ElderIntro.SequenceID = TEXT("ElderIntro");
    ElderIntro.bIsRepeatable = false;
    
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Elder");
    Line1.DialogueText = TEXT("Stay low, newcomer. The great beasts hunt these grounds.");
    Line1.Duration = 4.0f;
    Line1.bIsUrgent = false;
    
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Elder");
    Line2.DialogueText = TEXT("Follow the river stones to shelter before the sun sets.");
    Line2.Duration = 3.5f;
    Line2.bIsUrgent = false;
    
    ElderIntro.DialogueLines.Add(Line1);
    ElderIntro.DialogueLines.Add(Line2);
    RegisterDialogueSequence(ElderIntro);
    
    // Scout Warning
    FNarr_DialogueSequence ScoutWarning;
    ScoutWarning.SequenceID = TEXT("ScoutWarning");
    ScoutWarning.bIsRepeatable = true;
    
    FNarr_DialogueLine Warning1;
    Warning1.SpeakerName = TEXT("Scout");
    Warning1.DialogueText = TEXT("Warning! Thunderfoot approaches from the eastern ridge.");
    Warning1.Duration = 3.0f;
    Warning1.bIsUrgent = true;
    
    FNarr_DialogueLine Warning2;
    Warning2.SpeakerName = TEXT("Scout");
    Warning2.DialogueText = TEXT("Seek shelter immediately or become prey.");
    Warning2.Duration = 2.5f;
    Warning2.bIsUrgent = true;
    
    ScoutWarning.DialogueLines.Add(Warning1);
    ScoutWarning.DialogueLines.Add(Warning2);
    RegisterDialogueSequence(ScoutWarning);
    
    // Hunter Instructions
    FNarr_DialogueSequence HunterTips;
    HunterTips.SequenceID = TEXT("HunterTips");
    HunterTips.bIsRepeatable = true;
    
    FNarr_DialogueLine Tip1;
    Tip1.SpeakerName = TEXT("Hunter");
    Tip1.DialogueText = TEXT("Sharp stones make better spears than dull ones.");
    Tip1.Duration = 3.0f;
    Tip1.bIsUrgent = false;
    
    FNarr_DialogueLine Tip2;
    Tip2.SpeakerName = TEXT("Hunter");
    Tip2.DialogueText = TEXT("Aim for the soft belly, not the armored hide.");
    Tip2.Duration = 3.0f;
    Tip2.bIsUrgent = false;
    
    HunterTips.DialogueLines.Add(Tip1);
    HunterTips.DialogueLines.Add(Tip2);
    RegisterDialogueSequence(HunterTips);
}