#include "DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UDialogueManager::UDialogueManager()
{
    bIsCurrentlyPlaying = false;
    CurrentSequenceID = TEXT("");
    CurrentEntryIndex = 0;
    CurrentEntryTimer = 0.0f;
}

void UDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeThreatWarnings();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager initialized successfully"));
}

void UDialogueManager::Deinitialize()
{
    if (GetWorld() && DialogueUpdateHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(DialogueUpdateHandle);
    }
    
    Super::Deinitialize();
}

void UDialogueManager::InitializeThreatWarnings()
{
    ThreatWarnings.Empty();
    
    ThreatWarnings.Add(ESurvivalThreat::None, TEXT("Area is safe. Continue exploring."));
    ThreatWarnings.Add(ESurvivalThreat::Low, TEXT("Stay alert. Potential dangers nearby."));
    ThreatWarnings.Add(ESurvivalThreat::Medium, TEXT("Caution advised. Predators may be in the area."));
    ThreatWarnings.Add(ESurvivalThreat::High, TEXT("Danger! Large predator detected. Seek immediate cover."));
    ThreatWarnings.Add(ESurvivalThreat::Extreme, TEXT("EXTREME DANGER! Apex predator nearby. Evacuate immediately!"));
}

void UDialogueManager::PlayDialogueSequence(const FString& SequenceID)
{
    if (!CanPlaySequence(SequenceID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play dialogue sequence: %s (on cooldown or not found)"), *SequenceID);
        return;
    }

    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            if (bIsCurrentlyPlaying)
            {
                StopCurrentDialogue();
            }

            CurrentSequenceID = SequenceID;
            CurrentEntryIndex = 0;
            CurrentEntryTimer = 0.0f;
            bIsCurrentlyPlaying = true;

            // Set cooldown
            SequenceCooldowns.Add(SequenceID, GetWorld()->GetTimeSeconds() + Sequence.CooldownTime);

            // Start dialogue update timer
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().SetTimer(
                    DialogueUpdateHandle,
                    this,
                    &UDialogueManager::UpdateDialoguePlayback,
                    0.1f,
                    true
                );
            }

            PlayNextDialogueEntry();
            break;
        }
    }
}

void UDialogueManager::PlaySurvivalWarning(ESurvivalThreat ThreatType, const FString& CustomMessage)
{
    FString WarningText;
    
    if (!CustomMessage.IsEmpty())
    {
        WarningText = CustomMessage;
    }
    else if (ThreatWarnings.Contains(ThreatType))
    {
        WarningText = ThreatWarnings[ThreatType];
    }
    else
    {
        WarningText = TEXT("Unknown threat level detected.");
    }

    PlayNarration(WarningText, 4.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Survival Warning: %s"), *WarningText);
}

void UDialogueManager::PlayNarration(const FString& NarrationText, float Duration)
{
    if (bIsCurrentlyPlaying)
    {
        StopCurrentDialogue();
    }

    // Create temporary dialogue sequence for narration
    FNarr_DialogueSequence NarrationSequence;
    NarrationSequence.SequenceID = TEXT("NARRATION_TEMP");
    
    FNarr_DialogueEntry NarrationEntry;
    NarrationEntry.SpeakerName = TEXT("Narrator");
    NarrationEntry.DialogueText = NarrationText;
    NarrationEntry.Duration = Duration;
    NarrationEntry.bIsNarration = true;
    
    NarrationSequence.DialogueEntries.Add(NarrationEntry);
    
    // Add to sequences temporarily
    DialogueSequences.Add(NarrationSequence);
    
    PlayDialogueSequence(TEXT("NARRATION_TEMP"));
}

bool UDialogueManager::IsDialoguePlaying() const
{
    return bIsCurrentlyPlaying;
}

void UDialogueManager::StopCurrentDialogue()
{
    bIsCurrentlyPlaying = false;
    CurrentSequenceID = TEXT("");
    CurrentEntryIndex = 0;
    CurrentEntryTimer = 0.0f;

    if (GetWorld() && DialogueUpdateHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(DialogueUpdateHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("Dialogue stopped"));
}

void UDialogueManager::RegisterDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    // Remove existing sequence with same ID
    DialogueSequences.RemoveAll([&](const FNarr_DialogueSequence& Sequence)
    {
        return Sequence.SequenceID == NewSequence.SequenceID;
    });

    DialogueSequences.Add(NewSequence);
    
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s"), *NewSequence.SequenceID);
}

void UDialogueManager::UpdateDialoguePlayback(float DeltaTime)
{
    if (!bIsCurrentlyPlaying)
    {
        return;
    }

    CurrentEntryTimer += 0.1f; // Timer interval

    // Find current sequence
    const FNarr_DialogueSequence* CurrentSequence = nullptr;
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == CurrentSequenceID)
        {
            CurrentSequence = &Sequence;
            break;
        }
    }

    if (!CurrentSequence || CurrentEntryIndex >= CurrentSequence->DialogueEntries.Num())
    {
        OnDialogueSequenceComplete();
        return;
    }

    const FNarr_DialogueEntry& CurrentEntry = CurrentSequence->DialogueEntries[CurrentEntryIndex];
    
    if (CurrentEntryTimer >= CurrentEntry.Duration)
    {
        CurrentEntryIndex++;
        CurrentEntryTimer = 0.0f;
        
        if (CurrentEntryIndex < CurrentSequence->DialogueEntries.Num())
        {
            PlayNextDialogueEntry();
        }
        else
        {
            OnDialogueSequenceComplete();
        }
    }
}

void UDialogueManager::PlayNextDialogueEntry()
{
    const FNarr_DialogueSequence* CurrentSequence = nullptr;
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == CurrentSequenceID)
        {
            CurrentSequence = &Sequence;
            break;
        }
    }

    if (!CurrentSequence || CurrentEntryIndex >= CurrentSequence->DialogueEntries.Num())
    {
        return;
    }

    const FNarr_DialogueEntry& Entry = CurrentSequence->DialogueEntries[CurrentEntryIndex];
    
    // Display dialogue text (in a real implementation, this would trigger UI)
    FString DisplayText = FString::Printf(TEXT("[%s]: %s"), *Entry.SpeakerName, *Entry.DialogueText);
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue: %s"), *DisplayText);
    
    // In a full implementation, this would also trigger audio playback
    // and UI updates to show the dialogue text to the player
}

void UDialogueManager::OnDialogueSequenceComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Dialogue sequence complete: %s"), *CurrentSequenceID);
    
    // Clean up temporary narration sequences
    if (CurrentSequenceID == TEXT("NARRATION_TEMP"))
    {
        DialogueSequences.RemoveAll([](const FNarr_DialogueSequence& Sequence)
        {
            return Sequence.SequenceID == TEXT("NARRATION_TEMP");
        });
    }
    
    StopCurrentDialogue();
}

bool UDialogueManager::CanPlaySequence(const FString& SequenceID) const
{
    // Check if sequence exists
    bool bSequenceExists = false;
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            bSequenceExists = true;
            break;
        }
    }

    if (!bSequenceExists)
    {
        return false;
    }

    // Check cooldown
    if (SequenceCooldowns.Contains(SequenceID))
    {
        float CooldownEndTime = SequenceCooldowns[SequenceID];
        float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        
        return CurrentTime >= CooldownEndTime;
    }

    return true;
}