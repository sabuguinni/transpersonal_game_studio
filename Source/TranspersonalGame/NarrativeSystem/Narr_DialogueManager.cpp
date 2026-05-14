#include "Narr_DialogueManager.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    bIsDialogueActive = false;
    CurrentDialogueID = TEXT("");
    CurrentDialogueIndex = 0;
    VoiceAudioComponent = nullptr;
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Create audio component for voice playback
    if (UWorld* World = GetWorld())
    {
        VoiceAudioComponent = NewObject<UAudioComponent>(this);
        if (VoiceAudioComponent)
        {
            VoiceAudioComponent->bAutoActivate = false;
            VoiceAudioComponent->SetVolumeMultiplier(1.0f);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue Manager initialized"));
}

void UNarr_DialogueManager::Deinitialize()
{
    EndDialogue();
    
    if (VoiceAudioComponent)
    {
        VoiceAudioComponent->Stop();
        VoiceAudioComponent = nullptr;
    }
    
    DialogueSequences.Empty();
    
    Super::Deinitialize();
}

bool UNarr_DialogueManager::StartDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue %s - another dialogue is already active"), *DialogueID);
        return false;
    }
    
    if (!DialogueSequences.Contains(DialogueID))
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue sequence %s not found"), *DialogueID);
        return false;
    }
    
    CurrentSequence = DialogueSequences[DialogueID];
    if (CurrentSequence.DialogueEntries.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue sequence %s has no entries"), *DialogueID);
        return false;
    }
    
    // Set dialogue state
    bIsDialogueActive = true;
    CurrentDialogueID = DialogueID;
    CurrentDialogueIndex = 0;
    CurrentSpeaker = Speaker;
    CurrentListener = Listener;
    
    // Start first dialogue entry
    const FNarr_DialogueEntry& FirstEntry = CurrentSequence.DialogueEntries[0];
    PlayVoiceClip(FirstEntry);
    
    // Broadcast dialogue started event
    OnDialogueStarted.Broadcast(DialogueID, FirstEntry.SpeakerName);
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s with speaker: %s"), *DialogueID, *FirstEntry.SpeakerName);
    return true;
}

void UNarr_DialogueManager::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    StopVoiceClip();
    
    FString EndedDialogueID = CurrentDialogueID;
    
    // Reset dialogue state
    bIsDialogueActive = false;
    CurrentDialogueID = TEXT("");
    CurrentDialogueIndex = 0;
    CurrentSpeaker.Reset();
    CurrentListener.Reset();
    CurrentSequence = FNarr_DialogueSequence();
    
    // Broadcast dialogue ended event
    OnDialogueEnded.Broadcast(EndedDialogueID);
    
    UE_LOG(LogTemp, Log, TEXT("Ended dialogue: %s"), *EndedDialogueID);
}

bool UNarr_DialogueManager::AdvanceDialogue()
{
    if (!bIsDialogueActive)
    {
        return false;
    }
    
    // Check if we're at the end of the sequence
    if (CurrentDialogueIndex >= CurrentSequence.DialogueEntries.Num() - 1)
    {
        EndDialogue();
        return false;
    }
    
    // Move to next dialogue entry
    CurrentDialogueIndex++;
    const FNarr_DialogueEntry& NextEntry = CurrentSequence.DialogueEntries[CurrentDialogueIndex];
    
    // Handle player choices
    if (NextEntry.bIsPlayerChoice)
    {
        ProcessDialogueChoice(NextEntry);
    }
    else
    {
        PlayVoiceClip(NextEntry);
    }
    
    return true;
}

void UNarr_DialogueManager::SelectDialogueChoice(int32 ChoiceIndex)
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    const FNarr_DialogueEntry& CurrentEntry = CurrentSequence.DialogueEntries[CurrentDialogueIndex];
    
    if (!CurrentEntry.bIsPlayerChoice || ChoiceIndex < 0 || ChoiceIndex >= CurrentEntry.NextDialogueIDs.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid dialogue choice index: %d"), ChoiceIndex);
        return;
    }
    
    // Find the next dialogue sequence based on choice
    const FString& NextDialogueID = CurrentEntry.NextDialogueIDs[ChoiceIndex];
    
    if (NextDialogueID.IsEmpty())
    {
        EndDialogue();
        return;
    }
    
    // End current dialogue and start next one
    EndDialogue();
    StartDialogue(NextDialogueID, CurrentSpeaker.Get(), CurrentListener.Get());
}

void UNarr_DialogueManager::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    if (Sequence.SequenceID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register dialogue sequence with empty ID"));
        return;
    }
    
    // Validate all dialogue entries
    for (const FNarr_DialogueEntry& Entry : Sequence.DialogueEntries)
    {
        if (!ValidateDialogueEntry(Entry))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid dialogue entry in sequence %s"), *Sequence.SequenceID);
            return;
        }
    }
    
    DialogueSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s with %d entries"), 
           *Sequence.SequenceID, Sequence.DialogueEntries.Num());
}

bool UNarr_DialogueManager::HasDialogue(const FString& DialogueID) const
{
    return DialogueSequences.Contains(DialogueID);
}

FNarr_DialogueEntry UNarr_DialogueManager::GetCurrentDialogueEntry() const
{
    if (!bIsDialogueActive || CurrentDialogueIndex >= CurrentSequence.DialogueEntries.Num())
    {
        return FNarr_DialogueEntry();
    }
    
    return CurrentSequence.DialogueEntries[CurrentDialogueIndex];
}

void UNarr_DialogueManager::PlayVoiceClip(const FNarr_DialogueEntry& Entry)
{
    StopVoiceClip();
    
    if (Entry.VoiceClip.IsValid() && VoiceAudioComponent)
    {
        USoundCue* SoundCue = Entry.VoiceClip.LoadSynchronous();
        if (SoundCue)
        {
            VoiceAudioComponent->SetSound(SoundCue);
            VoiceAudioComponent->Play();
            
            UE_LOG(LogTemp, Log, TEXT("Playing voice clip for dialogue: %s"), *Entry.DialogueID);
        }
    }
}

void UNarr_DialogueManager::StopVoiceClip()
{
    if (VoiceAudioComponent && VoiceAudioComponent->IsPlaying())
    {
        VoiceAudioComponent->Stop();
    }
}

bool UNarr_DialogueManager::ValidateDialogueEntry(const FNarr_DialogueEntry& Entry) const
{
    if (Entry.DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue entry has empty ID"));
        return false;
    }
    
    if (Entry.SpeakerName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue entry %s has empty speaker name"), *Entry.DialogueID);
        return false;
    }
    
    if (Entry.DialogueText.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue entry %s has empty text"), *Entry.DialogueID);
        return false;
    }
    
    if (Entry.Duration <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue entry %s has invalid duration: %f"), *Entry.DialogueID, Entry.Duration);
    }
    
    return true;
}

void UNarr_DialogueManager::ProcessDialogueChoice(const FNarr_DialogueEntry& Entry)
{
    if (!Entry.bIsPlayerChoice)
    {
        return;
    }
    
    // Broadcast choice event with available options
    OnDialogueChoice.Broadcast(Entry.DialogueID, Entry.NextDialogueIDs, -1);
    
    UE_LOG(LogTemp, Log, TEXT("Presenting player choice for dialogue: %s with %d options"), 
           *Entry.DialogueID, Entry.NextDialogueIDs.Num());
}