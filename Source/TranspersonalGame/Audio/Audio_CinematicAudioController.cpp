#include "Audio_CinematicAudioController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

AAudio_CinematicAudioController::AAudio_CinematicAudioController()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create audio components
    VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudioComponent"));

    // Set root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Attach audio components
    VoiceAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->SetupAttachment(RootComponent);
    AmbienceAudioComponent->SetupAttachment(RootComponent);

    // Configure audio components
    VoiceAudioComponent->bAutoActivate = false;
    MusicAudioComponent->bAutoActivate = false;
    AmbienceAudioComponent->bAutoActivate = false;

    VoiceAudioComponent->SetVolumeMultiplier(1.0f);
    MusicAudioComponent->SetVolumeMultiplier(0.6f);
    AmbienceAudioComponent->SetVolumeMultiplier(0.4f);

    // Initialize state
    CurrentSequenceIndex = -1;
    bIsPlayingSequence = false;
    CurrentSequenceTime = 0.0f;
    bIsFading = false;
    FadeStartVolume = 0.0f;
    FadeTargetVolume = 0.0f;
    FadeDuration = 2.0f;
    FadeElapsedTime = 0.0f;
    FadeTargetComponent = nullptr;

    // Initialize default sequences
    InitializeDefaultSequences();
}

void AAudio_CinematicAudioController::BeginPlay()
{
    Super::BeginPlay();
    
    // Additional setup if needed
}

void AAudio_CinematicAudioController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SequenceTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
    }

    Super::EndPlay(EndPlayReason);
}

void AAudio_CinematicAudioController::InitializeDefaultSequences()
{
    // Volcanic Emergency Sequence
    FAudio_CinematicSequence VolcanicSequence;
    VolcanicSequence.SequenceName = TEXT("VolcanicEmergency");
    VolcanicSequence.SequenceDuration = 37.0f;
    VolcanicSequence.VoiceVolume = 1.0f;
    VolcanicSequence.MusicVolume = 0.3f;
    VolcanicSequence.AmbienceVolume = 0.7f;
    VolcanicSequence.bCanBeSkipped = false; // Emergency - cannot skip
    VolcanicSequence.bFadeInMusic = false;
    VolcanicSequence.bFadeOutMusic = true;
    CinematicSequences.Add(VolcanicSequence);

    // Herd Migration Sequence
    FAudio_CinematicSequence HerdSequence;
    HerdSequence.SequenceName = TEXT("HerdMigration");
    HerdSequence.SequenceDuration = 44.0f;
    HerdSequence.VoiceVolume = 0.9f;
    HerdSequence.MusicVolume = 0.5f;
    HerdSequence.AmbienceVolume = 0.6f;
    HerdSequence.bCanBeSkipped = true;
    HerdSequence.bFadeInMusic = true;
    HerdSequence.bFadeOutMusic = true;
    CinematicSequences.Add(HerdSequence);

    // Survival Narrative Sequence
    FAudio_CinematicSequence SurvivalSequence;
    SurvivalSequence.SequenceName = TEXT("SurvivalNarrative");
    SurvivalSequence.SequenceDuration = 18.0f;
    SurvivalSequence.VoiceVolume = 1.0f;
    SurvivalSequence.MusicVolume = 0.4f;
    SurvivalSequence.AmbienceVolume = 0.5f;
    SurvivalSequence.bCanBeSkipped = true;
    SurvivalSequence.bFadeInMusic = true;
    SurvivalSequence.bFadeOutMusic = false;
    CinematicSequences.Add(SurvivalSequence);

    // Threat Assessment Sequence
    FAudio_CinematicSequence ThreatSequence;
    ThreatSequence.SequenceName = TEXT("ThreatAssessment");
    ThreatSequence.SequenceDuration = 20.0f;
    ThreatSequence.VoiceVolume = 1.0f;
    ThreatSequence.MusicVolume = 0.2f;
    ThreatSequence.AmbienceVolume = 0.3f;
    ThreatSequence.bCanBeSkipped = false; // Critical threat info
    ThreatSequence.bFadeInMusic = false;
    ThreatSequence.bFadeOutMusic = true;
    CinematicSequences.Add(ThreatSequence);
}

