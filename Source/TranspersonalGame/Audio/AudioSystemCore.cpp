#include "AudioSystemCore.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"
#include "MetasoundParameterTransmitter.h"

UAudioSystemCore::UAudioSystemCore()
{
    // Initialize default audio state
    CurrentAudioState.EmotionalState = EAudioEmotionalState::Calm;
    CurrentAudioState.EnvironmentType = EEnvironmentType::DenseForest;
    CurrentAudioState.TensionLevel = 0.0f;
    CurrentAudioState.ProximityToDanger = 0.0f;
    CurrentAudioState.bIsNightTime = false;
    CurrentAudioState.WeatherIntensity = 0.0f;

    TargetEmotionalState = EAudioEmotionalState::Calm;
}

void UAudioSystemCore::InitializeAudioSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogAudio, Warning, TEXT("AudioSystemCore: No valid world found during initialization"));
        return;
    }

    // Create audio components
    MusicAudioComponent = NewObject<UAudioComponent>(this);
    AmbienceAudioComponent = NewObject<UAudioComponent>(this);
    PlayerAudioComponent = NewObject<UAudioComponent>(this);

    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetSound(AdaptiveMusicMetaSound);
        MusicAudioComponent->SetVolumeMultiplier(0.7f);
        MusicAudioComponent->bAutoActivate = false;
    }

    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetSound(EnvironmentalAmbienceMetaSound);
        AmbienceAudioComponent->SetVolumeMultiplier(0.8f);
        AmbienceAudioComponent->bAutoActivate = true;
        AmbienceAudioComponent->Play();
    }

    if (PlayerAudioComponent)
    {
        PlayerAudioComponent->SetVolumeMultiplier(0.6f);
        PlayerAudioComponent->bAutoActivate = false;
    }

    UE_LOG(LogAudio, Log, TEXT("AudioSystemCore: Audio system initialized successfully"));
}

void UAudioSystemCore::UpdateAudioState(const FAudioStateData& NewState)
{
    FAudioStateData PreviousState = CurrentAudioState;
    CurrentAudioState = NewState;

    // Calculate tension level based on multiple factors
    CalculateTensionLevel();

    // Update MetaSound parameters
    UpdateMetaSoundParameters();

    // Handle emotional state changes
    if (PreviousState.EmotionalState != CurrentAudioState.EmotionalState)
    {
        TransitionToEmotionalState(CurrentAudioState.EmotionalState, 1.5f);
    }

    // Update environmental layers
    ProcessEnvironmentalLayers();

    UE_LOG(LogAudio, VeryVerbose, TEXT("AudioSystemCore: Audio state updated - Tension: %f, Emotional State: %d"), 
           CurrentAudioState.TensionLevel, (int32)CurrentAudioState.EmotionalState);
}

void UAudioSystemCore::TransitionToEmotionalState(EAudioEmotionalState NewState, float TransitionTime)
{
    if (TargetEmotionalState == NewState)
    {
        return; // Already transitioning to this state
    }

    TargetEmotionalState = NewState;
    TransitionDuration = TransitionTime;
    StateTransitionProgress = 0.0f;

    UE_LOG(LogAudio, Log, TEXT("AudioSystemCore: Starting emotional state transition to %d over %f seconds"), 
           (int32)NewState, TransitionTime);
}

void UAudioSystemCore::SetEnvironmentType(EEnvironmentType NewEnvironment)
{
    if (CurrentAudioState.EnvironmentType != NewEnvironment)
    {
        CurrentAudioState.EnvironmentType = NewEnvironment;
        ProcessEnvironmentalLayers();
        
        UE_LOG(LogAudio, Log, TEXT("AudioSystemCore: Environment changed to %d"), (int32)NewEnvironment);
    }
}

void UAudioSystemCore::UpdateAmbientLayers(float DeltaTime)
{
    // Handle emotional state transitions
    if (StateTransitionProgress < 1.0f)
    {
        HandleEmotionalStateTransition(DeltaTime);
    }

    // Update environmental processing
    ProcessEnvironmentalLayers();
}

void UAudioSystemCore::PlayDinosaurCall(const FVector& Location, const FString& DinosaurType, float Intensity)
{
    if (!DinosaurAudioMetaSound)
    {
        UE_LOG(LogAudio, Warning, TEXT("AudioSystemCore: DinosaurAudioMetaSound not set"));
        return;
    }

    // Create temporary audio component for dinosaur call
    UAudioComponent* DinosaurCallComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(), 
        DinosaurAudioMetaSound, 
        Location,
        FRotator::ZeroRotator,
        Intensity
    );

    if (DinosaurCallComponent)
    {
        // Set MetaSound parameters for dinosaur type and intensity
        // This would be implemented with specific MetaSound parameter names
        UE_LOG(LogAudio, Log, TEXT("AudioSystemCore: Playing dinosaur call for %s at intensity %f"), 
               *DinosaurType, Intensity);
    }
}

void UAudioSystemCore::PlayDinosaurMovement(const FVector& Location, const FString& DinosaurType, float Speed)
{
    // Similar implementation to PlayDinosaurCall but for movement sounds
    UE_LOG(LogAudio, VeryVerbose, TEXT("AudioSystemCore: Playing dinosaur movement for %s at speed %f"), 
           *DinosaurType, Speed);
}

