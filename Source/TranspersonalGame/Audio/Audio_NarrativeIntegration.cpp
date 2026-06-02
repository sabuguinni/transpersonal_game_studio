#include "Audio_NarrativeIntegration.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UAudio_NarrativeIntegration::UAudio_NarrativeIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    UpdateInterval = 1.0f;

    // Create audio components
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudioComponent"));

    if (MusicAudioComponent)
    {
        MusicAudioComponent->bAutoActivate = false;
        MusicAudioComponent->SetVolumeMultiplier(0.7f);
    }

    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->bAutoActivate = false;
        AmbienceAudioComponent->SetVolumeMultiplier(0.5f);
    }

    // Initialize default state
    CurrentAudioState.CurrentIntensity = EAudio_NarrativeIntensity::Calm;
    CurrentAudioState.IntensityLevel = 0.0f;
    CurrentAudioState.CurrentPhase = TEXT("Awakening");
    CurrentAudioState.bIsInStoryLocation = false;
    CurrentAudioState.ThreatLevel = 0.0f;
}

void UAudio_NarrativeIntegration::BeginPlay()
{
    Super::BeginPlay();

    // Start periodic audio updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            AudioUpdateTimer,
            this,
            &UAudio_NarrativeIntegration::UpdateAudioIntensity,
            UpdateInterval,
            true
        );
    }

    // Initialize with calm music
    TransitionToIntensity(EAudio_NarrativeIntensity::Calm);
}

void UAudio_NarrativeIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateAudioState();
}

void UAudio_NarrativeIntegration::OnNarrativePhaseChanged(const FString& NewPhase, float IntensityLevel)
{
    CurrentAudioState.CurrentPhase = NewPhase;
    CurrentAudioState.IntensityLevel = IntensityLevel;

    // Determine intensity based on phase and level
    EAudio_NarrativeIntensity NewIntensity = EAudio_NarrativeIntensity::Calm;

    if (NewPhase == TEXT("Discovery") || NewPhase == TEXT("Adaptation"))
    {
        NewIntensity = EAudio_NarrativeIntensity::Discovery;
    }
    else if (NewPhase == TEXT("Mastery") || NewPhase == TEXT("Leadership"))
    {
        if (IntensityLevel > 0.7f)
        {
            NewIntensity = EAudio_NarrativeIntensity::Tension;
        }
        else
        {
            NewIntensity = EAudio_NarrativeIntensity::Calm;
        }
    }
    else if (NewPhase == TEXT("Legacy"))
    {
        NewIntensity = EAudio_NarrativeIntensity::Discovery;
    }

    TransitionToIntensity(NewIntensity);

    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Phase changed to %s with intensity %f"), *NewPhase, IntensityLevel);
}

void UAudio_NarrativeIntegration::OnThreatLevelChanged(float NewThreatLevel)
{
    CurrentAudioState.ThreatLevel = NewThreatLevel;

    // Override intensity based on threat level
    EAudio_NarrativeIntensity ThreatIntensity = CurrentAudioState.CurrentIntensity;

    if (NewThreatLevel > 0.8f)
    {
        ThreatIntensity = EAudio_NarrativeIntensity::Combat;
    }
    else if (NewThreatLevel > 0.5f)
    {
        ThreatIntensity = EAudio_NarrativeIntensity::Danger;
    }
    else if (NewThreatLevel > 0.2f)
    {
        ThreatIntensity = EAudio_NarrativeIntensity::Tension;
    }
    else
    {
        // Return to phase-based intensity
        OnNarrativePhaseChanged(CurrentAudioState.CurrentPhase, CurrentAudioState.IntensityLevel);
        return;
    }

    TransitionToIntensity(ThreatIntensity);

    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Threat level changed to %f"), NewThreatLevel);
}

