#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    bIsPlaying = false;
    CurrentDialogueTime = 0.0f;
    CurrentQueueIndex = 0;
    CurrentAudioComponent = nullptr;
    MasterVolume = 1.0f;
    LastUpdateTime = 0.0f;
}

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default voice type volumes
    VoiceTypeVolumes.Add(ENarr_DialogueType::Narrator, 0.8f);
    VoiceTypeVolumes.Add(ENarr_DialogueType::Warning, 1.0f);
    VoiceTypeVolumes.Add(ENarr_DialogueType::TribalElder, 0.9f);
    VoiceTypeVolumes.Add(ENarr_DialogueType::TrackerGuide, 0.85f);
    VoiceTypeVolumes.Add(ENarr_DialogueType::SystemMessage, 0.7f);
    
    InitializeDefaultSequences();
    
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem initialized"));
}

void UNarr_DialogueSystem::Deinitialize()
{
    StopCurrentDialogue();
    CleanupAudioComponents();
    DialogueSequences.Empty();
    DialogueQueue.Empty();
    SequenceCooldowns.Empty();
    PlayedOnceSequences.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueSystem::InitializeDefaultSequences()
{
    // Proximity-based survival guidance
    FNarr_DialogueSequence ProximitySequence;
    ProximitySequence.SequenceID = TEXT("ProximityGuidance");
    ProximitySequence.TriggerType = ENarr_DialogueTrigger::PlayerProximity;
    ProximitySequence.bPlayOnce = false;
    ProximitySequence.CooldownTime = 45.0f;
    
    FNarr_DialogueEntry ProximityEntry;
    ProximityEntry.DialogueText = TEXT("The ancient valley holds many dangers, survivor. Stay alert - the pack hunters communicate through low growls and coordinated movements.");
    ProximityEntry.VoiceType = ENarr_DialogueType::Narrator;
    ProximityEntry.DisplayDuration = 8.0f;
    ProximityEntry.Priority = 2;
    ProximitySequence.DialogueEntries.Add(ProximityEntry);
    
    RegisterDialogueSequence(ProximitySequence);
    
    // Danger warning sequence
    FNarr_DialogueSequence DangerSequence;
    DangerSequence.SequenceID = TEXT("DangerWarning");
    DangerSequence.TriggerType = ENarr_DialogueTrigger::DangerDetected;
    DangerSequence.bPlayOnce = false;
    DangerSequence.CooldownTime = 20.0f;
    
    FNarr_DialogueEntry DangerEntry;
    DangerEntry.DialogueText = TEXT("Danger approaches! The Thunder Lizard has caught your scent. Find cover behind the boulder formations!");
    DangerEntry.VoiceType = ENarr_DialogueType::Warning;
    DangerEntry.DisplayDuration = 6.0f;
    DangerEntry.Priority = 5;
    DangerSequence.DialogueEntries.Add(DangerEntry);
    
    RegisterDialogueSequence(DangerSequence);
    
    // Migration guidance sequence
    FNarr_DialogueSequence MigrationSequence;
    MigrationSequence.SequenceID = TEXT("MigrationGuidance");
    MigrationSequence.TriggerType = ENarr_DialogueTrigger::LocationDiscovered;
    MigrationSequence.bPlayOnce = true;
    MigrationSequence.CooldownTime = 0.0f;
    
    FNarr_DialogueEntry MigrationEntry;
    MigrationEntry.DialogueText = TEXT("The herbivore migration begins at sunrise. Follow the great long-necks to fresh water, but beware - predators hunt along their ancient paths.");
    MigrationEntry.VoiceType = ENarr_DialogueType::TribalElder;
    MigrationEntry.DisplayDuration = 10.0f;
    MigrationEntry.Priority = 3;
    MigrationSequence.DialogueEntries.Add(MigrationEntry);
    
    RegisterDialogueSequence(MigrationSequence);
    
    // Tracking guidance sequence
    FNarr_DialogueSequence TrackingSequence;
    TrackingSequence.SequenceID = TEXT("TrackingGuidance");
    TrackingSequence.TriggerType = ENarr_DialogueTrigger::DinosaurSighting;
    TrackingSequence.bPlayOnce = false;
    TrackingSequence.CooldownTime = 60.0f;
    
    FNarr_DialogueEntry TrackingEntry;
    TrackingEntry.DialogueText = TEXT("Fresh tracks in the mud - three-toed, deep impressions. The pack is close. They hunt in formation: two flankers, one direct assault.");
    TrackingEntry.VoiceType = ENarr_DialogueType::TrackerGuide;
    TrackingEntry.DisplayDuration = 8.0f;
    TrackingEntry.Priority = 4;
    TrackingSequence.DialogueEntries.Add(TrackingEntry);
    
    RegisterDialogueSequence(TrackingSequence);
}

void UNarr_DialogueSystem::PlayDialogue(const FString& SequenceID, bool bForcePlay)
{
    if (!bForcePlay && !CanPlayDialogue(SequenceID))
    {
        return;
    }
    
    const FNarr_DialogueSequence* Sequence = DialogueSequences.Find(SequenceID);
    if (!Sequence || Sequence->DialogueEntries.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found or empty: %s"), *SequenceID);
        return;
    }
    
    // Add entries to queue sorted by priority
    for (const FNarr_DialogueEntry& Entry : Sequence->DialogueEntries)
    {
        DialogueQueue.Add(Entry);
    }
    
    // Sort queue by priority (higher priority first)
    DialogueQueue.Sort([](const FNarr_DialogueEntry& A, const FNarr_DialogueEntry& B)
    {
        return A.Priority > B.Priority;
    });
    
    // Update cooldown
    if (Sequence->CooldownTime > 0.0f)
    {
        SequenceCooldowns.Add(SequenceID, GetWorld()->GetTimeSeconds() + Sequence->CooldownTime);
    }
    
    // Mark as played once if needed
    if (Sequence->bPlayOnce)
    {
        PlayedOnceSequences.Add(SequenceID);
    }
    
    // Start playback if not already playing
    if (!bIsPlaying && DialogueQueue.Num() > 0)
    {
        CurrentQueueIndex = 0;
        PlayDialogueEntry(DialogueQueue[CurrentQueueIndex]);
    }
}

void UNarr_DialogueSystem::PlayDialogueEntry(const FNarr_DialogueEntry& Entry)
{
    if (bIsPlaying)
    {
        StopCurrentDialogue();
    }
    
    CurrentDialogue = Entry;
    bIsPlaying = true;
    CurrentDialogueTime = 0.0f;
    
    // Get audio component
    CurrentAudioComponent = GetOrCreateAudioComponent();
    if (!CurrentAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create audio component for dialogue"));
        OnDialogueFinished();
        return;
    }
    
    // Set volume based on voice type and master volume
    float VoiceVolume = VoiceTypeVolumes.Contains(Entry.VoiceType) ? 
        VoiceTypeVolumes[Entry.VoiceType] : 1.0f;
    CurrentAudioComponent->SetVolumeMultiplier(MasterVolume * VoiceVolume);
    
    // Play audio if available
    if (Entry.AudioCue.IsValid())
    {
        USoundCue* SoundCue = Entry.AudioCue.LoadSynchronous();
        if (SoundCue)
        {
            CurrentAudioComponent->SetSound(SoundCue);
            CurrentAudioComponent->Play();
        }
    }
    
    // Display text (in a real implementation, this would update UI)
    UE_LOG(LogTemp, Log, TEXT("Dialogue: %s"), *Entry.DialogueText);
    
    // Set timer for dialogue duration
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, this, &UNarr_DialogueSystem::OnDialogueFinished, Entry.DisplayDuration, false);
    }
}

