#include "AudioManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AAudioManager::AAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Audio Components
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    RootComponent = MusicAudioComponent;
    MusicAudioComponent->bAutoActivate = false;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    // Initialize default values
    StateTransitionTime = 2.0f;
    IntensityChangeRate = 0.5f;
    TransitionProgress = 1.0f;
    bIsTransitioning = false;

    // Initialize target state to current state
    TargetMusicState = CurrentMusicState;
}

void AAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Start adaptive music system
    if (AdaptiveMusicMetaSound)
    {
        MusicAudioComponent->SetSound(AdaptiveMusicMetaSound);
        MusicAudioComponent->Play();
    }

    // Start ambient audio
    UpdateAmbientAudio();
}

void AAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle music state transitions
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime / StateTransitionTime;
        
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
            CurrentMusicState = TargetMusicState;
        }
        else
        {
            // Interpolate between current and target states
            CurrentMusicState.Intensity = FMath::Lerp(CurrentMusicState.Intensity, TargetMusicState.Intensity, TransitionProgress);
            CurrentMusicState.Tension = FMath::Lerp(CurrentMusicState.Tension, TargetMusicState.Tension, TransitionProgress);
        }

        UpdateAdaptiveMusic();
    }
}

void AAudioManager::SetAudioState(EAudioState NewState, float TransitionTime)
{
    if (CurrentMusicState.AudioState != NewState)
    {
        TargetMusicState.AudioState = NewState;
        
        // Set appropriate intensity and tension based on state
        switch (NewState)
        {
            case EAudioState::Exploration:
                TargetMusicState.Intensity = 0.3f;
                TargetMusicState.Tension = 0.2f;
                break;
            case EAudioState::Tension:
                TargetMusicState.Intensity = 0.6f;
                TargetMusicState.Tension = 0.7f;
                break;
            case EAudioState::Danger:
                TargetMusicState.Intensity = 0.8f;
                TargetMusicState.Tension = 0.9f;
                break;
            case EAudioState::Combat:
                TargetMusicState.Intensity = 1.0f;
                TargetMusicState.Tension = 1.0f;
                break;
            case EAudioState::Discovery:
                TargetMusicState.Intensity = 0.7f;
                TargetMusicState.Tension = 0.1f;
                break;
            case EAudioState::Peaceful:
                TargetMusicState.Intensity = 0.2f;
                TargetMusicState.Tension = 0.0f;
                break;
            case EAudioState::Domestication:
                TargetMusicState.Intensity = 0.4f;
                TargetMusicState.Tension = 0.3f;
                break;
            case EAudioState::Death:
                TargetMusicState.Intensity = 0.9f;
                TargetMusicState.Tension = 0.8f;
                break;
        }

        TransitionMusicState(TargetMusicState, TransitionTime);
    }
}

void AAudioManager::SetMusicIntensity(float NewIntensity, float TransitionTime)
{
    NewIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    TargetMusicState.Intensity = NewIntensity;
    TransitionMusicState(TargetMusicState, TransitionTime);
}

void AAudioManager::SetMusicTension(float NewTension, float TransitionTime)
{
    NewTension = FMath::Clamp(NewTension, 0.0f, 1.0f);
    TargetMusicState.Tension = NewTension;
    TransitionMusicState(TargetMusicState, TransitionTime);
}

void AAudioManager::SetEnvironmentType(EEnvironmentType NewEnvironment)
{
    if (CurrentMusicState.Environment != NewEnvironment)
    {
        CurrentMusicState.Environment = NewEnvironment;
        TargetMusicState.Environment = NewEnvironment;
        UpdateAmbientAudio();
        UpdateAdaptiveMusic();
    }
}

void AAudioManager::OnDinosaurDetected(bool bDetected, float Distance)
{
    CurrentMusicState.bIsNearDinosaurs = bDetected;
    TargetMusicState.bIsNearDinosaurs = bDetected;

    if (bDetected)
    {
        // Increase tension based on proximity
        float ProximityTension = FMath::Clamp(1.0f - (Distance / 1000.0f), 0.0f, 1.0f);
        SetMusicTension(FMath::Max(CurrentMusicState.Tension, ProximityTension), 1.0f);
    }
    else
    {
        // Gradually reduce tension when no dinosaurs nearby
        SetMusicTension(0.2f, 3.0f);
    }
}

void AAudioManager::OnPlayerHiding(bool bHiding)
{
    CurrentMusicState.bIsHiding = bHiding;
    TargetMusicState.bIsHiding = bHiding;

    if (bHiding)
    {
        // Reduce intensity when hiding
        SetMusicIntensity(FMath::Max(CurrentMusicState.Intensity * 0.5f, 0.1f), 0.5f);
    }
    else
    {
        // Restore intensity when not hiding
        SetMusicIntensity(0.6f, 1.0f);
    }
}

void AAudioManager::OnTimeOfDayChanged(float NewTimeOfDay)
{
    CurrentMusicState.TimeOfDay = NewTimeOfDay;
    TargetMusicState.TimeOfDay = NewTimeOfDay;
    UpdateAdaptiveMusic();
}

void AAudioManager::UpdateAmbientAudio()
{
    if (EnvironmentAmbientSounds.Contains(CurrentMusicState.Environment))
    {
        UMetaSoundSource* AmbientSound = EnvironmentAmbientSounds[CurrentMusicState.Environment];
        if (AmbientSound && AmbientAudioComponent)
        {
            AmbientAudioComponent->SetSound(AmbientSound);
            if (!AmbientAudioComponent->IsPlaying())
            {
                AmbientAudioComponent->Play();
            }
        }
    }
}

void AAudioManager::UpdateAdaptiveMusic()
{
    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        // Update MetaSound parameters
        MusicAudioComponent->SetFloatParameter(FName("Intensity"), CurrentMusicState.Intensity);
        MusicAudioComponent->SetFloatParameter(FName("Tension"), CurrentMusicState.Tension);
        MusicAudioComponent->SetFloatParameter(FName("TimeOfDay"), CurrentMusicState.TimeOfDay);
        MusicAudioComponent->SetIntParameter(FName("AudioState"), (int32)CurrentMusicState.AudioState);
        MusicAudioComponent->SetIntParameter(FName("Environment"), (int32)CurrentMusicState.Environment);
        MusicAudioComponent->SetBoolParameter(FName("IsNearDinosaurs"), CurrentMusicState.bIsNearDinosaurs);
        MusicAudioComponent->SetBoolParameter(FName("IsHiding"), CurrentMusicState.bIsHiding);
    }
}

void AAudioManager::TransitionMusicState(const FAdaptiveMusicState& TargetState, float TransitionTime)
{
    TargetMusicState = TargetState;
    StateTransitionTime = FMath::Max(TransitionTime, 0.1f);
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
}