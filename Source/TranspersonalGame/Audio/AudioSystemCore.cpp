#include "AudioSystemCore.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "MetasoundParameterPack.h"
#include "TimerManager.h"

// Parameter name constants
const FName UAudioSystemCore::PARAM_EMOTIONAL_STATE = "EmotionalState";
const FName UAudioSystemCore::PARAM_THREAT_LEVEL = "ThreatLevel";
const FName UAudioSystemCore::PARAM_TIME_OF_DAY = "TimeOfDay";
const FName UAudioSystemCore::PARAM_ENVIRONMENT_TYPE = "EnvironmentType";
const FName UAudioSystemCore::PARAM_STEALTH_LEVEL = "StealthLevel";
const FName UAudioSystemCore::PARAM_DINOSAUR_PROXIMITY = "DinosaurProximity";

void UAudioSystemCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default audio state
    CurrentAudioState.EmotionalState = EEmotionalState::Calm;
    CurrentAudioState.EnvironmentType = EEnvironmentType::DenseForest;
    CurrentAudioState.TimeOfDay = ETimeOfDay::Morning;
    CurrentAudioState.ThreatLevel = 0.0f;
    CurrentAudioState.PlayerStealthLevel = 1.0f;
    CurrentAudioState.bIsInPlayerBase = false;
    CurrentAudioState.NearbyDinosaurCount = 0;
    CurrentAudioState.DistanceToNearestPredator = 1000.0f;

    TargetAudioState = CurrentAudioState;

    UE_LOG(LogAudio, Log, TEXT("Audio System Core initialized"));
}

void UAudioSystemCore::Deinitialize()
{
    if (MusicComponent && IsValid(MusicComponent))
    {
        MusicComponent->Stop();
    }

    if (AmbientComponent && IsValid(AmbientComponent))
    {
        AmbientComponent->Stop();
    }

    for (auto* Component : LayeredAmbientComponents)
    {
        if (Component && IsValid(Component))
        {
            Component->Stop();
        }
    }

    Super::Deinitialize();
}

void UAudioSystemCore::InitializeAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Create main music component
    if (!MusicComponent)
    {
        MusicComponent = UGameplayStatics::CreateSound2D(World, nullptr, 1.0f, 1.0f, 0.0f, nullptr, true, false);
        if (MusicComponent)
        {
            MusicComponent->bAutoDestroy = false;
        }
    }

    // Create ambient component
    if (!AmbientComponent)
    {
        AmbientComponent = UGameplayStatics::CreateSound2D(World, nullptr, 1.0f, 1.0f, 0.0f, nullptr, true, false);
        if (AmbientComponent)
        {
            AmbientComponent->bAutoDestroy = false;
        }
    }

    // Create layered ambient components (for different environment layers)
    LayeredAmbientComponents.Empty();
    for (int32 i = 0; i < 4; i++) // Wind, Animals, Vegetation, Geological
    {
        UAudioComponent* LayerComponent = UGameplayStatics::CreateSound2D(World, nullptr, 1.0f, 1.0f, 0.0f, nullptr, true, false);
        if (LayerComponent)
        {
            LayerComponent->bAutoDestroy = false;
            LayeredAmbientComponents.Add(LayerComponent);
        }
    }
}

void UAudioSystemCore::UpdateAudioState(const FAudioStateParameters& NewState)
{
    TargetAudioState = NewState;
    
    // Start smooth transition
    TransitionProgress = 0.0f;
    TransitionDuration = 2.0f;

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(StateTransitionTimer, 
            FTimerDelegate::CreateUObject(this, &UAudioSystemCore::HandleStateTransition, World->GetDeltaSeconds()),
            0.016f, true); // 60fps updates
    }
}

void UAudioSystemCore::TransitionToEmotionalState(EEmotionalState NewState, float TransitionTime)
{
    TargetAudioState.EmotionalState = NewState;
    TransitionDuration = TransitionTime;
    TransitionProgress = 0.0f;

    // Log emotional state changes for debugging
    UE_LOG(LogAudio, Log, TEXT("Transitioning to emotional state: %d over %.2f seconds"), 
           (int32)NewState, TransitionTime);

    UpdateMetaSoundParameters();
}

