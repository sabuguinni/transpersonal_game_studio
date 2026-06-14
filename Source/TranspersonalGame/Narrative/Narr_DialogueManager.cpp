#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsDialogueActive = false;
    CurrentDialogueIndex = 0;
    CurrentSpeaker = nullptr;
    CurrentListener = nullptr;
    
    LoadDefaultDialogues();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue Manager initialized with %d sequences"), DialogueDatabase.Num());
}

void UNarr_DialogueManager::Deinitialize()
{
    EndDialogue();
    DialogueDatabase.Empty();
    Super::Deinitialize();
}

void UNarr_DialogueManager::StartDialogue(const FString& SequenceID, AActor* Speaker, AActor* Listener)
{
    if (!Speaker || !Listener)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue: Invalid speaker or listener"));
        return;
    }

    if (FNarr_DialogueSequence* Sequence = DialogueDatabase.Find(SequenceID))
    {
        if (!CheckPrerequisites(Sequence->Prerequisites))
        {
            UE_LOG(LogTemp, Warning, TEXT("Dialogue prerequisites not met for sequence: %s"), *SequenceID);
            return;
        }

        CurrentSequence = *Sequence;
        CurrentDialogueIndex = 0;
        bIsDialogueActive = true;
        CurrentSpeaker = Speaker;
        CurrentListener = Listener;

        UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
    }
}

void UNarr_DialogueManager::EndDialogue()
{
    if (bIsDialogueActive)
    {
        bIsDialogueActive = false;
        CurrentDialogueIndex = 0;
        CurrentSpeaker = nullptr;
        CurrentListener = nullptr;
        CurrentSequence = FNarr_DialogueSequence();
        
        UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
    }
}

void UNarr_DialogueManager::AdvanceDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    CurrentDialogueIndex++;
    
    if (CurrentDialogueIndex >= CurrentSequence.DialogueEntries.Num())
    {
        EndDialogue();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Advanced to dialogue entry %d"), CurrentDialogueIndex);
    }
}

bool UNarr_DialogueManager::IsDialogueActive() const
{
    return bIsDialogueActive;
}

FNarr_DialogueEntry UNarr_DialogueManager::GetCurrentDialogue() const
{
    if (bIsDialogueActive && CurrentSequence.DialogueEntries.IsValidIndex(CurrentDialogueIndex))
    {
        return CurrentSequence.DialogueEntries[CurrentDialogueIndex];
    }
    
    return FNarr_DialogueEntry();
}

void UNarr_DialogueManager::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueDatabase.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s"), *Sequence.SequenceID);
}

TArray<FString> UNarr_DialogueManager::GetAvailableDialogues(AActor* Speaker) const
{
    TArray<FString> AvailableDialogues;
    
    for (const auto& DialoguePair : DialogueDatabase)
    {
        if (CheckPrerequisites(DialoguePair.Value.Prerequisites))
        {
            AvailableDialogues.Add(DialoguePair.Key);
        }
    }
    
    return AvailableDialogues;
}

void UNarr_DialogueManager::LoadDefaultDialogues()
{
    // Tribal Elder Krog - Introduction
    FNarr_DialogueSequence KrogIntro;
    KrogIntro.SequenceID = TEXT("krog_intro");
    KrogIntro.bIsRepeatable = false;
    
    FNarr_DialogueEntry Entry1;
    Entry1.SpeakerName = TEXT("Krog");
    Entry1.DialogueText = FText::FromString(TEXT("The ancient hunting grounds call to you, survivor. I am Krog, keeper of the old ways."));
    Entry1.Duration = 4.5f;
    Entry1.ResponseOptions.Add(TEXT("Who are you?"));
    Entry1.ResponseOptions.Add(TEXT("What dangers lurk here?"));
    
    FNarr_DialogueEntry Entry2;
    Entry2.SpeakerName = TEXT("Krog");
    Entry2.DialogueText = FText::FromString(TEXT("The great beasts roam these lands - some friend, some foe. Listen well to the wind."));
    Entry2.Duration = 4.0f;
    
    KrogIntro.DialogueEntries.Add(Entry1);
    KrogIntro.DialogueEntries.Add(Entry2);
    
    RegisterDialogueSequence(KrogIntro);
    
    // Warning System
    FNarr_DialogueSequence WarningSequence;
    WarningSequence.SequenceID = TEXT("predator_warning");
    WarningSequence.bIsRepeatable = true;
    
    FNarr_DialogueEntry Warning;
    Warning.SpeakerName = TEXT("Warning System");
    Warning.DialogueText = FText::FromString(TEXT("Warning! Massive predator approaching. Take shelter or prepare for battle."));
    Warning.Duration = 3.5f;
    
    WarningSequence.DialogueEntries.Add(Warning);
    
    RegisterDialogueSequence(WarningSequence);
    
    // Hunter's Advice
    FNarr_DialogueSequence HunterAdvice;
    HunterAdvice.SequenceID = TEXT("hunter_advice");
    HunterAdvice.bIsRepeatable = true;
    
    FNarr_DialogueEntry Advice1;
    Advice1.SpeakerName = TEXT("Veteran Hunter");
    Advice1.DialogueText = FText::FromString(TEXT("Track the wounded beasts to their lairs. They are weakest when cornered."));
    Advice1.Duration = 4.0f;
    
    FNarr_DialogueEntry Advice2;
    Advice2.SpeakerName = TEXT("Veteran Hunter");
    Advice2.DialogueText = FText::FromString(TEXT("Remember - the pack hunts together. Never face a raptor alone."));
    Advice2.Duration = 3.8f;
    
    HunterAdvice.DialogueEntries.Add(Advice1);
    HunterAdvice.DialogueEntries.Add(Advice2);
    
    RegisterDialogueSequence(HunterAdvice);
}

bool UNarr_DialogueManager::CheckPrerequisites(const TArray<FString>& Prerequisites) const
{
    // For now, assume all prerequisites are met
    // In a full implementation, this would check game state, completed quests, etc.
    return true;
}