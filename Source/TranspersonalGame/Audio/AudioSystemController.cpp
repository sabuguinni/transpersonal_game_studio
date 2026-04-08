#include "AudioSystemArchitecture.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "AudioDevice.h"

UAudioSystemController::UAudioSystemController()
{
    // Initialize default audio state
    CurrentAudioState.CurrentTension = ETensionState::Safe;
    CurrentAudioState.Environment = EEnvironmentalContext::Dense_Forest;
    CurrentAudioState.TimeOfDay = 12.0f;
    CurrentAudioState.WeatherIntensity = 0.0f;
    CurrentAudioState.NearestPredatorDistance = 10000.0f;
    CurrentAudioState.NearestHerbivoreDistance = 10000.0f;
    CurrentAudioState.bPlayerIsHidden = false;
    CurrentAudioState.bPlayerIsMoving = false;
    CurrentAudioState.PlayerHeartRate = 60.0f;
}

void UAudioSystemController::InitializeAudioSystem()
{
    UE_LOG(LogAudio, Log, TEXT("Initializing Adaptive Audio System"));
    
    // Create audio components for each layer
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogAudio, Error, TEXT("Failed to get world for audio system initialization"));
        return;
    }

    // Initialize environmental layers
    for (int32 i = 0; i < (int32)EAudioLayer::Temporary_Relief + 1; ++i)
    {
        EAudioLayer Layer = (EAudioLayer)i;
        
        if (AudioLayers.Contains(Layer) && AudioLayers[Layer])
        {
            UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(
                World, 
                AudioLayers[Layer], 
                0.0f, // Start at 0 volume
                1.0f, // Normal pitch
                0.0f, // Start time
                nullptr, // No concurrency
                false,  // Not persistent
                true    // Auto destroy
            );
            
            if (AudioComp)
            {
                AudioComp->SetVolumeMultiplier(0.0f);
                AudioComp->bAutoDestroy = false; // We manage these manually
                LayerComponents.Add(Layer, AudioComp);
                
                UE_LOG(LogAudio, Log, TEXT("Initialized audio layer: %d"), (int32)Layer);
            }
        }
    }
    
    // Start with base environmental audio
    UpdateEnvironmentalLayers();
}

void UAudioSystemController::UpdateAudioState(const FAudioStateData& NewState)
{
    FAudioStateData PreviousState = CurrentAudioState;
    CurrentAudioState = NewState;
    
    // Check for significant changes that require audio updates
    bool bTensionChanged = (PreviousState.CurrentTension != NewState.CurrentTension);
    bool bEnvironmentChanged = (PreviousState.Environment != NewState.Environment);
    bool bTimeChanged = (FMath::Abs(PreviousState.TimeOfDay - NewState.TimeOfDay) > 0.5f);
    bool bWeatherChanged = (FMath::Abs(PreviousState.WeatherIntensity - NewState.WeatherIntensity) > 0.1f);
    bool bPredatorProximityChanged = (FMath::Abs(PreviousState.NearestPredatorDistance - NewState.NearestPredatorDistance) > 500.0f);
    
    if (bTensionChanged)
    {
        UpdateTensionLayers();
    }
    
    if (bEnvironmentChanged || bTimeChanged || bWeatherChanged)
    {
        UpdateEnvironmentalLayers();
    }
    
    if (bPredatorProximityChanged)
    {
        UpdateCreatureLayers();
    }
    
    // Always update subtle biometric effects
    ApplyHeartRateFiltering();
    ApplyBreathingModulation();
}

void UAudioSystemController::SetTensionLevel(ETensionState NewTension, float TransitionTime)
{
    if (CurrentAudioState.CurrentTension == NewTension)
    {
        return; // No change needed
    }
    
    UE_LOG(LogAudio, Log, TEXT("Changing tension from %d to %d over %.2f seconds"), 
           (int32)CurrentAudioState.CurrentTension, (int32)NewTension, TransitionTime);
    
    CurrentAudioState.CurrentTension = NewTension;
    UpdateTensionLayers();
}

