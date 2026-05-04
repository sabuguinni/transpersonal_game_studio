#include "Narr_DialogueSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentSequence = nullptr;
    CurrentLineIndex = 0;
    bIsDialogueActive = false;
    
    // Create audio component for voice playback
    VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
    if (VoiceAudioComponent)
    {
        VoiceAudioComponent->bAutoActivate = false;
        VoiceAudioComponent->SetVolumeMultiplier(0.8f);
    }
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponent();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue System initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNarr_DialogueSystem::InitializeAudioComponent()
{
    if (VoiceAudioComponent && GetOwner())
    {
        VoiceAudioComponent->AttachToComponent(
            GetOwner()->GetRootComponent(),
            FAttachmentTransformRules::KeepWorldTransform
        );
    }
}

bool UNarr_DialogueSystem::StartDialogueSequence(const FString& SequenceID)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue - another sequence is active"));
        return false;
    }
    
    FNarr_DialogueSequence* Sequence = FindSequenceByID(SequenceID);
    if (!Sequence)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
        return false;
    }
    
    if (Sequence->bHasBeenPlayed && !Sequence->bIsRepeatable)
    {
        UE_LOG(LogTemp, Log, TEXT("Dialogue sequence already played and not repeatable: %s"), *SequenceID);
        return false;
    }
    
    CurrentSequence = Sequence;
    CurrentLineIndex = 0;
    bIsDialogueActive = true;
    
    // Mark as played
    Sequence->bHasBeenPlayed = true;
    
    // Trigger Blueprint event
    OnDialogueStarted(SequenceID);
    
    // Start first line
    ProcessCurrentLine();
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
    return true;
}

void UNarr_DialogueSystem::NextDialogueLine()
{
    if (!bIsDialogueActive || !CurrentSequence)
    {
        return;
    }
    
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        // End of sequence
        EndDialogue();
        return;
    }
    
    ProcessCurrentLine();
}

void UNarr_DialogueSystem::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    // Stop any playing audio
    StopVoicePlayback();
    
    // Clear timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    bIsDialogueActive = false;
    CurrentSequence = nullptr;
    CurrentLineIndex = 0;
    
    // Trigger Blueprint event
    OnDialogueEnded();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

bool UNarr_DialogueSystem::IsDialogueActive() const
{
    return bIsDialogueActive;
}

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentDialogueLine() const
{
    if (bIsDialogueActive && CurrentSequence && 
        CurrentLineIndex >= 0 && CurrentLineIndex < CurrentSequence->DialogueLines.Num())
    {
        return CurrentSequence->DialogueLines[CurrentLineIndex];
    }
    
    return FNarr_DialogueLine();
}

void UNarr_DialogueSystem::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    // Check if sequence already exists
    for (int32 i = 0; i < DialogueSequences.Num(); i++)
    {
        if (DialogueSequences[i].SequenceID == NewSequence.SequenceID)
        {
            // Replace existing sequence
            DialogueSequences[i] = NewSequence;
            UE_LOG(LogTemp, Log, TEXT("Replaced dialogue sequence: %s"), *NewSequence.SequenceID);
            return;
        }
    }
    
    // Add new sequence
    DialogueSequences.Add(NewSequence);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue sequence: %s"), *NewSequence.SequenceID);
}

bool UNarr_DialogueSystem::HasSequence(const FString& SequenceID) const
{
    return FindSequenceByID(SequenceID) != nullptr;
}

TArray<FString> UNarr_DialogueSystem::GetAvailableSequenceIDs() const
{
    TArray<FString> SequenceIDs;
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (!Sequence.bHasBeenPlayed || Sequence.bIsRepeatable)
        {
            SequenceIDs.Add(Sequence.SequenceID);
        }
    }
    return SequenceIDs;
}

void UNarr_DialogueSystem::PlayVoiceLine(USoundBase* VoiceClip)
{
    if (!VoiceClip || !VoiceAudioComponent)
    {
        return;
    }
    
    VoiceAudioComponent->SetSound(VoiceClip);
    VoiceAudioComponent->Play();
    
    UE_LOG(LogTemp, Log, TEXT("Playing voice clip"));
}

void UNarr_DialogueSystem::StopVoicePlayback()
{
    if (VoiceAudioComponent && VoiceAudioComponent->IsPlaying())
    {
        VoiceAudioComponent->Stop();
    }
}

FNarr_DialogueSequence* UNarr_DialogueSystem::FindSequenceByID(const FString& SequenceID)
{
    for (FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            return &Sequence;
        }
    }
    return nullptr;
}

const FNarr_DialogueSequence* UNarr_DialogueSystem::FindSequenceByID(const FString& SequenceID) const
{
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            return &Sequence;
        }
    }
    return nullptr;
}

void UNarr_DialogueSystem::ProcessCurrentLine()
{
    if (!CurrentSequence || CurrentLineIndex < 0 || 
        CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = CurrentSequence->DialogueLines[CurrentLineIndex];
    
    // Play voice clip if available
    if (CurrentLine.VoiceClip)
    {
        PlayVoiceLine(CurrentLine.VoiceClip);
    }
    
    // Trigger Blueprint event
    OnDialogueLineChanged(CurrentLine);
    
    // Check if this is a player choice
    if (CurrentLine.bIsPlayerChoice)
    {
        // Collect all consecutive player choice lines
        TArray<FNarr_DialogueLine> Choices;
        int32 ChoiceIndex = CurrentLineIndex;
        
        while (ChoiceIndex < CurrentSequence->DialogueLines.Num() && 
               CurrentSequence->DialogueLines[ChoiceIndex].bIsPlayerChoice)
        {
            Choices.Add(CurrentSequence->DialogueLines[ChoiceIndex]);
            ChoiceIndex++;
        }
        
        OnPlayerChoiceRequired(Choices);
        return; // Don't auto-advance for player choices
    }
    
    // Set timer for auto-advance if not a player choice
    if (GetWorld() && CurrentLine.Duration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            DialogueTimerHandle,
            this,
            &UNarr_DialogueSystem::AutoAdvanceDialogue,
            CurrentLine.Duration,
            false
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Processing dialogue line: %s - %s"), 
           *CurrentLine.SpeakerName, *CurrentLine.DialogueText.ToString());
}

void UNarr_DialogueSystem::AutoAdvanceDialogue()
{
    NextDialogueLine();
}