void UNarr_DialogueSystem::StopCurrentDialogue()
{
    if (CurrentAudioComponent && CurrentAudioComponent->IsPlaying())
    {
        CurrentAudioComponent->Stop();
    }
    
    bIsPlaying = false;
    CurrentDialogueTime = 0.0f;
    
    // Clear any pending timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }
}

bool UNarr_DialogueSystem::IsDialoguePlaying() const
{
    return bIsPlaying;
}

void UNarr_DialogueSystem::TriggerProximityDialogue(const FVector& PlayerLocation, const FVector& TriggerLocation, float Range)
{
    float Distance = FVector::Dist(PlayerLocation, TriggerLocation);
    if (Distance <= Range)
    {
        PlayDialogue(TEXT("ProximityGuidance"));
    }
}

void UNarr_DialogueSystem::TriggerDangerWarning(const FString& DinosaurType, const FVector& ThreatLocation)
{
    PlayDialogue(TEXT("DangerWarning"), true); // Force play for danger warnings
}

void UNarr_DialogueSystem::TriggerLocationDiscovery(const FString& LocationName)
{
    if (LocationName.Contains(TEXT("Migration")) || LocationName.Contains(TEXT("Watering")))
    {
        PlayDialogue(TEXT("MigrationGuidance"));
    }
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s"), *Sequence.SequenceID);
}