void UAudioSystemController::UpdateEnvironmentalLayers()
{
    // Calculate base environmental volume based on context
    float BaseVolume = 0.6f;
    
    // Adjust for time of day
    float TimeOfDayFactor = 1.0f;
    if (CurrentAudioState.TimeOfDay < 6.0f || CurrentAudioState.TimeOfDay > 20.0f)
    {
        // Night time - more ominous
        TimeOfDayFactor = 1.2f;
        CrossfadeToLayer(EAudioLayer::TimeOfDay_Layer, 0.4f, DefaultTransitionTime);
    }
    else
    {
        // Day time
        CrossfadeToLayer(EAudioLayer::TimeOfDay_Layer, 0.2f, DefaultTransitionTime);
    }
    
    // Weather layer intensity
    float WeatherVolume = CurrentAudioState.WeatherIntensity * 0.5f;
    CrossfadeToLayer(EAudioLayer::Weather_Layer, WeatherVolume, DefaultTransitionTime * 0.5f);
    
    // Base environmental layer adjusted by context
    switch (CurrentAudioState.Environment)
    {
        case EEnvironmentalContext::Dense_Forest:
            BaseVolume = 0.7f; // Rich forest ambience
            break;
        case EEnvironmentalContext::Open_Plains:
            BaseVolume = 0.4f; // Sparse, wind-dominated
            break;
        case EEnvironmentalContext::Cave_System:
            BaseVolume = 0.3f; // Echoey, minimal
            break;
        case EEnvironmentalContext::Player_Base:
            BaseVolume = 0.5f; // Safer feeling
            break;
        default:
            BaseVolume = 0.6f;
            break;
    }
    
    CrossfadeToLayer(EAudioLayer::Environmental_Base, BaseVolume * TimeOfDayFactor, DefaultTransitionTime);
}

void UAudioSystemController::UpdateTensionLayers()
{
    // Fade out all tension layers first
    CrossfadeToLayer(EAudioLayer::Tension_Subtle, 0.0f, DefaultTransitionTime * 0.5f);
    CrossfadeToLayer(EAudioLayer::Tension_Medium, 0.0f, DefaultTransitionTime * 0.5f);
    CrossfadeToLayer(EAudioLayer::Tension_High, 0.0f, DefaultTransitionTime * 0.5f);
    CrossfadeToLayer(EAudioLayer::Tension_Extreme, 0.0f, DefaultTransitionTime * 0.5f);
    
    // Fade in appropriate tension layer
    float TensionVolume = 0.0f;
    EAudioLayer ActiveTensionLayer = EAudioLayer::Environmental_Base;
    
    switch (CurrentAudioState.CurrentTension)
    {
        case ETensionState::Safe:
            // No tension layers, maybe some relief
            CrossfadeToLayer(EAudioLayer::Base_Safety, 0.3f, DefaultTransitionTime);
            break;
            
        case ETensionState::Uneasy:
            ActiveTensionLayer = EAudioLayer::Tension_Subtle;
            TensionVolume = 0.2f;
            break;
            
        case ETensionState::Tense:
            ActiveTensionLayer = EAudioLayer::Tension_Medium;
            TensionVolume = 0.4f;
            break;
            
        case ETensionState::Danger:
            ActiveTensionLayer = EAudioLayer::Tension_High;
            TensionVolume = 0.6f;
            break;
            
        case ETensionState::Terror:
            ActiveTensionLayer = EAudioLayer::Tension_Extreme;
            TensionVolume = 0.8f;
            break;
    }
    
    if (TensionVolume > 0.0f)
    {
        CrossfadeToLayer(ActiveTensionLayer, TensionVolume, DefaultTransitionTime);
        // Reduce safety layer when tension is present
        CrossfadeToLayer(EAudioLayer::Base_Safety, 0.0f, DefaultTransitionTime);
    }
}

