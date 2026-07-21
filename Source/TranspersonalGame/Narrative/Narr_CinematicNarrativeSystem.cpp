#include "Narr_CinematicNarrativeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UNarr_CinematicNarrativeSystem::UNarr_CinematicNarrativeSystem()
{
    CurrentCinematicID = TEXT("");
    bCinematicActive = false;
    CinematicStartTime = 0.0f;
    CinematicAudioComponent = nullptr;
}

void UNarr_CinematicNarrativeSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Narr_CinematicNarrativeSystem: Initializing cinematic narrative system"));
    
    // Initialize default cinematic sequences
    InitializeDefaultSequences();
    
    // Create audio component for cinematic playback
    if (UWorld* World = GetWorld())
    {
        CinematicAudioComponent = NewObject<UAudioComponent>(this);
        if (CinematicAudioComponent)
        {
            CinematicAudioComponent->bAutoActivate = false;
            CinematicAudioComponent->SetVolumeMultiplier(1.0f);
        }
    }
}

void UNarr_CinematicNarrativeSystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Narr_CinematicNarrativeSystem: Deinitializing cinematic narrative system"));
    
    // Clean up any active cinematics
    if (bCinematicActive)
    {
        CleanupCurrentCinematic();
    }
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CinematicTimerHandle);
    }
    
    // Clear sequences
    CinematicSequences.Empty();
    
    Super::Deinitialize();
}

void UNarr_CinematicNarrativeSystem::InitializeDefaultSequences()
{
    // Intro sequence
    FNarr_CinematicSequence IntroSequence;
    IntroSequence.SequenceID = TEXT("GameIntro");
    IntroSequence.CinematicType = ENarr_CinematicType::IntroSequence;
    IntroSequence.NarrativeText = FText::FromString(TEXT("Day 239 in the Cretaceous wilderness. The morning reveals disturbing signs - massive three-toed prints near our shelter, each one the size of a dinner plate. The tracks lead toward the river crossing where we gather water. Something big is hunting in our territory."));
    IntroSequence.Duration = 18.0f;
    IntroSequence.bSkippable = true;
    CinematicSequences.Add(IntroSequence.SequenceID, IntroSequence);
    
    // Threat warning sequence
    FNarr_CinematicSequence ThreatSequence;
    ThreatSequence.SequenceID = TEXT("CarnotaurusThreat");
    ThreatSequence.CinematicType = ENarr_CinematicType::ThreatWarning;
    ThreatSequence.NarrativeText = FText::FromString(TEXT("Critical threat assessment! Massive Carnotaurus pack detected approaching from the eastern cliffs. Count confirmed at seven adults, moving in coordinated hunting formation. Immediate evacuation to high ground recommended. This is not a drill - these predators show advanced pack tactics."));
    ThreatSequence.Duration = 20.0f;
    ThreatSequence.bSkippable = false;
    CinematicSequences.Add(ThreatSequence.SequenceID, ThreatSequence);
    
    // Migration event sequence
    FNarr_CinematicSequence MigrationSequence;
    MigrationSequence.SequenceID = TEXT("HerdMigration");
    MigrationSequence.CinematicType = ENarr_CinematicType::MajorEvent;
    MigrationSequence.NarrativeText = FText::FromString(TEXT("Listen carefully, survivor. The Parasaurolophus herd migration begins at dawn. Their calls echo through the valley - a haunting sound that means both opportunity and danger. Where the herbivores go, the predators follow. Stay alert."));
    MigrationSequence.Duration = 16.0f;
    MigrationSequence.bSkippable = true;
    CinematicSequences.Add(MigrationSequence.SequenceID, MigrationSequence);
    
    // Emergency protocol sequence
    FNarr_CinematicSequence EmergencySequence;
    EmergencySequence.SequenceID = TEXT("VolcanicEmergency");
    EmergencySequence.CinematicType = ENarr_CinematicType::EmergencyBroadcast;
    EmergencySequence.NarrativeText = FText::FromString(TEXT("Emergency shelter protocol activated. Volcanic ash clouds approaching from the southwest. All personnel must reach reinforced caves within the next two hours. The ash will block sunlight for days and make breathing dangerous. Move now!"));
    EmergencySequence.Duration = 16.0f;
    EmergencySequence.bSkippable = false;
    CinematicSequences.Add(EmergencySequence.SequenceID, EmergencySequence);
    
    UE_LOG(LogTemp, Log, TEXT("Narr_CinematicNarrativeSystem: Initialized %d default cinematic sequences"), CinematicSequences.Num());
}