void UAudioSystemCore::SetEnvironmentType(EEnvironmentType NewEnvironment, float TransitionTime)
{
    TargetAudioState.EnvironmentType = NewEnvironment;
    TransitionDuration = TransitionTime;
    TransitionProgress = 0.0f;

    UE_LOG(LogAudio, Log, TEXT("Transitioning to environment: %d over %.2f seconds"), 
           (int32)NewEnvironment, TransitionTime);

    UpdateAmbientLayers();
}

void UAudioSystemCore::StartAdaptiveMusic()
{
    if (!MusicComponent)
    {
        InitializeAudioComponents();
    }

    if (MusicComponent && AdaptiveMusicMetaSound)
    {
        MusicComponent->SetSound(AdaptiveMusicMetaSound);
        MusicComponent->Play();
        
        UpdateMetaSoundParameters();
        
        UE_LOG(LogAudio, Log, TEXT("Started adaptive music system"));
    }
}

void UAudioSystemCore::StopAdaptiveMusic(float FadeOutTime)
{
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        MusicComponent->FadeOut(FadeOutTime, 0.0f);
        UE_LOG(LogAudio, Log, TEXT("Stopping adaptive music with %.2f second fade"), FadeOutTime);
    }
}

void UAudioSystemCore::UpdateAmbientLayers()
{
    if (LayeredAmbientComponents.Num() == 0)
    {
        InitializeAudioComponents();
    }

    // Update each ambient layer based on current environment and state
    if (AmbientLayersMetaSound && LayeredAmbientComponents.Num() > 0)
    {
        for (int32 i = 0; i < LayeredAmbientComponents.Num(); i++)
        {
            if (LayeredAmbientComponents[i])
            {
                LayeredAmbientComponents[i]->SetSound(AmbientLayersMetaSound);
                if (!LayeredAmbientComponents[i]->IsPlaying())
                {
                    LayeredAmbientComponents[i]->Play();
                }
            }
        }
        
        UpdateMetaSoundParameters();
    }
}

void UAudioSystemCore::TriggerDinosaurEncounter(class ADinosaur* Dinosaur, float Distance)
{
    if (!Dinosaur) return;

    // Increase threat level based on dinosaur type and distance
    float ThreatIncrease = FMath::Clamp(1.0f - (Distance / 1000.0f), 0.1f, 1.0f);
    
    TargetAudioState.ThreatLevel = FMath::Clamp(CurrentAudioState.ThreatLevel + ThreatIncrease, 0.0f, 1.0f);
    TargetAudioState.DistanceToNearestPredator = Distance;
    TargetAudioState.NearbyDinosaurCount++;

    // Trigger immediate emotional state change for close encounters
    if (Distance < 200.0f)
    {
        TransitionToEmotionalState(EEmotionalState::Danger, 0.5f);
    }
    else if (Distance < 500.0f)
    {
        TransitionToEmotionalState(EEmotionalState::Tension, 1.0f);
    }

    UE_LOG(LogAudio, Log, TEXT("Dinosaur encounter triggered - Distance: %.2f, Threat Level: %.2f"), 
           Distance, TargetAudioState.ThreatLevel);
}

void UAudioSystemCore::TriggerDomesticationMoment(class ADinosaur* Dinosaur, bool bSuccessful)
{
    if (bSuccessful)
    {
        TransitionToEmotionalState(EEmotionalState::Relief, 2.0f);
        // Reduce threat level as player gains an ally
        TargetAudioState.ThreatLevel = FMath::Max(0.0f, CurrentAudioState.ThreatLevel - 0.3f);
    }
    else
    {
        TransitionToEmotionalState(EEmotionalState::Tension, 1.0f);
    }

    UE_LOG(LogAudio, Log, TEXT("Domestication moment - Success: %s"), bSuccessful ? TEXT("true") : TEXT("false"));
}

