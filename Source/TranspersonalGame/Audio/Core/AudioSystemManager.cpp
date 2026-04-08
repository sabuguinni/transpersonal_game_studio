#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "AudioDevice.h"
#include "Kismet/GameplayStatics.h"

// Parameter name constants
const FName UAudioSystemManager::PARAM_ThreatLevel = "ThreatLevel";
const FName UAudioSystemManager::PARAM_TimeOfDay = "TimeOfDay";
const FName UAudioSystemManager::PARAM_WeatherIntensity = "WeatherIntensity";
const FName UAudioSystemManager::PARAM_EnvironmentType = "EnvironmentType";

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default state
    CurrentEnvironmentState = EAudioEnvironmentState::Forest_Day_Calm;
    CurrentThreatLevel = EThreatLevel::Safe;
    CurrentTimeOfDay = 0.5f; // Start at noon
    CurrentWeatherIntensity = 0.0f; // Clear weather
    
    InitializeAudioComponents();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio System Manager Initialized"));
}

void UAudioSystemManager::Deinitialize()
{
    if (MusicAudioComponent)
    {
        MusicAudioComponent->Stop();
        MusicAudioComponent = nullptr;
    }
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
        AmbientAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Create Music Audio Component
    if (AdaptiveMusicMetaSound)
    {
        MusicAudioComponent = UGameplayStatics::CreateSound2D(
            World, 
            AdaptiveMusicMetaSound, 
            1.0f, 
            1.0f, 
            0.0f, 
            nullptr, 
            true, // Persistent
            false  // Auto destroy
        );
        
        if (MusicAudioComponent)
        {
            MusicAudioComponent->bAutoActivate = false;
        }
    }

    // Create Ambient Audio Component
    if (AmbientLayersMetaSound)
    {
        AmbientAudioComponent = UGameplayStatics::CreateSound2D(
            World, 
            AmbientLayersMetaSound, 
            1.0f, 
            1.0f, 
            0.0f, 
            nullptr, 
            true, // Persistent
            false  // Auto destroy
        );
        
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->bAutoActivate = false;
            AmbientAudioComponent->Play();
        }
    }
}

void UAudioSystemManager::SetEnvironmentState(EAudioEnvironmentState NewState)
{
    if (CurrentEnvironmentState != NewState)
    {
        CurrentEnvironmentState = NewState;
        UpdateMusicParameters();
        UpdateAmbientParameters();
        
        OnAudioStateChanged.Broadcast(FString::Printf(TEXT("Environment: %d"), (int32)NewState));
    }
}

void UAudioSystemManager::SetThreatLevel(EThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        CurrentThreatLevel = NewThreatLevel;
        UpdateMusicParameters();
        
        OnAudioStateChanged.Broadcast(FString::Printf(TEXT("Threat: %d"), (int32)NewThreatLevel));
    }
}

void UAudioSystemManager::SetTimeOfDay(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 1.0f);
    UpdateMusicParameters();
    UpdateAmbientParameters();
}

void UAudioSystemManager::SetWeatherIntensity(float Intensity)
{
    CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UpdateMusicParameters();
    UpdateAmbientParameters();
}

void UAudioSystemManager::UpdateMusicParameters()
{
    if (!MusicAudioComponent) return;

    // Convert threat level to float
    float ThreatValue = (float)CurrentThreatLevel / 4.0f; // 0.0 to 1.0
    
    // Convert environment to float for MetaSound parameter
    float EnvironmentValue = (float)CurrentEnvironmentState / 11.0f; // 0.0 to 1.0
    
    // Set MetaSound parameters
    MusicAudioComponent->SetFloatParameter(PARAM_ThreatLevel, ThreatValue);
    MusicAudioComponent->SetFloatParameter(PARAM_TimeOfDay, CurrentTimeOfDay);
    MusicAudioComponent->SetFloatParameter(PARAM_WeatherIntensity, CurrentWeatherIntensity);
    MusicAudioComponent->SetFloatParameter(PARAM_EnvironmentType, EnvironmentValue);
}

void UAudioSystemManager::UpdateAmbientParameters()
{
    if (!AmbientAudioComponent) return;

    // Same parameter updates for ambient layers
    float EnvironmentValue = (float)CurrentEnvironmentState / 11.0f;
    
    AmbientAudioComponent->SetFloatParameter(PARAM_TimeOfDay, CurrentTimeOfDay);
    AmbientAudioComponent->SetFloatParameter(PARAM_WeatherIntensity, CurrentWeatherIntensity);
    AmbientAudioComponent->SetFloatParameter(PARAM_EnvironmentType, EnvironmentValue);
}

void UAudioSystemManager::TriggerMusicTransition(const FString& TargetState, float TransitionTime)
{
    if (!MusicAudioComponent) return;

    // Start music if not already playing
    if (!MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->Play();
    }
    
    // Set transition time parameter
    MusicAudioComponent->SetFloatParameter("TransitionTime", TransitionTime);
    
    UE_LOG(LogTemp, Warning, TEXT("Music transition triggered: %s"), *TargetState);
}

void UAudioSystemManager::PlayDinosaurCall(const FString& DinosaurSpecies, const FVector& Location, float Intensity)
{
    if (!DinosaurCallsMetaSound) return;

    // Create 3D positioned audio component for dinosaur call
    UAudioComponent* CallComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        DinosaurCallsMetaSound,
        Location,
        FRotator::ZeroRotator,
        Intensity,
        1.0f, // Pitch
        0.0f, // Start time
        nullptr, // Attenuation
        nullptr, // Concurrency
        true // Auto destroy
    );
    
    if (CallComponent)
    {
        // Set species-specific parameters
        CallComponent->SetStringParameter("Species", DinosaurSpecies);
        CallComponent->SetFloatParameter("Intensity", Intensity);
        CallComponent->SetFloatParameter("ThreatLevel", (float)CurrentThreatLevel / 4.0f);
    }
}

void UAudioSystemManager::PlayFootstepSequence(const FString& SurfaceType, const FVector& Location, float Weight)
{
    // This would trigger a MetaSound that handles procedural footstep generation
    // Based on surface type and creature weight
    UE_LOG(LogTemp, Log, TEXT("Footstep: %s at %s, Weight: %f"), *SurfaceType, *Location.ToString(), Weight);
}

void UAudioSystemManager::UpdateAmbientLayers()
{
    UpdateAmbientParameters();
}