void UNarr_DialogueSystem::UnregisterDialogueSequence(const FString& SequenceID)
{
    DialogueSequences.Remove(SequenceID);
    SequenceCooldowns.Remove(SequenceID);
    PlayedOnceSequences.Remove(SequenceID);
}

FNarr_DialogueSequence UNarr_DialogueSystem::GetDialogueSequence(const FString& SequenceID) const
{
    const FNarr_DialogueSequence* Found = DialogueSequences.Find(SequenceID);
    return Found ? *Found : FNarr_DialogueSequence();
}

void UNarr_DialogueSystem::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UNarr_DialogueSystem::SetVoiceTypeVolume(ENarr_DialogueType VoiceType, float Volume)
{
    VoiceTypeVolumes.Add(VoiceType, FMath::Clamp(Volume, 0.0f, 1.0f));
}

bool UNarr_DialogueSystem::CanPlayDialogue(const FString& SequenceID) const
{
    // Check if already played once
    if (PlayedOnceSequences.Contains(SequenceID))
    {
        return false;
    }
    
    // Check cooldown
    const float* CooldownEnd = SequenceCooldowns.Find(SequenceID);
    if (CooldownEnd && GetWorld()->GetTimeSeconds() < *CooldownEnd)
    {
        return false;
    }
    
    return true;
}

void UNarr_DialogueSystem::OnDialogueFinished()
{
    bIsPlaying = false;
    
    // Move to next dialogue in queue
    CurrentQueueIndex++;
    if (CurrentQueueIndex < DialogueQueue.Num())
    {
        PlayDialogueEntry(DialogueQueue[CurrentQueueIndex]);
    }
    else
    {
        // Queue finished, clear it
        DialogueQueue.Empty();
        CurrentQueueIndex = 0;
    }
}

UAudioComponent* UNarr_DialogueSystem::GetOrCreateAudioComponent()
{
    // Try to find an available audio component
    for (UAudioComponent* Component : AudioComponents)
    {
        if (Component && !Component->IsPlaying())
        {
            return Component;
        }
    }
    
    // Create new audio component if needed
    if (UWorld* World = GetWorld())
    {
        UAudioComponent* NewComponent = NewObject<UAudioComponent>(this);
        if (NewComponent)
        {
            AudioComponents.Add(NewComponent);
            return NewComponent;
        }
    }
    
    return nullptr;
}

void UNarr_DialogueSystem::CleanupAudioComponents()
{
    for (UAudioComponent* Component : AudioComponents)
    {
        if (Component)
        {
            Component->Stop();
            Component->DestroyComponent();
        }
    }
    AudioComponents.Empty();
    CurrentAudioComponent = nullptr;
}