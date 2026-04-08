#include "AudioSystemCore.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundSubmix.h"
#include "TimerManager.h"

UAudioSystemCore::UAudioSystemCore()
{
    // Initialize default audio state
    CurrentAudioState.ThreatLevel = EThreatLevel::Safe;
    CurrentAudioState.TimeOfDay = ETimeOfDay::Morning;
    CurrentAudioState.Environment = EAudioEnvironmentType::DenseForest;
    CurrentAudioState.Tension = 0.0f;
    CurrentAudioState.bPlayerHidden = false;
    CurrentAudioState.bDinosaurNearby = false;
    CurrentAudioState.NearbyDinosaurCount = 0;
}

void UAudioSystemCore::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Initializing adaptive audio system"));

    // Initialize environment settings with default values
    FAudioEnvironmentSettings ForestSettings;
    ForestSettings.AmbientVolume = 0.8f;
    ForestSettings.EchoIntensity = 0.3f;
    ForestSettings.HighFrequencyAbsorption = 0.4f;
    EnvironmentSettings.Add(EAudioEnvironmentType::DenseForest, ForestSettings);

    FAudioEnvironmentSettings PlainsSettings;
    PlainsSettings.AmbientVolume = 0.6f;
    PlainsSettings.EchoIntensity = 0.1f;
    PlainsSettings.HighFrequencyAbsorption = 0.2f;
    EnvironmentSettings.Add(EAudioEnvironmentType::OpenPlains, PlainsSettings);

    FAudioEnvironmentSettings CaveSettings;
    CaveSettings.AmbientVolume = 0.4f;
    CaveSettings.EchoIntensity = 0.8f;
    CaveSettings.HighFrequencyAbsorption = 0.6f;
    EnvironmentSettings.Add(EAudioEnvironmentType::Cave, CaveSettings);

    FAudioEnvironmentSettings SwampSettings;
    SwampSettings.AmbientVolume = 0.9f;
    SwampSettings.EchoIntensity = 0.5f;
    SwampSettings.HighFrequencyAbsorption = 0.7f;
    EnvironmentSettings.Add(EAudioEnvironmentType::Swamp, SwampSettings);

    FAudioEnvironmentSettings RiverSettings;
    RiverSettings.AmbientVolume = 0.7f;
    RiverSettings.EchoIntensity = 0.4f;
    RiverSettings.HighFrequencyAbsorption = 0.3f;
    EnvironmentSettings.Add(EAudioEnvironmentType::RiverBank, RiverSettings);

    FAudioEnvironmentSettings CanyonSettings;
    CanyonSettings.AmbientVolume = 0.5f;
    CanyonSettings.EchoIntensity = 0.9f;
    CanyonSettings.HighFrequencyAbsorption = 0.2f;
    EnvironmentSettings.Add(EAudioEnvironmentType::Canyon, CanyonSettings);

    // Start with the initial environment
    StartAmbientLoop(CurrentAudioState.Environment);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Audio system initialized successfully"));
}

void UAudioSystemCore::UpdateAudioState(const FAdaptiveMusicState& NewState)
{
    bool bStateChanged = false;

    // Check if any significant state has changed
    if (CurrentAudioState.ThreatLevel != NewState.ThreatLevel ||
        CurrentAudioState.Environment != NewState.Environment ||
        FMath::Abs(CurrentAudioState.Tension - NewState.Tension) > 0.1f ||
        CurrentAudioState.TimeOfDay != NewState.TimeOfDay)
    {
        bStateChanged = true;
    }

    if (bStateChanged)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Audio state changed - updating audio"));
        
        FAdaptiveMusicState PreviousState = CurrentAudioState;
        CurrentAudioState = NewState;

        // Update environment if changed
        if (PreviousState.Environment != NewState.Environment)
        {
            SetEnvironmentType(NewState.Environment);
        }

        // Update threat level if changed
        if (PreviousState.ThreatLevel != NewState.ThreatLevel)
        {
            SetThreatLevel(NewState.ThreatLevel);
        }

        // Update music tension
        if (FMath::Abs(PreviousState.Tension - NewState.Tension) > 0.1f)
        {
            UpdateMusicTension(NewState.Tension);
        }

        UpdateEnvironmentalAudio();
        UpdateAdaptiveMusic();
    }
}

void UAudioSystemCore::SetEnvironmentType(EAudioEnvironmentType NewEnvironment)
{
    if (CurrentAudioState.Environment != NewEnvironment)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Changing environment to %d"), (int32)NewEnvironment);
        
        CurrentAudioState.Environment = NewEnvironment;
        
        // Fade out current ambient sounds
        StopAmbientLoop(1.5f);
        
        // Start new ambient sounds after a brief delay
        if (UWorld* World = GetWorld())
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, NewEnvironment]()
            {
                StartAmbientLoop(NewEnvironment);
            }, 1.0f, false);
        }
    }
}

