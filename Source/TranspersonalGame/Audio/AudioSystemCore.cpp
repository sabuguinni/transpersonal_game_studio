#include "AudioSystemCore.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "AudioParameterControllerInterface.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UAudioSystemCore::UAudioSystemCore()
{
    // Initialize default audio state
    CurrentAudioState.CurrentEmotion = EEmotionalState::Calm;
    CurrentAudioState.Environment = EEnvironmentType::DenseForest;
    CurrentAudioState.ThreatLevel = EDinosaurThreatLevel::None;
    CurrentAudioState.TimeOfDay = 12.0f;
    CurrentAudioState.WeatherIntensity = 0.0f;
    CurrentAudioState.bIsPlayerHidden = false;
    CurrentAudioState.PlayerHeartRate = 60.0f;
}

void UAudioSystemCore::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemCore: Initializing Jurassic Survival Audio System"));

    // Create audio components if they don't exist
    if (!MusicAudioComponent)
    {
        MusicAudioComponent = NewObject<UAudioComponent>(this);
        MusicAudioComponent->bAutoActivate = false;
        MusicAudioComponent->SetVolumeMultiplier(0.7f);
    }

    if (!AmbienceAudioComponent)
    {
        AmbienceAudioComponent = NewObject<UAudioComponent>(this);
        AmbienceAudioComponent->bAutoActivate = false;
        AmbienceAudioComponent->SetVolumeMultiplier(0.8f);
    }

    // Start with default ambience
    UpdateEnvironmentAmbience(CurrentAudioState.Environment);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemCore: Audio system initialized successfully"));
}

void UAudioSystemCore::UpdateAudioState(const FAudioStateData& NewState)
{
    EEmotionalState PreviousEmotion = CurrentAudioState.CurrentEmotion;
    EEnvironmentType PreviousEnvironment = CurrentAudioState.Environment;
    
    CurrentAudioState = NewState;

    // Trigger transitions if needed
    if (PreviousEmotion != NewState.CurrentEmotion)
    {
        TransitionToEmotionalState(NewState.CurrentEmotion);
    }

    if (PreviousEnvironment != NewState.Environment)
    {
        UpdateEnvironmentAmbience(NewState.Environment);
    }

    // Update all audio parameters
    UpdateMusicParameters();
    UpdateAmbienceParameters();
}

void UAudioSystemCore::TriggerDinosaurSound(FVector Location, FString DinosaurSpecies, float Distance)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemCore: Dinosaur sound triggered - Species: %s, Distance: %f"), 
           *DinosaurSpecies, Distance);

    // Calculate threat level based on species and distance
    EDinosaurThreatLevel NewThreatLevel = EDinosaurThreatLevel::None;
    
    if (DinosaurSpecies.Contains(TEXT("TRex")) || DinosaurSpecies.Contains(TEXT("Spinosaurus")))
    {
        NewThreatLevel = EDinosaurThreatLevel::ApexPredator;
    }
    else if (DinosaurSpecies.Contains(TEXT("Raptor")) || DinosaurSpecies.Contains(TEXT("Carnotaurus")))
    {
        NewThreatLevel = EDinosaurThreatLevel::LargePredator;
    }
    else if (DinosaurSpecies.Contains(TEXT("Compy")) || DinosaurSpecies.Contains(TEXT("Dilo")))
    {
        NewThreatLevel = EDinosaurThreatLevel::SmallPredator;
    }
    else
    {
        NewThreatLevel = EDinosaurThreatLevel::Herbivore;
    }

    // Update threat level and emotional state
    CurrentAudioState.ThreatLevel = NewThreatLevel;
    
    if (Distance < 50.0f && NewThreatLevel >= EDinosaurThreatLevel::LargePredator)
    {
        TransitionToEmotionalState(EEmotionalState::Terror, 1.0f);
    }
    else if (Distance < 100.0f && NewThreatLevel >= EDinosaurThreatLevel::SmallPredator)
    {
        TransitionToEmotionalState(EEmotionalState::Fear, 2.0f);
    }
    else if (NewThreatLevel > EDinosaurThreatLevel::None)
    {
        TransitionToEmotionalState(EEmotionalState::Tension, 3.0f);
    }

    // Play dinosaur-specific sound at location
    // This would trigger 3D positioned audio based on species
    PlayEnvironmentalEvent(FString::Printf(TEXT("Dinosaur_%s"), *DinosaurSpecies), Location);
}

void UAudioSystemCore::PlayEnvironmentalEvent(FString EventName, FVector Location)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemCore: Environmental event - %s at location %s"), 
           *EventName, *Location.ToString());

    // This would trigger specific environmental sounds
    // Implementation would load appropriate sound cue and play at location
}

void UAudioSystemCore::TransitionToEmotionalState(EEmotionalState TargetState, float TransitionTime)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemCore: Transitioning to emotional state: %d"), (int32)TargetState);

    CurrentAudioState.CurrentEmotion = TargetState;
    
    // Calculate music parameters for target state
    float TargetIntensity = 0.0f;
    float TargetTension = 0.0f;
    
    CalculateEmotionalTransition(CurrentAudioState.CurrentEmotion, TargetState, TargetIntensity, TargetTension);
    
    // Apply parameters to MetaSound
    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        // Set MetaSound parameters for emotional transition
        if (auto ParameterController = MusicAudioComponent->GetParameterInterface())
        {
            ParameterController->SetFloatParameter(FName("Intensity"), TargetIntensity);
            ParameterController->SetFloatParameter(FName("Tension"), TargetTension);
            ParameterController->SetFloatParameter(FName("TransitionTime"), TransitionTime);
        }
    }
}

