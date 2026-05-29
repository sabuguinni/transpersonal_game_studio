#include "DialogueComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    bIsDialogueActive = false;
    InteractionRange = 300.0f;
    DefaultVoiceSound = nullptr;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default dialogue sequences for tribal NPCs
    FNarr_DialogueSequence ChieftainGreeting;
    ChieftainGreeting.SequenceID = TEXT("ChieftainGreeting");
    ChieftainGreeting.bRepeatable = true;
    ChieftainGreeting.Priority = 10;
    
    FNarr_DialogueLine GreetingLine;
    GreetingLine.SpeakerName = TEXT("Tribal Chieftain");
    GreetingLine.DialogueText = FText::FromString(TEXT("Welcome, hunter. The great beasts grow bolder each day. We need warriors who can face the Carnotaurus and live to tell the tale."));
    GreetingLine.Duration = 6.0f;
    GreetingLine.EmotionalTone = ENarr_EmotionalTone::Authoritative;
    
    ChieftainGreeting.DialogueLines.Add(GreetingLine);
    DialogueSequences.Add(ChieftainGreeting);
    
    // Scout warning dialogue
    FNarr_DialogueSequence ScoutWarning;
    ScoutWarning.SequenceID = TEXT("ScoutWarning");
    ScoutWarning.bRepeatable = false;
    ScoutWarning.Priority = 15;
    
    FNarr_DialogueLine WarningLine;
    WarningLine.SpeakerName = TEXT("Scout");
    WarningLine.DialogueText = FText::FromString(TEXT("Chief! The pack hunters circle the eastern ridge. Three Velociraptors, moving in formation. They've learned to coordinate their attacks."));
    WarningLine.Duration = 7.0f;
    WarningLine.EmotionalTone = ENarr_EmotionalTone::Urgent;
    
    ScoutWarning.DialogueLines.Add(WarningLine);
    DialogueSequences.Add(ScoutWarning);
}

bool UNarr_DialogueComponent::StartDialogueSequence(const FString& SequenceID)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already active, cannot start new sequence"));
        return false;
    }
    
    FNarr_DialogueSequence* Sequence = FindSequenceByID(SequenceID);
    if (!Sequence || Sequence->DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found or empty: %s"), *SequenceID);
        return false;
    }
    
    if (!CanStartDialogue(SequenceID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue sequence: %s"), *SequenceID);
        return false;
    }
    
    CurrentSequenceID = SequenceID;
    CurrentLineIndex = 0;
    bIsDialogueActive = true;
    
    const FNarr_DialogueLine& FirstLine = Sequence->DialogueLines[0];
    PlayVoiceLine(FirstLine);
    
    OnDialogueStarted.Broadcast(SequenceID, FirstLine);
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
    return true;
}

void UNarr_DialogueComponent::NextDialogueLine()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    FNarr_DialogueSequence* CurrentSequence = FindSequenceByID(CurrentSequenceID);
    if (!CurrentSequence)
    {
        EndDialogue();
        return;
    }
    
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        EndDialogue();
        return;
    }
    
    const FNarr_DialogueLine& NextLine = CurrentSequence->DialogueLines[CurrentLineIndex];
    PlayVoiceLine(NextLine);
    
    OnDialogueLineChanged.Broadcast(NextLine);
    
    UE_LOG(LogTemp, Log, TEXT("Advanced to dialogue line %d"), CurrentLineIndex);
}

void UNarr_DialogueComponent::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    bIsDialogueActive = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    
    OnDialogueEnded.Broadcast();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentDialogueLine() const
{
    if (!bIsDialogueActive)
    {
        return FNarr_DialogueLine();
    }
    
    const FNarr_DialogueSequence* CurrentSequence = const_cast<UNarr_DialogueComponent*>(this)->FindSequenceByID(CurrentSequenceID);
    if (!CurrentSequence || CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        return FNarr_DialogueLine();
    }
    
    return CurrentSequence->DialogueLines[CurrentLineIndex];
}

bool UNarr_DialogueComponent::CanStartDialogue(const FString& SequenceID) const
{
    const FNarr_DialogueSequence* Sequence = const_cast<UNarr_DialogueComponent*>(this)->FindSequenceByID(SequenceID);
    if (!Sequence)
    {
        return false;
    }
    
    // Check if sequence is repeatable or hasn't been played yet
    // For now, all sequences are available (quest phase checking can be added later)
    return true;
}

void UNarr_DialogueComponent::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    // Remove existing sequence with same ID if it exists
    DialogueSequences.RemoveAll([&NewSequence](const FNarr_DialogueSequence& Existing)
    {
        return Existing.SequenceID == NewSequence.SequenceID;
    });
    
    DialogueSequences.Add(NewSequence);
    
    // Sort by priority (higher priority first)
    DialogueSequences.Sort([](const FNarr_DialogueSequence& A, const FNarr_DialogueSequence& B)
    {
        return A.Priority > B.Priority;
    });
    
    UE_LOG(LogTemp, Log, TEXT("Added dialogue sequence: %s"), *NewSequence.SequenceID);
}

TArray<FString> UNarr_DialogueComponent::GetAvailableSequenceIDs() const
{
    TArray<FString> AvailableIDs;
    
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (CanStartDialogue(Sequence.SequenceID))
        {
            AvailableIDs.Add(Sequence.SequenceID);
        }
    }
    
    return AvailableIDs;
}

FNarr_DialogueSequence* UNarr_DialogueComponent::FindSequenceByID(const FString& SequenceID)
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

void UNarr_DialogueComponent::PlayVoiceLine(const FNarr_DialogueLine& DialogueLine)
{
    if (DialogueLine.AudioAssetPath.IsEmpty() && !DefaultVoiceSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("No audio asset specified for dialogue line"));
        return;
    }
    
    USoundBase* SoundToPlay = DefaultVoiceSound;
    
    if (!DialogueLine.AudioAssetPath.IsEmpty())
    {
        // Try to load the specific audio asset
        SoundToPlay = LoadObject<USoundBase>(nullptr, *DialogueLine.AudioAssetPath);
        if (!SoundToPlay)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load audio asset: %s"), *DialogueLine.AudioAssetPath);
            SoundToPlay = DefaultVoiceSound;
        }
    }
    
    if (SoundToPlay)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), SoundToPlay);
        UE_LOG(LogTemp, Log, TEXT("Playing voice line for: %s"), *DialogueLine.SpeakerName);
    }
}