void UAudioSystemCore::SetThreatLevel(EThreatLevel NewThreatLevel, float TransitionTime)
{
    if (CurrentAudioState.ThreatLevel != NewThreatLevel)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Threat level changed to %d"), (int32)NewThreatLevel);
        
        EThreatLevel PreviousThreatLevel = CurrentAudioState.ThreatLevel;
        CurrentAudioState.ThreatLevel = NewThreatLevel;

        // Calculate new tension based on threat level
        float NewTension = 0.0f;
        switch (NewThreatLevel)
        {
            case EThreatLevel::Safe:
                NewTension = 0.0f;
                break;
            case EThreatLevel::Cautious:
                NewTension = 0.25f;
                break;
            case EThreatLevel::Danger:
                NewTension = 0.6f;
                break;
            case EThreatLevel::Imminent:
                NewTension = 0.85f;
                break;
            case EThreatLevel::Combat:
                NewTension = 1.0f;
                break;
        }

        UpdateMusicTension(NewTension, TransitionTime);
    }
}

void UAudioSystemCore::StartAdaptiveMusic()
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Starting adaptive music system"));
    
    if (!MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        if (UWorld* World = GetWorld())
        {
            MusicAudioComponent = UGameplayStatics::SpawnSound2D(World, AdaptiveMusicMetaSound);
            if (MusicAudioComponent)
            {
                MusicAudioComponent->SetVolumeMultiplier(0.7f); // Start at moderate volume
                UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Adaptive music started"));
            }
        }
    }
}

void UAudioSystemCore::StopAdaptiveMusic(float FadeOutTime)
{
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Stopping adaptive music"));
        MusicAudioComponent->FadeOut(FadeOutTime, 0.0f);
    }
}

void UAudioSystemCore::UpdateMusicTension(float NewTension, float TransitionTime)
{
    NewTension = FMath::Clamp(NewTension, 0.0f, 1.0f);
    
    if (FMath::Abs(CurrentAudioState.Tension - NewTension) > 0.05f)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Updating music tension to %f"), NewTension);
        
        CurrentAudioState.Tension = NewTension;

        // Update MetaSound parameters if music is playing
        if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
        {
            // Set MetaSound parameters for tension, threat level, environment, etc.
            MusicAudioComponent->SetFloatParameter(FName("Tension"), NewTension);
            MusicAudioComponent->SetIntParameter(FName("ThreatLevel"), (int32)CurrentAudioState.ThreatLevel);
            MusicAudioComponent->SetIntParameter(FName("Environment"), (int32)CurrentAudioState.Environment);
            MusicAudioComponent->SetIntParameter(FName("TimeOfDay"), (int32)CurrentAudioState.TimeOfDay);
        }
    }
}

void UAudioSystemCore::PlayEnvironmentalSound(USoundCue* SoundCue, FVector Location, float VolumeMultiplier)
{
    if (SoundCue)
    {
        if (UWorld* World = GetWorld())
        {
            UGameplayStatics::PlaySoundAtLocation(World, SoundCue, Location, VolumeMultiplier);
        }
    }
}

void UAudioSystemCore::StartAmbientLoop(EAudioEnvironmentType Environment)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Starting ambient loop for environment %d"), (int32)Environment);
    
    if (EnvironmentSettings.Contains(Environment))
    {
        const FAudioEnvironmentSettings& Settings = EnvironmentSettings[Environment];
        
        // Play ambient sounds for this environment
        for (USoundCue* AmbientSound : Settings.AmbientSounds)
        {
            if (AmbientSound && GetWorld())
            {
                UAudioComponent* AmbientComponent = UGameplayStatics::SpawnSound2D(GetWorld(), AmbientSound);
                if (AmbientComponent)
                {
                    AmbientComponent->SetVolumeMultiplier(Settings.AmbientVolume);
                    AmbientAudioComponents.Add(AmbientComponent);
                }
            }
        }
    }
}

void UAudioSystemCore::StopAmbientLoop(float FadeOutTime)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Stopping ambient loops"));
    
    for (UAudioComponent* AmbientComponent : AmbientAudioComponents)
    {
        if (AmbientComponent && AmbientComponent->IsPlaying())
        {
            AmbientComponent->FadeOut(FadeOutTime, 0.0f);
        }
    }
    
    // Clear the array after a delay to allow fade out
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            AmbientAudioComponents.Empty();
        }, FadeOutTime + 0.5f, false);
    }
}