void AAudio_CinematicAudioController::PlaySequence(int32 SequenceIndex)
{
    if (SequenceIndex < 0 || SequenceIndex >= CinematicSequences.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid sequence index: %d"), SequenceIndex);
        return;
    }

    // Stop current sequence if playing
    if (bIsPlayingSequence)
    {
        StopCurrentSequence();
    }

    StartSequence(SequenceIndex);
}

void AAudio_CinematicAudioController::PlaySequenceByName(const FString& SequenceName)
{
    for (int32 i = 0; i < CinematicSequences.Num(); i++)
    {
        if (CinematicSequences[i].SequenceName.Equals(SequenceName, ESearchCase::IgnoreCase))
        {
            PlaySequence(i);
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Sequence not found: %s"), *SequenceName);
}

void AAudio_CinematicAudioController::StopCurrentSequence()
{
    if (!bIsPlayingSequence)
    {
        return;
    }

    // Stop all audio components
    VoiceAudioComponent->Stop();
    MusicAudioComponent->Stop();
    AmbienceAudioComponent->Stop();

    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SequenceTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
    }

    // Reset state
    bIsPlayingSequence = false;
    CurrentSequenceIndex = -1;
    CurrentSequenceTime = 0.0f;
    bIsFading = false;
}

void AAudio_CinematicAudioController::SkipCurrentSequence()
{
    if (!bIsPlayingSequence)
    {
        return;
    }

    const FAudio_CinematicSequence& CurrentSequence = CinematicSequences[CurrentSequenceIndex];
    if (!CurrentSequence.bCanBeSkipped)
    {
        UE_LOG(LogTemp, Warning, TEXT("Current sequence cannot be skipped: %s"), *CurrentSequence.SequenceName);
        return;
    }

    EndSequence();
}

void AAudio_CinematicAudioController::PlayNextSequence()
{
    if (CurrentSequenceIndex + 1 < CinematicSequences.Num())
    {
        PlaySequence(CurrentSequenceIndex + 1);
    }
}

void AAudio_CinematicAudioController::PlayPreviousSequence()
{
    if (CurrentSequenceIndex > 0)
    {
        PlaySequence(CurrentSequenceIndex - 1);
    }
}

void AAudio_CinematicAudioController::SetVoiceVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    VoiceAudioComponent->SetVolumeMultiplier(Volume);
}

void AAudio_CinematicAudioController::SetMusicVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    MusicAudioComponent->SetVolumeMultiplier(Volume);
}

void AAudio_CinematicAudioController::SetAmbienceVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    AmbienceAudioComponent->SetVolumeMultiplier(Volume);
}

void AAudio_CinematicAudioController::FadeInMusic(float FadeDuration)
{
    if (!MusicAudioComponent || bIsFading)
    {
        return;
    }

    bIsFading = true;
    FadeStartVolume = 0.0f;
    FadeTargetVolume = CinematicSequences[CurrentSequenceIndex].MusicVolume;
    this->FadeDuration = FadeDuration;
    FadeElapsedTime = 0.0f;
    FadeTargetComponent = MusicAudioComponent;

    MusicAudioComponent->SetVolumeMultiplier(0.0f);

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &AAudio_CinematicAudioController::HandleFadeTimer, 0.1f, true);
    }
}

void AAudio_CinematicAudioController::FadeOutMusic(float FadeDuration)
{
    if (!MusicAudioComponent || bIsFading)
    {
        return;
    }

    bIsFading = true;
    FadeStartVolume = MusicAudioComponent->GetVolumeMultiplier();
    FadeTargetVolume = 0.0f;
    this->FadeDuration = FadeDuration;
    FadeElapsedTime = 0.0f;
    FadeTargetComponent = MusicAudioComponent;

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &AAudio_CinematicAudioController::HandleFadeTimer, 0.1f, true);
    }
}

void AAudio_CinematicAudioController::AddSequence(const FAudio_CinematicSequence& NewSequence)
{
    CinematicSequences.Add(NewSequence);
}

void AAudio_CinematicAudioController::RemoveSequence(int32 SequenceIndex)
{
    if (SequenceIndex >= 0 && SequenceIndex < CinematicSequences.Num())
    {
        CinematicSequences.RemoveAt(SequenceIndex);
    }
}

