#include "AudioSystemCore.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "MetasoundParameterPack.h"

UAudioSystemCore::UAudioSystemCore()
{
    CurrentEmotionalState = EAudioEmotionalState::Calm_Exploration;
    CurrentEnvironment = EAudioEnvironmentType::Dense_Forest;
}

void UAudioSystemCore::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Initializing adaptive audio system"));
    
    CreateAudioComponents();
    ApplyEmotionalStateParameters();
    
    // Start with calm exploration state
    UpdateEmotionalState(EAudioEmotionalState::Calm_Exploration, 0.0f);
}

void UAudioSystemCore::CreateAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Create music audio component
    MusicAudioComponent = NewObject<UAudioComponent>(this);
    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        MusicAudioComponent->SetSound(AdaptiveMusicMetaSound);
        MusicAudioComponent->bAutoActivate = true;
        MusicAudioComponent->bIsUISound = false;
    }

    // Create ambience audio component
    AmbienceAudioComponent = NewObject<UAudioComponent>(this);
    if (AmbienceAudioComponent && EnvironmentAmbienceMetaSound)
    {
        AmbienceAudioComponent->SetSound(EnvironmentAmbienceMetaSound);
        AmbienceAudioComponent->bAutoActivate = true;
        AmbienceAudioComponent->bIsUISound = false;
    }

    // Create proximity audio component
    ProximityAudioComponent = NewObject<UAudioComponent>(this);
    if (ProximityAudioComponent && DinosaurProximityMetaSound)
    {
        ProximityAudioComponent->SetSound(DinosaurProximityMetaSound);
        ProximityAudioComponent->bAutoActivate = false;
        ProximityAudioComponent->bIsUISound = false;
    }
}

void UAudioSystemCore::UpdateEmotionalState(EAudioEmotionalState NewState, float TransitionTime)
{
    if (CurrentEmotionalState == NewState) return;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Transitioning from %d to %d over %.2f seconds"), 
           (int32)CurrentEmotionalState, (int32)NewState, TransitionTime);

    EAudioEmotionalState PreviousState = CurrentEmotionalState;
    CurrentEmotionalState = NewState;

    if (TransitionTime > 0.0f)
    {
        HandleStateTransition(NewState, TransitionTime);
    }
    else
    {
        ApplyEmotionalStateParameters();
    }
}

void UAudioSystemCore::HandleStateTransition(EAudioEmotionalState NewState, float Duration)
{
    // Find transition curve if available
    UCurveFloat* TransitionCurve = nullptr;
    for (const FAudioStateTransition& Transition : StateTransitions)
    {
        if (Transition.ToState == NewState)
        {
            TransitionCurve = Transition.TransitionCurve;
            break;
        }
    }

    // Apply immediate parameter changes for critical states
    if (NewState == EAudioEmotionalState::Terror_Active_Threat)
    {
        SetMusicIntensity(1.0f);
        TriggerStingerSound("ThreatStinger");
    }
    else if (NewState == EAudioEmotionalState::Relief_Safety_Found)
    {
        TriggerStingerSound("ReliefStinger");
    }

    ApplyEmotionalStateParameters();
}

void UAudioSystemCore::ApplyEmotionalStateParameters()
{
    if (!MusicAudioComponent) return;

    // Set MetaSound parameters based on emotional state
    switch (CurrentEmotionalState)
    {
        case EAudioEmotionalState::Calm_Exploration:
            SetMusicIntensity(0.2f);
            SetAmbienceLayer(0, 0.8f); // Nature sounds
            SetAmbienceLayer(1, 0.3f); // Distant animal calls
            break;

        case EAudioEmotionalState::Tense_Awareness:
            SetMusicIntensity(0.4f);
            SetAmbienceLayer(0, 0.6f);
            SetAmbienceLayer(1, 0.5f); // More frequent animal sounds
            break;

        case EAudioEmotionalState::Fear_Imminent_Danger:
            SetMusicIntensity(0.7f);
            SetAmbienceLayer(0, 0.4f);
            SetAmbienceLayer(1, 0.7f); // Threatening sounds
            break;

        case EAudioEmotionalState::Terror_Active_Threat:
            SetMusicIntensity(1.0f);
            SetAmbienceLayer(0, 0.2f); // Reduced nature
            SetAmbienceLayer(1, 0.9f); // Maximum threat audio
            break;

        case EAudioEmotionalState::Relief_Safety_Found:
            SetMusicIntensity(0.3f);
            SetAmbienceLayer(0, 0.9f); // Peaceful nature
            SetAmbienceLayer(1, 0.1f); // Minimal threats
            break;

        case EAudioEmotionalState::Wonder_Discovery:
            SetMusicIntensity(0.5f);
            SetAmbienceLayer(0, 0.7f);
            SetAmbienceLayer(1, 0.4f);
            break;

        case EAudioEmotionalState::Melancholy_Solitude:
            SetMusicIntensity(0.3f);
            SetAmbienceLayer(0, 0.6f);
            SetAmbienceLayer(1, 0.2f);
            break;
    }
}

void UAudioSystemCore::SetEnvironmentType(EAudioEnvironmentType Environment)
{
    if (CurrentEnvironment == Environment) return;

    CurrentEnvironment = Environment;
    
    // Update ambience MetaSound parameters based on environment
    if (AmbienceAudioComponent)
    {
        // Environment-specific parameter setting would go here
        // This would control which ambience layers are active
    }
}

void UAudioSystemCore::TriggerDinosaurProximityAudio(float Distance, float DinosaurSize)
{
    if (!ProximityAudioComponent) return;

    // Calculate proximity intensity based on distance and size
    float ProximityIntensity = FMath::Clamp((1.0f - (Distance / 1000.0f)) * DinosaurSize, 0.0f, 1.0f);
    
    if (ProximityIntensity > 0.1f)
    {
        // Set MetaSound parameters for proximity audio
        // This would control heartbeat intensity, breathing, etc.
        if (!ProximityAudioComponent->IsPlaying())
        {
            ProximityAudioComponent->Play();
        }
    }
    else
    {
        if (ProximityAudioComponent->IsPlaying())
        {
            ProximityAudioComponent->FadeOut(1.0f, 0.0f);
        }
    }
}

void UAudioSystemCore::SetMusicIntensity(float Intensity)
{
    if (MusicAudioComponent)
    {
        // Set MetaSound parameter for music intensity
        MusicAudioComponent->SetFloatParameter(FName("MusicIntensity"), Intensity);
    }
}

void UAudioSystemCore::SetAmbienceLayer(int32 LayerIndex, float Volume)
{
    if (AmbienceAudioComponent)
    {
        FString ParameterName = FString::Printf(TEXT("Layer%d_Volume"), LayerIndex);
        AmbienceAudioComponent->SetFloatParameter(FName(*ParameterName), Volume);
    }
}

void UAudioSystemCore::TriggerStingerSound(const FString& StingerType)
{
    if (MusicAudioComponent)
    {
        // Trigger one-shot stinger sounds through MetaSound
        MusicAudioComponent->SetTriggerParameter(FName(*StingerType));
    }
}