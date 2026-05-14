#include "Audio_EnvironmentalSoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

void UAudio_EnvironmentalSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_EnvironmentalSoundManager: Initializing environmental sound system"));
    
    // Initialize default biome settings
    InitializeBiomeSettings();
    
    // Set default values
    CurrentBiome = EBiomeType::Forest;
    MasterEnvironmentalVolume = 1.0f;
    CurrentTimeOfDay = 0.5f; // Noon
    CurrentWeatherIntensity = 0.0f; // Clear weather
    
    // Create audio components when world is available
    if (UWorld* World = GetWorld())
    {
        AmbientAudioComponent = CreateAudioComponent(TEXT("AmbientAudio"));
        WindAudioComponent = CreateAudioComponent(TEXT("WindAudio"));
        WeatherAudioComponent = CreateAudioComponent(TEXT("WeatherAudio"));
        
        // Start random nature sound timer
        World->GetTimerManager().SetTimer(RandomSoundTimer, this, &UAudio_EnvironmentalSoundManager::PlayRandomNatureSound, 15.0f, true);
    }
}

void UAudio_EnvironmentalSoundManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RandomSoundTimer);
    }
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
        AmbientAudioComponent = nullptr;
    }
    
    if (WindAudioComponent)
    {
        WindAudioComponent->Stop();
        WindAudioComponent = nullptr;
    }
    
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->Stop();
        WeatherAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudio_EnvironmentalSoundManager::SetCurrentBiome(EBiomeType NewBiome)
{
    if (CurrentBiome == NewBiome)
    {
        return;
    }
    
    CurrentBiome = NewBiome;
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_EnvironmentalSoundManager: Switching to biome: %d"), (int32)NewBiome);
    
    // Update ambient audio based on new biome
    if (BiomeAudioSettings.Contains(NewBiome))
    {
        const FAudio_BiomeAudioSettings& Settings = BiomeAudioSettings[NewBiome];
        
        // Load and play ambient sound
        if (AmbientAudioComponent && Settings.AmbientLoop.IsValid())
        {
            if (USoundBase* AmbientSound = Settings.AmbientLoop.LoadSynchronous())
            {
                AmbientAudioComponent->SetSound(AmbientSound);
                AmbientAudioComponent->SetVolumeMultiplier(Settings.AmbientVolume * MasterEnvironmentalVolume);
                AmbientAudioComponent->Play();
            }
        }
        
        // Update wind sound
        if (WindAudioComponent && Settings.WindSound.IsValid())
        {
            if (USoundBase* WindSound = Settings.WindSound.LoadSynchronous())
            {
                WindAudioComponent->SetSound(WindSound);
                WindAudioComponent->SetVolumeMultiplier(0.5f * MasterEnvironmentalVolume);
                WindAudioComponent->Play();
            }
        }
        
        // Update random sound timer interval
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(RandomSoundTimer);
            World->GetTimerManager().SetTimer(RandomSoundTimer, this, &UAudio_EnvironmentalSoundManager::PlayRandomNatureSound, Settings.RandomSoundInterval, true);
        }
    }
}

void UAudio_EnvironmentalSoundManager::UpdatePlayerLocation(const FVector& PlayerLocation)
{
    LastPlayerLocation = PlayerLocation;
    UpdateProximityTriggers(PlayerLocation);
}

void UAudio_EnvironmentalSoundManager::PlayProximitySound(const FString& TriggerName, const FVector& Location)
{
    if (ProximityTriggers.Contains(TriggerName))
    {
        const FAudio_ProximityAudioTrigger& Trigger = ProximityTriggers[TriggerName];
        
        // Check cooldown
        if (TriggerCooldowns.Contains(TriggerName))
        {
            float LastTriggerTime = TriggerCooldowns[TriggerName];
            if (UWorld* World = GetWorld())
            {
                float CurrentTime = World->GetTimeSeconds();
                if (CurrentTime - LastTriggerTime < Trigger.Cooldown)
                {
                    return; // Still on cooldown
                }
            }
        }
        
        // Play proximity sound
        if (Trigger.ProximitySound.IsValid())
        {
            if (USoundBase* Sound = Trigger.ProximitySound.LoadSynchronous())
            {
                UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, Trigger.Volume * MasterEnvironmentalVolume);
                
                // Update cooldown
                if (UWorld* World = GetWorld())
                {
                    TriggerCooldowns.Add(TriggerName, World->GetTimeSeconds());
                }
            }
        }
    }
}

void UAudio_EnvironmentalSoundManager::SetMasterEnvironmentalVolume(float Volume)
{
    MasterEnvironmentalVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active audio components
    if (AmbientAudioComponent)
    {
        float BiomeVolume = BiomeAudioSettings.Contains(CurrentBiome) ? BiomeAudioSettings[CurrentBiome].AmbientVolume : 0.7f;
        AmbientAudioComponent->SetVolumeMultiplier(BiomeVolume * MasterEnvironmentalVolume);
    }
    
    if (WindAudioComponent)
    {
        WindAudioComponent->SetVolumeMultiplier(0.5f * MasterEnvironmentalVolume);
    }
    
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->SetVolumeMultiplier(CurrentWeatherIntensity * MasterEnvironmentalVolume);
    }
}

void UAudio_EnvironmentalSoundManager::AddProximityTrigger(const FString& TriggerName, const FAudio_ProximityAudioTrigger& Trigger)
{
    ProximityTriggers.Add(TriggerName, Trigger);
    UE_LOG(LogTemp, Warning, TEXT("Audio_EnvironmentalSoundManager: Added proximity trigger: %s"), *TriggerName);
}

void UAudio_EnvironmentalSoundManager::RemoveProximityTrigger(const FString& TriggerName)
{
    ProximityTriggers.Remove(TriggerName);
    TriggerCooldowns.Remove(TriggerName);
}