void UAudioSystemCore::TriggerDiscoveryMoment(const FString& DiscoveryType)
{
    TransitionToEmotionalState(EEmotionalState::Discovery, 1.5f);
    
    // Temporarily reduce threat as player focuses on discovery
    TargetAudioState.ThreatLevel = FMath::Max(0.0f, CurrentAudioState.ThreatLevel - 0.2f);

    UE_LOG(LogAudio, Log, TEXT("Discovery moment triggered: %s"), *DiscoveryType);
}

void UAudioSystemCore::UpdateMetaSoundParameters()
{
    // Update music component parameters
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        MusicComponent->SetFloatParameter(PARAM_EMOTIONAL_STATE, (float)CurrentAudioState.EmotionalState);
        MusicComponent->SetFloatParameter(PARAM_THREAT_LEVEL, CurrentAudioState.ThreatLevel);
        MusicComponent->SetFloatParameter(PARAM_TIME_OF_DAY, (float)CurrentAudioState.TimeOfDay);
        MusicComponent->SetFloatParameter(PARAM_ENVIRONMENT_TYPE, (float)CurrentAudioState.EnvironmentType);
        MusicComponent->SetFloatParameter(PARAM_STEALTH_LEVEL, CurrentAudioState.PlayerStealthLevel);
        MusicComponent->SetFloatParameter(PARAM_DINOSAUR_PROXIMITY, 
            FMath::Clamp(1000.0f / FMath::Max(CurrentAudioState.DistanceToNearestPredator, 50.0f), 0.0f, 1.0f));
    }

    // Update ambient component parameters
    for (UAudioComponent* Component : LayeredAmbientComponents)
    {
        if (Component && Component->IsPlaying())
        {
            Component->SetFloatParameter(PARAM_EMOTIONAL_STATE, (float)CurrentAudioState.EmotionalState);
            Component->SetFloatParameter(PARAM_THREAT_LEVEL, CurrentAudioState.ThreatLevel);
            Component->SetFloatParameter(PARAM_TIME_OF_DAY, (float)CurrentAudioState.TimeOfDay);
            Component->SetFloatParameter(PARAM_ENVIRONMENT_TYPE, (float)CurrentAudioState.EnvironmentType);
            Component->SetFloatParameter(PARAM_STEALTH_LEVEL, CurrentAudioState.PlayerStealthLevel);
        }
    }
}

void UAudioSystemCore::HandleStateTransition(float DeltaTime)
{
    if (TransitionProgress >= 1.0f)
    {
        CurrentAudioState = TargetAudioState;
        UWorld* World = GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(StateTransitionTimer);
        }
        return;
    }

    TransitionProgress += DeltaTime / TransitionDuration;
    TransitionProgress = FMath::Clamp(TransitionProgress, 0.0f, 1.0f);

    // Smooth interpolation between current and target states
    float Alpha = FMath::SmoothStep(0.0f, 1.0f, TransitionProgress);

    CurrentAudioState.ThreatLevel = FMath::Lerp(CurrentAudioState.ThreatLevel, TargetAudioState.ThreatLevel, Alpha);
    CurrentAudioState.PlayerStealthLevel = FMath::Lerp(CurrentAudioState.PlayerStealthLevel, TargetAudioState.PlayerStealthLevel, Alpha);
    CurrentAudioState.DistanceToNearestPredator = FMath::Lerp(CurrentAudioState.DistanceToNearestPredator, TargetAudioState.DistanceToNearestPredator, Alpha);

    // Update discrete states when transition is more than halfway
    if (Alpha > 0.5f)
    {
        CurrentAudioState.EmotionalState = TargetAudioState.EmotionalState;
        CurrentAudioState.EnvironmentType = TargetAudioState.EnvironmentType;
        CurrentAudioState.TimeOfDay = TargetAudioState.TimeOfDay;
        CurrentAudioState.bIsInPlayerBase = TargetAudioState.bIsInPlayerBase;
        CurrentAudioState.NearbyDinosaurCount = TargetAudioState.NearbyDinosaurCount;
    }

    UpdateMetaSoundParameters();
}