void UAudio_NarrativeIntegration::OnStoryLocationEntered(const FString& LocationName)
{
    CurrentAudioState.bIsInStoryLocation = true;

    // Play discovery music for story locations
    TransitionToIntensity(EAudio_NarrativeIntensity::Discovery);

    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Entered story location: %s"), *LocationName);
}

void UAudio_NarrativeIntegration::OnStoryLocationExited()
{
    CurrentAudioState.bIsInStoryLocation = false;

    // Return to normal intensity based on current state
    OnNarrativePhaseChanged(CurrentAudioState.CurrentPhase, CurrentAudioState.IntensityLevel);

    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Exited story location"));
}

void UAudio_NarrativeIntegration::UpdateAudioIntensity()
{
    // This is called periodically to update audio based on current game state
    UpdateAudioState();
}

void UAudio_NarrativeIntegration::TransitionToIntensity(EAudio_NarrativeIntensity NewIntensity)
{
    if (CurrentAudioState.CurrentIntensity == NewIntensity)
    {
        return;
    }

    CurrentAudioState.CurrentIntensity = NewIntensity;
    PlayMusicForIntensity(NewIntensity);

    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Transitioned to intensity: %d"), (int32)NewIntensity);
}

void UAudio_NarrativeIntegration::SetMusicVolume(float Volume)
{
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.0f, 1.0f));
    }
}

void UAudio_NarrativeIntegration::SetAmbienceVolume(float Volume)
{
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.0f, 1.0f));
    }
}

void UAudio_NarrativeIntegration::UpdateAudioState()
{
    // Update audio state based on current conditions
    // This could check for nearby threats, player health, etc.
    
    // Example: Reduce music volume if player is low on health
    if (AActor* Owner = GetOwner())
    {
        // Check if owner has health component and adjust audio accordingly
        // This would integrate with the survival system
    }
}

void UAudio_NarrativeIntegration::PlayMusicForIntensity(EAudio_NarrativeIntensity Intensity)
{
    USoundCue* NewMusic = GetSoundCueForIntensity(Intensity);
    
    if (NewMusic && MusicAudioComponent)
    {
        CrossfadeMusic(NewMusic);
    }
}

USoundCue* UAudio_NarrativeIntegration::GetSoundCueForIntensity(EAudio_NarrativeIntensity Intensity)
{
    switch (Intensity)
    {
        case EAudio_NarrativeIntensity::Calm:
            return AudioConfig.CalmMusic.LoadSynchronous();
        case EAudio_NarrativeIntensity::Tension:
            return AudioConfig.TensionMusic.LoadSynchronous();
        case EAudio_NarrativeIntensity::Danger:
            return AudioConfig.DangerMusic.LoadSynchronous();
        case EAudio_NarrativeIntensity::Combat:
            return AudioConfig.CombatMusic.LoadSynchronous();
        case EAudio_NarrativeIntensity::Discovery:
            return AudioConfig.DiscoveryMusic.LoadSynchronous();
        default:
            return AudioConfig.CalmMusic.LoadSynchronous();
    }
}

void UAudio_NarrativeIntegration::CrossfadeMusic(USoundCue* NewMusic)
{
    if (!MusicAudioComponent || !NewMusic)
    {
        return;
    }

    // Simple crossfade implementation
    if (MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(AudioConfig.TransitionDuration, 0.0f);
        
        // Start new music after fade out
        if (UWorld* World = GetWorld())
        {
            FTimerHandle FadeInTimer;
            World->GetTimerManager().SetTimer(
                FadeInTimer,
                [this, NewMusic]()
                {
                    if (MusicAudioComponent)
                    {
                        MusicAudioComponent->SetSound(NewMusic);
                        MusicAudioComponent->FadeIn(AudioConfig.TransitionDuration, 0.7f);
                    }
                },
                AudioConfig.TransitionDuration,
                false
            );
        }
    }
    else
    {
        // No current music, just start the new one
        MusicAudioComponent->SetSound(NewMusic);
        MusicAudioComponent->FadeIn(AudioConfig.TransitionDuration, 0.7f);
    }
}