void UAudioSystemCore::RegisterDinosaurPresence(FVector DinosaurLocation, float ThreatRadius)
{
    RegisteredDinosaurLocations.AddUnique(DinosaurLocation);
    CurrentAudioState.NearbyDinosaurCount = RegisteredDinosaurLocations.Num();
    CurrentAudioState.bDinosaurNearby = CurrentAudioState.NearbyDinosaurCount > 0;
    
    // Increase tension based on dinosaur proximity
    float AdditionalTension = FMath::Min(0.3f * CurrentAudioState.NearbyDinosaurCount, 0.8f);
    float NewTension = FMath::Min(CurrentAudioState.Tension + AdditionalTension, 1.0f);
    
    UpdateMusicTension(NewTension, 0.5f);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Dinosaur registered. Total nearby: %d"), CurrentAudioState.NearbyDinosaurCount);
}

void UAudioSystemCore::UnregisterDinosaurPresence(FVector DinosaurLocation)
{
    RegisteredDinosaurLocations.Remove(DinosaurLocation);
    CurrentAudioState.NearbyDinosaurCount = RegisteredDinosaurLocations.Num();
    CurrentAudioState.bDinosaurNearby = CurrentAudioState.NearbyDinosaurCount > 0;
    
    // Decrease tension when dinosaurs leave
    if (CurrentAudioState.NearbyDinosaurCount == 0)
    {
        float BaseTension = CalculateTensionFromThreats();
        UpdateMusicTension(BaseTension, 2.0f); // Slower relaxation
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemCore: Dinosaur unregistered. Total nearby: %d"), CurrentAudioState.NearbyDinosaurCount);
}

void UAudioSystemCore::PlayDinosaurSound(USoundCue* DinosaurSound, FVector Location, float MaxDistance)
{
    if (DinosaurSound && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), DinosaurSound, Location, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
    }
}

void UAudioSystemCore::UpdateEnvironmentalAudio()
{
    // Update environmental audio parameters based on current state
    if (EnvironmentSettings.Contains(CurrentAudioState.Environment))
    {
        const FAudioEnvironmentSettings& Settings = EnvironmentSettings[CurrentAudioState.Environment];
        
        // Apply environmental effects to submixes
        if (AmbientSubmix)
        {
            UAudioMixerBlueprintLibrary::SetBypassSubmixEffectChainEntry(GetWorld(), AmbientSubmix, 0, false);
        }
    }
}

void UAudioSystemCore::UpdateAdaptiveMusic()
{
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        // Update all MetaSound parameters
        MusicAudioComponent->SetFloatParameter(FName("Tension"), CurrentAudioState.Tension);
        MusicAudioComponent->SetIntParameter(FName("ThreatLevel"), (int32)CurrentAudioState.ThreatLevel);
        MusicAudioComponent->SetIntParameter(FName("Environment"), (int32)CurrentAudioState.Environment);
        MusicAudioComponent->SetIntParameter(FName("TimeOfDay"), (int32)CurrentAudioState.TimeOfDay);
        MusicAudioComponent->SetBoolParameter(FName("PlayerHidden"), CurrentAudioState.bPlayerHidden);
        MusicAudioComponent->SetBoolParameter(FName("DinosaurNearby"), CurrentAudioState.bDinosaurNearby);
        MusicAudioComponent->SetIntParameter(FName("DinosaurCount"), CurrentAudioState.NearbyDinosaurCount);
    }
}

float UAudioSystemCore::CalculateTensionFromThreats() const
{
    float BaseTension = 0.0f;
    
    // Base tension from threat level
    switch (CurrentAudioState.ThreatLevel)
    {
        case EThreatLevel::Safe: BaseTension = 0.0f; break;
        case EThreatLevel::Cautious: BaseTension = 0.2f; break;
        case EThreatLevel::Danger: BaseTension = 0.5f; break;
        case EThreatLevel::Imminent: BaseTension = 0.8f; break;
        case EThreatLevel::Combat: BaseTension = 1.0f; break;
    }
    
    // Additional tension from nearby dinosaurs
    float DinosaurTension = FMath::Min(0.1f * CurrentAudioState.NearbyDinosaurCount, 0.4f);
    
    // Time of day influence
    float TimeInfluence = CalculateTimeOfDayInfluence();
    
    return FMath::Clamp(BaseTension + DinosaurTension + TimeInfluence, 0.0f, 1.0f);
}

float UAudioSystemCore::CalculateTimeOfDayInfluence() const
{
    switch (CurrentAudioState.TimeOfDay)
    {
        case ETimeOfDay::Night:
        case ETimeOfDay::DeepNight:
            return 0.2f; // Night is more tense
        case ETimeOfDay::Dusk:
        case ETimeOfDay::Dawn:
            return 0.1f; // Transition times are slightly tense
        default:
            return 0.0f; // Day is neutral
    }
}

float UAudioSystemCore::CalculateEnvironmentalInfluence() const
{
    switch (CurrentAudioState.Environment)
    {
        case EAudioEnvironmentType::DenseForest:
        case EAudioEnvironmentType::Swamp:
            return 0.1f; // Dense environments are more tense
        case EAudioEnvironmentType::Cave:
            return 0.15f; // Caves are very tense
        default:
            return 0.0f; // Open areas are neutral
    }
}