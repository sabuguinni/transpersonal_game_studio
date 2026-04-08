#include "AudioSystemArchitecture.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "AudioDevice.h"

UAudioSystemManager::UAudioSystemManager()
{
    TransitionTimer = 0.0f;
    TransitionDuration = 2.0f;
    bIsTransitioning = false;
    TargetEmotionalState = EAudioEmotionalState::Calm;
}

void UAudioSystemManager::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initializing Adaptive Audio System"));

    // Initialize audio components
    if (!MusicAudioComponent)
    {
        MusicAudioComponent = NewObject<UAudioComponent>(this);
        MusicAudioComponent->bAutoActivate = false;
    }

    if (!AmbienceAudioComponent)
    {
        AmbienceAudioComponent = NewObject<UAudioComponent>(this);
        AmbienceAudioComponent->bAutoActivate = false;
    }

    if (!StingerAudioComponent)
    {
        StingerAudioComponent = NewObject<UAudioComponent>(this);
        StingerAudioComponent->bAutoActivate = false;
    }

    // Set initial audio state
    CurrentAudioState = FAudioStateParameters();
    
    // Apply default sound mix
    ApplySoundMixForState();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Audio System Initialized Successfully"));
}

void UAudioSystemManager::UpdateAudioState(const FAudioStateParameters& NewState)
{
    FAudioStateParameters PreviousState = CurrentAudioState;
    CurrentAudioState = NewState;

    // Check if emotional state changed
    if (PreviousState.EmotionalState != NewState.EmotionalState)
    {
        TransitionToEmotionalState(NewState.EmotionalState);
    }

    // Update environment if changed
    if (PreviousState.EnvironmentType != NewState.EnvironmentType)
    {
        SetEnvironmentType(NewState.EnvironmentType);
    }

    // Update threat level if changed
    if (PreviousState.ThreatLevel != NewState.ThreatLevel)
    {
        UpdateThreatLevel(NewState.ThreatLevel);
    }

    // Update continuous parameters
    UpdateMusicLayer();
    UpdateAmbienceLayer();

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Audio State Updated - Emotional: %d, Environment: %d, Threat: %d"), 
           (int32)NewState.EmotionalState, (int32)NewState.EnvironmentType, (int32)NewState.ThreatLevel);
}

void UAudioSystemManager::TransitionToEmotionalState(EAudioEmotionalState NewState, float TransitionTime)
{
    if (CurrentAudioState.EmotionalState == NewState && !bIsTransitioning)
    {
        return; // Already in target state
    }

    TargetEmotionalState = NewState;
    TransitionDuration = TransitionTime;
    TransitionTimer = 0.0f;
    bIsTransitioning = true;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Starting transition to emotional state %d over %f seconds"), 
           (int32)NewState, TransitionTime);

    // Apply sound mix for new state
    ApplySoundMixForState();
}

void UAudioSystemManager::SetEnvironmentType(EAudioEnvironmentType NewEnvironment)
{
    CurrentAudioState.EnvironmentType = NewEnvironment;
    UpdateAmbienceLayer();

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Environment changed to %d"), (int32)NewEnvironment);
}

void UAudioSystemManager::UpdateThreatLevel(EDinosaurThreatLevel NewThreatLevel)
{
    EDinosaurThreatLevel PreviousThreat = CurrentAudioState.ThreatLevel;
    CurrentAudioState.ThreatLevel = NewThreatLevel;

    // Update stress level based on threat
    switch (NewThreatLevel)
    {
        case EDinosaurThreatLevel::None:
            CurrentAudioState.StressLevel = FMath::Max(0.0f, CurrentAudioState.StressLevel - 0.1f);
            break;
        case EDinosaurThreatLevel::Herbivore:
            CurrentAudioState.StressLevel = FMath::Clamp(CurrentAudioState.StressLevel + 0.1f, 0.0f, 0.3f);
            break;
        case EDinosaurThreatLevel::SmallPredator:
            CurrentAudioState.StressLevel = FMath::Clamp(CurrentAudioState.StressLevel + 0.3f, 0.0f, 0.6f);
            break;
        case EDinosaurThreatLevel::LargePredator:
            CurrentAudioState.StressLevel = FMath::Clamp(CurrentAudioState.StressLevel + 0.5f, 0.0f, 0.8f);
            break;
        case EDinosaurThreatLevel::ApexPredator:
            CurrentAudioState.StressLevel = 1.0f;
            break;
    }

    // Trigger emotional state change if threat escalated significantly
    if (NewThreatLevel > PreviousThreat)
    {
        if (NewThreatLevel >= EDinosaurThreatLevel::LargePredator)
        {
            TransitionToEmotionalState(EAudioEmotionalState::Fear, 1.0f);
        }
        else if (NewThreatLevel >= EDinosaurThreatLevel::SmallPredator)
        {
            TransitionToEmotionalState(EAudioEmotionalState::Tension, 1.5f);
        }
    }

    UpdateMusicLayer();

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Threat level updated to %d, Stress level: %f"), 
           (int32)NewThreatLevel, CurrentAudioState.StressLevel);
}