void UAudio_EnvironmentalSoundManager::SetTimeOfDay(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 1.0f);
    UpdateTimeBasedAudio();
}

void UAudio_EnvironmentalSoundManager::SetWeatherIntensity(float Intensity)
{
    CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UpdateWeatherAudio();
}

void UAudio_EnvironmentalSoundManager::InitializeBiomeSettings()
{
    // Forest biome settings
    FAudio_BiomeAudioSettings ForestSettings;
    ForestSettings.AmbientVolume = 0.7f;
    ForestSettings.RandomSoundInterval = 12.0f;
    BiomeAudioSettings.Add(EBiomeType::Forest, ForestSettings);
    
    // Swamp biome settings
    FAudio_BiomeAudioSettings SwampSettings;
    SwampSettings.AmbientVolume = 0.8f;
    SwampSettings.RandomSoundInterval = 18.0f;
    BiomeAudioSettings.Add(EBiomeType::Swamp, SwampSettings);
    
    // Savanna biome settings
    FAudio_BiomeAudioSettings SavannaSettings;
    SavannaSettings.AmbientVolume = 0.6f;
    SavannaSettings.RandomSoundInterval = 20.0f;
    BiomeAudioSettings.Add(EBiomeType::Savanna, SavannaSettings);
    
    // Desert biome settings
    FAudio_BiomeAudioSettings DesertSettings;
    DesertSettings.AmbientVolume = 0.4f;
    DesertSettings.RandomSoundInterval = 30.0f;
    BiomeAudioSettings.Add(EBiomeType::Desert, DesertSettings);
    
    // Mountain biome settings
    FAudio_BiomeAudioSettings MountainSettings;
    MountainSettings.AmbientVolume = 0.5f;
    MountainSettings.RandomSoundInterval = 25.0f;
    BiomeAudioSettings.Add(EBiomeType::Mountain, MountainSettings);
}

void UAudio_EnvironmentalSoundManager::PlayRandomNatureSound()
{
    if (!BiomeAudioSettings.Contains(CurrentBiome))
    {
        return;
    }
    
    const FAudio_BiomeAudioSettings& Settings = BiomeAudioSettings[CurrentBiome];
    
    if (Settings.RandomNatureSounds.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, Settings.RandomNatureSounds.Num() - 1);
        TSoftObjectPtr<USoundBase> RandomSound = Settings.RandomNatureSounds[RandomIndex];
        
        if (RandomSound.IsValid())
        {
            if (USoundBase* Sound = RandomSound.LoadSynchronous())
            {
                // Play at player location with some random offset
                FVector PlayLocation = LastPlayerLocation + FVector(
                    FMath::RandRange(-500.0f, 500.0f),
                    FMath::RandRange(-500.0f, 500.0f),
                    FMath::RandRange(-100.0f, 100.0f)
                );
                
                UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, PlayLocation, 0.6f * MasterEnvironmentalVolume);
            }
        }
    }
}

void UAudio_EnvironmentalSoundManager::UpdateProximityTriggers(const FVector& PlayerLocation)
{
    for (auto& TriggerPair : ProximityTriggers)
    {
        const FString& TriggerName = TriggerPair.Key;
        const FAudio_ProximityAudioTrigger& Trigger = TriggerPair.Value;
        
        if (!Trigger.bIsActive)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Trigger.TriggerLocation);
        
        if (Distance <= Trigger.TriggerRadius)
        {
            PlayProximitySound(TriggerName, Trigger.TriggerLocation);
        }
    }
}

void UAudio_EnvironmentalSoundManager::UpdateTimeBasedAudio()
{
    // Adjust ambient volume based on time of day
    // Night time (0.0-0.2 and 0.8-1.0) should be quieter
    float TimeVolumeMultiplier = 1.0f;
    
    if (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f)
    {
        // Night time - reduce ambient volume
        TimeVolumeMultiplier = 0.6f;
    }
    else if (CurrentTimeOfDay < 0.3f || CurrentTimeOfDay > 0.7f)
    {
        // Dawn/dusk - moderate volume
        TimeVolumeMultiplier = 0.8f;
    }
    
    if (AmbientAudioComponent)
    {
        float BiomeVolume = BiomeAudioSettings.Contains(CurrentBiome) ? BiomeAudioSettings[CurrentBiome].AmbientVolume : 0.7f;
        AmbientAudioComponent->SetVolumeMultiplier(BiomeVolume * MasterEnvironmentalVolume * TimeVolumeMultiplier);
    }
}

void UAudio_EnvironmentalSoundManager::UpdateWeatherAudio()
{
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->SetVolumeMultiplier(CurrentWeatherIntensity * MasterEnvironmentalVolume);
        
        if (CurrentWeatherIntensity > 0.1f && !WeatherAudioComponent->IsPlaying())
        {
            // Start weather audio if intensity is significant
            WeatherAudioComponent->Play();
        }
        else if (CurrentWeatherIntensity <= 0.1f && WeatherAudioComponent->IsPlaying())
        {
            // Stop weather audio if intensity is too low
            WeatherAudioComponent->Stop();
        }
    }
}

UAudioComponent* UAudio_EnvironmentalSoundManager::CreateAudioComponent(const FString& ComponentName)
{
    if (UWorld* World = GetWorld())
    {
        UAudioComponent* AudioComp = NewObject<UAudioComponent>(this, *ComponentName);
        if (AudioComp)
        {
            AudioComp->bAutoActivate = false;
            AudioComp->bIsUISound = false;
            AudioComp->bAllowSpatialization = false; // Environmental sounds are non-spatial
            AudioComp->RegisterComponent();
            return AudioComp;
        }
    }
    
    return nullptr;
}