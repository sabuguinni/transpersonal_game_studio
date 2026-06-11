#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentSequence = nullptr;
    CurrentLineIndex = 0;
    bDialogueActive = false;
    
    // Initialize survival dialogue sequences
    FNarr_DialogueSequence IntroSequence;
    IntroSequence.SequenceID = TEXT("GameIntro");
    IntroSequence.TriggerCondition = ENarr_DialogueTrigger::GameStart;
    
    FNarr_DialogueLine IntroLine1;
    IntroLine1.SpeakerName = TEXT("Narrator");
    IntroLine1.DialogueText = FText::FromString(TEXT("You awaken in a world lost to time. The air is thick with danger, and survival is your only goal."));
    IntroLine1.Duration = 5.0f;
    IntroSequence.DialogueLines.Add(IntroLine1);
    
    FNarr_DialogueLine IntroLine2;
    IntroLine2.SpeakerName = TEXT("Narrator");
    IntroLine2.DialogueText = FText::FromString(TEXT("Find shelter, gather resources, and beware the ancient predators that rule this land."));
    IntroLine2.Duration = 4.0f;
    IntroSequence.DialogueLines.Add(IntroLine2);
    
    RegisterDialogueSequence(IntroSequence);
    
    // Dinosaur encounter dialogue
    FNarr_DialogueSequence DinosaurEncounter;
    DinosaurEncounter.SequenceID = TEXT("DinosaurSighting");
    DinosaurEncounter.TriggerCondition = ENarr_DialogueTrigger::DinosaurNear;
    
    FNarr_DialogueLine EncounterLine;
    EncounterLine.SpeakerName = TEXT("Player");
    EncounterLine.DialogueText = FText::FromString(TEXT("By the spirits... that creature is massive. I must move carefully or become its next meal."));
    EncounterLine.Duration = 4.0f;
    DinosaurEncounter.DialogueLines.Add(EncounterLine);
    
    RegisterDialogueSequence(DinosaurEncounter);
    
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueManager initialized with %d sequences"), DialogueSequences.Num());
}

void UNarr_DialogueManager::StartDialogue(const FString& SequenceID)
{
    if (FNarr_DialogueSequence* Sequence = DialogueSequences.Find(SequenceID))
    {
        CurrentSequence = Sequence;
        CurrentLineIndex = 0;
        bDialogueActive = true;
        
        if (Sequence->DialogueLines.Num() > 0)
        {
            ProcessDialogueLine(Sequence->DialogueLines[0]);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
    }
}

void UNarr_DialogueManager::StopCurrentDialogue()
{
    if (bDialogueActive)
    {
        bDialogueActive = false;
        CurrentSequence = nullptr;
        CurrentLineIndex = 0;
        
        UE_LOG(LogTemp, Log, TEXT("Stopped current dialogue"));
    }
}

bool UNarr_DialogueManager::IsDialogueActive() const
{
    return bDialogueActive;
}

void UNarr_DialogueManager::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s with %d lines"), 
           *Sequence.SequenceID, Sequence.DialogueLines.Num());
}

void UNarr_DialogueManager::TriggerContextualDialogue(ENarr_DialogueTrigger TriggerType, const FVector& Location)
{
    for (const auto& SequencePair : DialogueSequences)
    {
        if (SequencePair.Value.TriggerCondition == TriggerType)
        {
            StartDialogue(SequencePair.Key);
            break;
        }
    }
}

void UNarr_DialogueManager::AdvanceDialogue()
{
    if (!bDialogueActive || !CurrentSequence)
    {
        return;
    }
    
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        StopCurrentDialogue();
        return;
    }
    
    ProcessDialogueLine(CurrentSequence->DialogueLines[CurrentLineIndex]);
}

void UNarr_DialogueManager::ProcessDialogueLine(const FNarr_DialogueLine& Line)
{
    // Display dialogue text to screen
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), 
                                            *Line.SpeakerName, 
                                            *Line.DialogueText.ToString());
        
        GEngine->AddOnScreenDebugMessage(-1, Line.Duration, FColor::White, DisplayText);
    }
    
    // Set timer to advance to next line
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, this, &UNarr_DialogueManager::AdvanceDialogue, Line.Duration, false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Processing dialogue line: %s - %s"), 
           *Line.SpeakerName, *Line.DialogueText.ToString());
}