void UAudioSystemManager::TriggerDinosaurEvent(const FString& DinosaurType, const FVector& Location, float Intensity)
{
    // This will be expanded to trigger specific audio stingers based on dinosaur type
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Dinosaur event triggered - Type: %s, Intensity: %f"), 
           *DinosaurType, Intensity);

    // Play appropriate stinger sound
    if (StingerAudioComponent && StingerAudioComponent->Sound)
    {
        StingerAudioComponent->SetVolumeMultiplier(Intensity);
        StingerAudioComponent->Play();
    }
}

void UAudioSystemManager::PlayAdaptiveMusic()
{
    if (MusicAudioComponent)
    {
        UpdateMusicLayer();
        MusicAudioComponent->Play();
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Adaptive music started"));
    }
}

void UAudioSystemManager::StopAdaptiveMusic(float FadeOutTime)
{
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(FadeOutTime, 0.0f);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Adaptive music stopping with %f second fade"), FadeOutTime);
    }
}

void UAudioSystemManager::UpdateMusicLayer()
{
    if (!MusicAudioComponent)
        return;

    // Update music parameters based on current state
    float IntensityMultiplier = 1.0f;
    float PitchMultiplier = 1.0f;

    // Adjust based on emotional state
    switch (CurrentAudioState.EmotionalState)
    {
        case EAudioEmotionalState::Calm:
            IntensityMultiplier = 0.6f;
            PitchMultiplier = 0.95f;
            break;
        case EAudioEmotionalState::Tension:
            IntensityMultiplier = 0.8f;
            PitchMultiplier = 1.0f;
            break;
        case EAudioEmotionalState::Fear:
            IntensityMultiplier = 1.0f;
            PitchMultiplier = 1.05f;
            break;
        case EAudioEmotionalState::Panic:
            IntensityMultiplier = 1.2f;
            PitchMultiplier = 1.1f;
            break;
        case EAudioEmotionalState::Wonder:
            IntensityMultiplier = 0.7f;
            PitchMultiplier = 1.02f;
            break;
        case EAudioEmotionalState::Discovery:
            IntensityMultiplier = 0.9f;
            PitchMultiplier = 1.03f;
            break;
    }

    // Apply stress level influence
    IntensityMultiplier *= (1.0f + CurrentAudioState.StressLevel * 0.3f);
    PitchMultiplier *= (1.0f + CurrentAudioState.StressLevel * 0.05f);

    // Apply time of day influence
    float TimeIntensity = 1.0f;
    if (CurrentAudioState.TimeOfDay > 0.4f && CurrentAudioState.TimeOfDay < 0.8f) // Night time
    {
        TimeIntensity = 1.2f; // Night is more intense
        IntensityMultiplier *= TimeIntensity;
    }

    MusicAudioComponent->SetVolumeMultiplier(IntensityMultiplier);
    MusicAudioComponent->SetPitchMultiplier(PitchMultiplier);
}

void UAudioSystemManager::UpdateAmbienceLayer()
{
    if (!AmbienceAudioComponent)
        return;

    // Update ambience based on environment type
    float AmbienceVolume = 0.8f;
    float AmbiencePitch = 1.0f;

    // Adjust based on environment
    switch (CurrentAudioState.EnvironmentType)
    {
        case EAudioEnvironmentType::DenseForest:
            AmbienceVolume = 0.9f;
            break;
        case EAudioEnvironmentType::OpenPlains:
            AmbienceVolume = 0.6f;
            break;
        case EAudioEnvironmentType::RiverBank:
            AmbienceVolume = 1.0f;
            break;
        case EAudioEnvironmentType::Cave:
            AmbienceVolume = 0.4f;
            AmbiencePitch = 0.9f;
            break;
        case EAudioEnvironmentType::Clearing:
            AmbienceVolume = 0.7f;
            break;
        case EAudioEnvironmentType::Swamp:
            AmbienceVolume = 0.8f;
            AmbiencePitch = 0.95f;
            break;
    }

    // Apply isolation level influence
    AmbienceVolume *= (1.0f - CurrentAudioState.IsolationLevel * 0.3f);

    AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume);
    AmbienceAudioComponent->SetPitchMultiplier(AmbiencePitch);
}

void UAudioSystemManager::ApplySoundMixForState()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    USoundMix* TargetSoundMix = DefaultSoundMix;

    switch (CurrentAudioState.EmotionalState)
    {
        case EAudioEmotionalState::Tension:
            TargetSoundMix = TensionSoundMix;
            break;
        case EAudioEmotionalState::Fear:
            TargetSoundMix = FearSoundMix;
            break;
        case EAudioEmotionalState::Panic:
            TargetSoundMix = PanicSoundMix;
            break;
        default:
            TargetSoundMix = DefaultSoundMix;
            break;
    }

    if (TargetSoundMix)
    {
        UGameplayStatics::PushSoundMixModifier(World, TargetSoundMix);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Applied sound mix for emotional state"));
    }
}

void UAudioSystemManager::ProcessAudioTransition(float DeltaTime)
{
    if (!bIsTransitioning)
        return;

    TransitionTimer += DeltaTime;
    float TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        CurrentAudioState.EmotionalState = TargetEmotionalState;
        bIsTransitioning = false;
        TransitionTimer = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transition to emotional state %d completed"), 
               (int32)TargetEmotionalState);
    }

    // Update audio layers during transition
    UpdateMusicLayer();
    UpdateAmbienceLayer();
}