void AAudio_CinematicAudioController::ClearAllSequences()
{
    StopCurrentSequence();
    CinematicSequences.Empty();
}

FAudio_CinematicSequence AAudio_CinematicAudioController::GetSequence(int32 SequenceIndex) const
{
    if (SequenceIndex >= 0 && SequenceIndex < CinematicSequences.Num())
    {
        return CinematicSequences[SequenceIndex];
    }
    return FAudio_CinematicSequence();
}

void AAudio_CinematicAudioController::StartSequence(int32 SequenceIndex)
{
    CurrentSequenceIndex = SequenceIndex;
    const FAudio_CinematicSequence& Sequence = CinematicSequences[SequenceIndex];

    // Load and configure audio
    LoadSequenceAudio(Sequence);

    // Set volumes
    VoiceAudioComponent->SetVolumeMultiplier(Sequence.VoiceVolume);
    AmbienceAudioComponent->SetVolumeMultiplier(Sequence.AmbienceVolume);

    // Handle music fade in
    if (Sequence.bFadeInMusic)
    {
        MusicAudioComponent->SetVolumeMultiplier(0.0f);
        FadeInMusic(2.0f);
    }
    else
    {
        MusicAudioComponent->SetVolumeMultiplier(Sequence.MusicVolume);
    }

    // Start audio playback
    VoiceAudioComponent->Play();
    MusicAudioComponent->Play();
    AmbienceAudioComponent->Play();

    // Set sequence state
    bIsPlayingSequence = true;
    CurrentSequenceTime = 0.0f;

    // Set timer for sequence end
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(SequenceTimerHandle, this, &AAudio_CinematicAudioController::EndSequence, Sequence.SequenceDuration, false);
    }

    UE_LOG(LogTemp, Log, TEXT("Started cinematic sequence: %s (Duration: %.1fs)"), *Sequence.SequenceName, Sequence.SequenceDuration);
}

void AAudio_CinematicAudioController::EndSequence()
{
    if (!bIsPlayingSequence)
    {
        return;
    }

    const FAudio_CinematicSequence& Sequence = CinematicSequences[CurrentSequenceIndex];

    // Handle music fade out
    if (Sequence.bFadeOutMusic)
    {
        FadeOutMusic(1.5f);
    }

    // Stop voice and ambience immediately
    VoiceAudioComponent->Stop();
    AmbienceAudioComponent->Stop();

    // If not fading music, stop it too
    if (!Sequence.bFadeOutMusic)
    {
        MusicAudioComponent->Stop();
    }

    // Clear sequence timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SequenceTimerHandle);
    }

    // Reset state
    bIsPlayingSequence = false;
    CurrentSequenceTime = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Ended cinematic sequence: %s"), *Sequence.SequenceName);
}

void AAudio_CinematicAudioController::UpdateSequenceTimer()
{
    if (bIsPlayingSequence)
    {
        CurrentSequenceTime += 0.1f;
    }
}

void AAudio_CinematicAudioController::HandleFadeTimer()
{
    if (!bIsFading || !FadeTargetComponent)
    {
        return;
    }

    FadeElapsedTime += 0.1f;
    float FadeProgress = FadeElapsedTime / FadeDuration;

    if (FadeProgress >= 1.0f)
    {
        // Fade complete
        FadeTargetComponent->SetVolumeMultiplier(FadeTargetVolume);
        bIsFading = false;
        
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
        }

        // If fading out to zero, stop the component
        if (FadeTargetVolume <= 0.0f)
        {
            FadeTargetComponent->Stop();
        }
    }
    else
    {
        // Continue fade
        float CurrentVolume = FMath::Lerp(FadeStartVolume, FadeTargetVolume, FadeProgress);
        FadeTargetComponent->SetVolumeMultiplier(CurrentVolume);
    }
}

void AAudio_CinematicAudioController::LoadSequenceAudio(const FAudio_CinematicSequence& Sequence)
{
    // In a full implementation, this would load the actual audio assets
    // For now, we'll use placeholder logic
    
    UE_LOG(LogTemp, Log, TEXT("Loading audio for sequence: %s"), *Sequence.SequenceName);
    
    // This would typically load USoundWave assets from the sequence configuration
    // and assign them to the audio components
}