void UNarr_CinematicNarrativeSystem::PlayCinematicSequence(const FString& SequenceID)
{
    if (bCinematicActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_CinematicNarrativeSystem: Cannot play sequence %s - another cinematic is already active"), *SequenceID);
        return;
    }
    
    FNarr_CinematicSequence* Sequence = CinematicSequences.Find(SequenceID);
    if (!Sequence)
    {
        UE_LOG(LogTemp, Error, TEXT("Narr_CinematicNarrativeSystem: Sequence %s not found"), *SequenceID);
        return;
    }
    
    // Check trigger conditions
    if (!CheckTriggerConditions(Sequence->TriggerConditions))
    {
        UE_LOG(LogTemp, Log, TEXT("Narr_CinematicNarrativeSystem: Trigger conditions not met for sequence %s"), *SequenceID);
        return;
    }
    
    // Start cinematic
    CurrentCinematicID = SequenceID;
    bCinematicActive = true;
    CinematicStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Narr_CinematicNarrativeSystem: Playing cinematic sequence %s (Duration: %.1fs)"), *SequenceID, Sequence->Duration);
    
    // Play audio if available
    if (Sequence->VoiceAudio.IsValid() && CinematicAudioComponent)
    {
        USoundBase* AudioAsset = Sequence->VoiceAudio.LoadSynchronous();
        if (AudioAsset)
        {
            CinematicAudioComponent->SetSound(AudioAsset);
            CinematicAudioComponent->Play();
        }
    }
    
    // Set timer for sequence duration
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CinematicTimerHandle,
            this,
            &UNarr_CinematicNarrativeSystem::OnCinematicTimerComplete,
            Sequence->Duration,
            false
        );
    }
    
    // Broadcast start event
    OnCinematicStarted.Broadcast(SequenceID);
}

void UNarr_CinematicNarrativeSystem::SkipCurrentCinematic()
{
    if (!bCinematicActive)
    {
        return;
    }
    
    FNarr_CinematicSequence* Sequence = CinematicSequences.Find(CurrentCinematicID);
    if (Sequence && !Sequence->bSkippable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_CinematicNarrativeSystem: Cannot skip non-skippable sequence %s"), *CurrentCinematicID);
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Narr_CinematicNarrativeSystem: Skipping cinematic sequence %s"), *CurrentCinematicID);
    
    // Broadcast skip event
    OnCinematicSkipped.Broadcast(CurrentCinematicID);
    
    // Clean up and complete
    CleanupCurrentCinematic();
}

void UNarr_CinematicNarrativeSystem::RegisterCinematicSequence(const FNarr_CinematicSequence& Sequence)
{
    if (Sequence.SequenceID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Narr_CinematicNarrativeSystem: Cannot register sequence with empty ID"));
        return;
    }
    
    CinematicSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Narr_CinematicNarrativeSystem: Registered cinematic sequence %s"), *Sequence.SequenceID);
}

bool UNarr_CinematicNarrativeSystem::IsCinematicPlaying() const
{
    return bCinematicActive;
}

FString UNarr_CinematicNarrativeSystem::GetCurrentCinematicID() const
{
    return CurrentCinematicID;
}

void UNarr_CinematicNarrativeSystem::TriggerIntroSequence()
{
    PlayCinematicSequence(TEXT("GameIntro"));
}

void UNarr_CinematicNarrativeSystem::TriggerChapterTransition(const FString& ChapterName)
{
    // Create dynamic chapter transition sequence
    FNarr_CinematicSequence ChapterSequence;
    ChapterSequence.SequenceID = FString::Printf(TEXT("Chapter_%s"), *ChapterName);
    ChapterSequence.CinematicType = ENarr_CinematicType::ChapterTransition;
    ChapterSequence.NarrativeText = FText::FromString(FString::Printf(TEXT("Chapter: %s"), *ChapterName));
    ChapterSequence.Duration = 8.0f;
    ChapterSequence.bSkippable = true;
    
    RegisterCinematicSequence(ChapterSequence);
    PlayCinematicSequence(ChapterSequence.SequenceID);
}

void UNarr_CinematicNarrativeSystem::TriggerEmergencyBroadcast(const FString& ThreatType)
{
    if (ThreatType.Contains(TEXT("Carnotaurus")))
    {
        PlayCinematicSequence(TEXT("CarnotaurusThreat"));
    }
    else if (ThreatType.Contains(TEXT("Volcanic")))
    {
        PlayCinematicSequence(TEXT("VolcanicEmergency"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_CinematicNarrativeSystem: Unknown threat type for emergency broadcast: %s"), *ThreatType);
    }
}

void UNarr_CinematicNarrativeSystem::TriggerEnvironmentalNarration(const FString& LocationContext)
{
    if (LocationContext.Contains(TEXT("Migration")))
    {
        PlayCinematicSequence(TEXT("HerdMigration"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Narr_CinematicNarrativeSystem: No specific sequence for location context: %s"), *LocationContext);
    }
}

void UNarr_CinematicNarrativeSystem::OnCinematicTimerComplete()
{
    if (bCinematicActive)
    {
        UE_LOG(LogTemp, Log, TEXT("Narr_CinematicNarrativeSystem: Cinematic sequence %s completed"), *CurrentCinematicID);
        
        // Broadcast completion event
        OnCinematicCompleted.Broadcast(CurrentCinematicID);
        
        // Clean up
        CleanupCurrentCinematic();
    }
}

void UNarr_CinematicNarrativeSystem::CleanupCurrentCinematic()
{
    // Stop audio
    if (CinematicAudioComponent && CinematicAudioComponent->IsPlaying())
    {
        CinematicAudioComponent->Stop();
    }
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CinematicTimerHandle);
    }
    
    // Reset state
    CurrentCinematicID = TEXT("");
    bCinematicActive = false;
    CinematicStartTime = 0.0f;
}

bool UNarr_CinematicNarrativeSystem::CheckTriggerConditions(const TArray<FString>& Conditions)
{
    // For now, return true - conditions can be implemented later
    // This could check game state, player progress, time of day, etc.
    return true;
}