void UAudioSystemCore::SetMusicIntensity(float Intensity)
{
    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        if (auto ParameterController = MusicAudioComponent->GetParameterInterface())
        {
            ParameterController->SetFloatParameter(FName("MasterIntensity"), FMath::Clamp(Intensity, 0.0f, 1.0f));
        }
    }
}

void UAudioSystemCore::UpdateEnvironmentAmbience(EEnvironmentType NewEnvironment)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemCore: Updating environment ambience to: %d"), (int32)NewEnvironment);

    CurrentAudioState.Environment = NewEnvironment;

    if (AmbienceAudioComponent && AmbientForestMetaSound)
    {
        if (auto ParameterController = AmbienceAudioComponent->GetParameterInterface())
        {
            // Set environment-specific parameters
            switch (NewEnvironment)
            {
                case EEnvironmentType::DenseForest:
                    ParameterController->SetFloatParameter(FName("ForestDensity"), 1.0f);
                    ParameterController->SetFloatParameter(FName("WaterPresence"), 0.0f);
                    ParameterController->SetFloatParameter(FName("EchoAmount"), 0.2f);
                    break;
                case EEnvironmentType::Clearing:
                    ParameterController->SetFloatParameter(FName("ForestDensity"), 0.3f);
                    ParameterController->SetFloatParameter(FName("WaterPresence"), 0.0f);
                    ParameterController->SetFloatParameter(FName("EchoAmount"), 0.1f);
                    break;
                case EEnvironmentType::RiverBank:
                    ParameterController->SetFloatParameter(FName("ForestDensity"), 0.6f);
                    ParameterController->SetFloatParameter(FName("WaterPresence"), 1.0f);
                    ParameterController->SetFloatParameter(FName("EchoAmount"), 0.3f);
                    break;
                case EEnvironmentType::Cave:
                    ParameterController->SetFloatParameter(FName("ForestDensity"), 0.0f);
                    ParameterController->SetFloatParameter(FName("WaterPresence"), 0.2f);
                    ParameterController->SetFloatParameter(FName("EchoAmount"), 0.8f);
                    break;
                case EEnvironmentType::Cliffs:
                    ParameterController->SetFloatParameter(FName("ForestDensity"), 0.1f);
                    ParameterController->SetFloatParameter(FName("WaterPresence"), 0.0f);
                    ParameterController->SetFloatParameter(FName("EchoAmount"), 0.6f);
                    break;
                case EEnvironmentType::Swamp:
                    ParameterController->SetFloatParameter(FName("ForestDensity"), 0.8f);
                    ParameterController->SetFloatParameter(FName("WaterPresence"), 0.7f);
                    ParameterController->SetFloatParameter(FName("EchoAmount"), 0.4f);
                    break;
            }
        }
    }
}

void UAudioSystemCore::SetTimeOfDayAudio(float Hour, float TransitionSpeed)
{
    CurrentAudioState.TimeOfDay = Hour;

    if (AmbienceAudioComponent)
    {
        if (auto ParameterController = AmbienceAudioComponent->GetParameterInterface())
        {
            // Convert 24-hour time to 0-1 range for day/night cycle
            float DayNightCycle = FMath::Sin((Hour / 24.0f) * 2.0f * PI) * 0.5f + 0.5f;
            
            ParameterController->SetFloatParameter(FName("TimeOfDay"), Hour / 24.0f);
            ParameterController->SetFloatParameter(FName("DayNightIntensity"), DayNightCycle);
            ParameterController->SetFloatParameter(FName("TransitionSpeed"), TransitionSpeed);
        }
    }
}

void UAudioSystemCore::UpdateMusicParameters()
{
    if (!MusicAudioComponent) return;

    if (auto ParameterController = MusicAudioComponent->GetParameterInterface())
    {
        // Update all music parameters based on current state
        ParameterController->SetFloatParameter(FName("ThreatLevel"), (float)CurrentAudioState.ThreatLevel / 4.0f);
        ParameterController->SetFloatParameter(FName("PlayerHeartRate"), CurrentAudioState.PlayerHeartRate);
        ParameterController->SetBoolParameter(FName("IsPlayerHidden"), CurrentAudioState.bIsPlayerHidden);
        ParameterController->SetFloatParameter(FName("WeatherIntensity"), CurrentAudioState.WeatherIntensity);
    }
}

void UAudioSystemCore::UpdateAmbienceParameters()
{
    if (!AmbienceAudioComponent) return;

    if (auto ParameterController = AmbienceAudioComponent->GetParameterInterface())
    {
        // Update ambience parameters
        ParameterController->SetFloatParameter(FName("ThreatPresence"), (float)CurrentAudioState.ThreatLevel / 4.0f);
        ParameterController->SetFloatParameter(FName("WeatherIntensity"), CurrentAudioState.WeatherIntensity);
    }
}

void UAudioSystemCore::CalculateEmotionalTransition(EEmotionalState From, EEmotionalState To, float& IntensityTarget, float& TensionTarget)
{
    // Define emotional state mappings to musical parameters
    switch (To)
    {
        case EEmotionalState::Calm:
            IntensityTarget = 0.2f;
            TensionTarget = 0.1f;
            break;
        case EEmotionalState::Tension:
            IntensityTarget = 0.4f;
            TensionTarget = 0.6f;
            break;
        case EEmotionalState::Fear:
            IntensityTarget = 0.7f;
            TensionTarget = 0.8f;
            break;
        case EEmotionalState::Terror:
            IntensityTarget = 1.0f;
            TensionTarget = 1.0f;
            break;
        case EEmotionalState::Relief:
            IntensityTarget = 0.3f;
            TensionTarget = 0.2f;
            break;
        case EEmotionalState::Wonder:
            IntensityTarget = 0.5f;
            TensionTarget = 0.3f;
            break;
    }
}