void UAudioSystemCore::PlayPlayerHeartbeat(float Intensity)
{
    if (PlayerAudioComponent && Intensity > 0.1f)
    {
        // Set heartbeat parameters and play
        PlayerAudioComponent->SetVolumeMultiplier(FMath::Clamp(Intensity, 0.0f, 1.0f));
        
        if (!PlayerAudioComponent->IsPlaying())
        {
            PlayerAudioComponent->Play();
        }
        
        UE_LOG(LogAudio, VeryVerbose, TEXT("AudioSystemCore: Player heartbeat intensity: %f"), Intensity);
    }
}

void UAudioSystemCore::PlayPlayerBreathing(float Intensity)
{
    // Similar to heartbeat but for breathing sounds
    UE_LOG(LogAudio, VeryVerbose, TEXT("AudioSystemCore: Player breathing intensity: %f"), Intensity);
}

void UAudioSystemCore::StartAdaptiveMusic()
{
    if (MusicAudioComponent && !MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeIn(2.0f);
        UE_LOG(LogAudio, Log, TEXT("AudioSystemCore: Adaptive music started"));
    }
}

void UAudioSystemCore::StopAdaptiveMusic(float FadeTime)
{
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(FadeTime, 0.0f);
        UE_LOG(LogAudio, Log, TEXT("AudioSystemCore: Adaptive music stopping with fade time %f"), FadeTime);
    }
}

void UAudioSystemCore::UpdateMetaSoundParameters()
{
    // Update MetaSound parameters based on current audio state
    // This would use the MetaSound parameter system to drive the adaptive audio

    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        // Example parameter updates (would be implemented with actual MetaSound parameter names)
        // MusicAudioComponent->SetFloatParameter(TEXT("TensionLevel"), CurrentAudioState.TensionLevel);
        // MusicAudioComponent->SetFloatParameter(TEXT("ProximityToDanger"), CurrentAudioState.ProximityToDanger);
        // MusicAudioComponent->SetBoolParameter(TEXT("IsNightTime"), CurrentAudioState.bIsNightTime);
    }

    if (AmbienceAudioComponent && EnvironmentalAmbienceMetaSound)
    {
        // Update environmental parameters
        // AmbienceAudioComponent->SetFloatParameter(TEXT("WeatherIntensity"), CurrentAudioState.WeatherIntensity);
        // AmbienceAudioComponent->SetIntParameter(TEXT("EnvironmentType"), (int32)CurrentAudioState.EnvironmentType);
    }
}

void UAudioSystemCore::CalculateTensionLevel()
{
    float BaseTension = 0.0f;

    // Base tension from emotional state
    switch (CurrentAudioState.EmotionalState)
    {
        case EAudioEmotionalState::Calm:
            BaseTension = 0.1f;
            break;
        case EAudioEmotionalState::Tension:
            BaseTension = 0.4f;
            break;
        case EAudioEmotionalState::Danger:
            BaseTension = 0.7f;
            break;
        case EAudioEmotionalState::Terror:
            BaseTension = 1.0f;
            break;
        case EAudioEmotionalState::Relief:
            BaseTension = 0.2f;
            break;
        case EAudioEmotionalState::Wonder:
            BaseTension = 0.3f;
            break;
    }

    // Modify based on proximity to danger
    float ProximityMultiplier = 1.0f + (CurrentAudioState.ProximityToDanger * 0.5f);
    
    // Night time increases base tension
    float NightMultiplier = CurrentAudioState.bIsNightTime ? 1.3f : 1.0f;
    
    // Weather can add tension
    float WeatherAddition = CurrentAudioState.WeatherIntensity * 0.2f;

    CurrentAudioState.TensionLevel = FMath::Clamp(
        (BaseTension * ProximityMultiplier * NightMultiplier) + WeatherAddition, 
        0.0f, 
        1.0f
    );
}

void UAudioSystemCore::ProcessEnvironmentalLayers()
{
    // Process different environmental audio layers based on current environment type
    // This would control various ambient sound layers, reverb settings, etc.
    
    switch (CurrentAudioState.EnvironmentType)
    {
        case EEnvironmentType::DenseForest:
            // Dense forest: muffled sounds, close reverb, bird calls, rustling
            break;
        case EEnvironmentType::OpenPlains:
            // Open plains: wind, distant sounds, echo
            break;
        case EEnvironmentType::RiverArea:
            // River: water sounds, different reverb characteristics
            break;
        case EEnvironmentType::CaveSystem:
            // Caves: heavy reverb, dripping, echo
            break;
        case EEnvironmentType::DinosaurNest:
            // Nest area: increased tension, specific dinosaur ambience
            break;
        case EEnvironmentType::PlayerBase:
            // Player base: safer feeling, reduced tension
            break;
    }
}

void UAudioSystemCore::HandleEmotionalStateTransition(float DeltaTime)
{
    StateTransitionProgress += DeltaTime / TransitionDuration;
    StateTransitionProgress = FMath::Clamp(StateTransitionProgress, 0.0f, 1.0f);

    if (StateTransitionProgress >= 1.0f)
    {
        // Transition complete
        CurrentAudioState.EmotionalState = TargetEmotionalState;
        UE_LOG(LogAudio, Log, TEXT("AudioSystemCore: Emotional state transition completed to %d"), 
               (int32)TargetEmotionalState);
    }

    // Update transition parameters in MetaSounds
    UpdateMetaSoundParameters();
}