void UAudioSystemController::UpdateCreatureLayers()
{
    // Predator proximity affects tension and specific creature layers
    float PredatorProximity = FMath::Clamp(
        (5000.0f - CurrentAudioState.NearestPredatorDistance) / 5000.0f, 
        0.0f, 1.0f
    );
    
    if (PredatorProximity > 0.1f)
    {
        // Predator is relatively close
        if (PredatorProximity > 0.7f)
        {
            // Very close - hunting behavior
            CrossfadeToLayer(EAudioLayer::Predator_Hunting, 0.5f * PredatorProximity, 1.0f);
            CrossfadeToLayer(EAudioLayer::Predator_Stalking, 0.0f, 1.0f);
        }
        else
        {
            // Medium distance - stalking behavior
            CrossfadeToLayer(EAudioLayer::Predator_Stalking, 0.3f * PredatorProximity, 2.0f);
            CrossfadeToLayer(EAudioLayer::Predator_Hunting, 0.0f, 1.0f);
        }
    }
    else
    {
        // No immediate predator threat
        CrossfadeToLayer(EAudioLayer::Predator_Stalking, 0.0f, DefaultTransitionTime);
        CrossfadeToLayer(EAudioLayer::Predator_Hunting, 0.0f, DefaultTransitionTime);
    }
    
    // Herbivore proximity - can provide some relief or warning
    float HerbivoreProximity = FMath::Clamp(
        (3000.0f - CurrentAudioState.NearestHerbivoreDistance) / 3000.0f, 
        0.0f, 1.0f
    );
    
    if (HerbivoreProximity > 0.2f)
    {
        if (HerbivoreProximity > 0.6f)
        {
            CrossfadeToLayer(EAudioLayer::Herbivore_Close, 0.3f * HerbivoreProximity, 2.0f);
        }
        else
        {
            CrossfadeToLayer(EAudioLayer::Herbivore_Distant, 0.2f * HerbivoreProximity, 3.0f);
        }
    }
    else
    {
        CrossfadeToLayer(EAudioLayer::Herbivore_Close, 0.0f, DefaultTransitionTime);
        CrossfadeToLayer(EAudioLayer::Herbivore_Distant, 0.0f, DefaultTransitionTime);
    }
}

void UAudioSystemController::CrossfadeToLayer(EAudioLayer Layer, float TargetVolume, float TransitionTime)
{
    UAudioComponent** AudioCompPtr = LayerComponents.Find(Layer);
    if (AudioCompPtr && *AudioCompPtr)
    {
        UAudioComponent* AudioComp = *AudioCompPtr;
        
        // Clamp target volume
        TargetVolume = FMath::Clamp(TargetVolume, 0.0f, MaxLayerVolume);
        
        // TODO: Implement smooth crossfading using UE5's audio parameter system
        // For now, set volume directly
        AudioComp->SetVolumeMultiplier(TargetVolume);
        
        UE_LOG(LogAudio, VeryVerbose, TEXT("Crossfading layer %d to volume %.2f over %.2f seconds"), 
               (int32)Layer, TargetVolume, TransitionTime);
    }
}

void UAudioSystemController::ApplyHeartRateFiltering()
{
    // Subtle high-frequency filtering based on player stress (heart rate)
    float StressFactor = FMath::Clamp((CurrentAudioState.PlayerHeartRate - 60.0f) / 60.0f, 0.0f, 1.0f);
    
    // Apply subtle filtering to environmental layers when stressed
    // This creates a subconscious feeling of muffled hearing under stress
    if (StressFactor > 0.2f)
    {
        // TODO: Apply low-pass filtering to environmental layers
        // This would be implemented through MetaSound parameter modulation
    }
}

void UAudioSystemController::ApplyBreathingModulation()
{
    // Subtle amplitude modulation based on breathing patterns
    // Creates subconscious rhythm that affects player's breathing
    if (CurrentAudioState.CurrentTension >= ETensionState::Tense)
    {
        // TODO: Apply subtle amplitude modulation to tension layers
        // Frequency should match elevated breathing rate (15-20 BPM)
    }
}

void UAudioSystemController::TriggerAudioEvent(const FString& EventName, FVector Location)
{
    UE_LOG(LogAudio, Log, TEXT("Triggering audio event: %s at location: %s"), 
           *EventName, *Location.ToString());
    
    // Handle specific audio events
    if (EventName == "DinosaurRoar")
    {
        // Temporarily spike tension
        ETensionState OriginalTension = CurrentAudioState.CurrentTension;
        SetTensionLevel(ETensionState::Terror, 0.5f);
        
        // TODO: Set timer to return to original tension after roar effect
    }
    else if (EventName == "SafeZoneEntered")
    {
        SetTensionLevel(ETensionState::Safe, 3.0f);
        CrossfadeToLayer(EAudioLayer::Temporary_Relief, 0.4f, 2.0f);
    }
    else if (EventName == "FootstepsDetected")
    {
        // Increase tension based on current state
        if (CurrentAudioState.CurrentTension < ETensionState::Tense)
        {
            SetTensionLevel(ETensionState::Uneasy, 1.0f);
        }
    }
}