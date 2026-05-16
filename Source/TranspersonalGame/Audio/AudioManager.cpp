#include "AudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAudioManager::UAudioManager()
{
    CurrentBiome = EAudio_BiomeType::Savana;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;
    MusicVolume = 0.5f;
    AmbientAudioComponent = nullptr;
    MusicAudioComponent = nullptr;
}

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Initializing audio subsystem"));
    
    InitializeAudioConfigs();
    
    // Set up cleanup timer for temporary audio components
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UAudioManager::CleanupTemporaryAudioComponents,
            5.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Audio subsystem initialized successfully"));
}

void UAudioManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Deinitializing audio subsystem"));
    
    // Clean up audio components
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
        AmbientAudioComponent = nullptr;
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->Stop();
        MusicAudioComponent = nullptr;
    }
    
    // Clean up temporary components
    for (UAudioComponent* Component : TemporaryAudioComponents)
    {
        if (Component && IsValid(Component))
        {
            Component->Stop();
        }
    }
    TemporaryAudioComponents.Empty();
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    Super::Deinitialize();
}

void UAudioManager::InitializeAudioConfigs()
{
    // Initialize biome audio configurations
    FAudio_BiomeAudioConfig SavanaConfig;
    SavanaConfig.BaseVolume = 0.7f;
    SavanaConfig.FadeTime = 3.0f;
    BiomeAudioConfigs.Add(EAudio_BiomeType::Savana, SavanaConfig);
    
    FAudio_BiomeAudioConfig PantanoConfig;
    PantanoConfig.BaseVolume = 0.6f;
    PantanoConfig.FadeTime = 4.0f;
    BiomeAudioConfigs.Add(EAudio_BiomeType::Pantano, PantanoConfig);
    
    FAudio_BiomeAudioConfig FlorestaConfig;
    FlorestaConfig.BaseVolume = 0.8f;
    FlorestaConfig.FadeTime = 2.5f;
    BiomeAudioConfigs.Add(EAudio_BiomeType::Floresta, FlorestaConfig);
    
    FAudio_BiomeAudioConfig DesertoConfig;
    DesertoConfig.BaseVolume = 0.5f;
    DesertoConfig.FadeTime = 5.0f;
    BiomeAudioConfigs.Add(EAudio_BiomeType::Deserto, DesertoConfig);
    
    FAudio_BiomeAudioConfig MontanhaConfig;
    MontanhaConfig.BaseVolume = 0.6f;
    MontanhaConfig.FadeTime = 3.5f;
    BiomeAudioConfigs.Add(EAudio_BiomeType::Montanha, MontanhaConfig);
    
    // Initialize threat level audio configurations
    FAudio_ThreatAudioConfig SafeConfig;
    SafeConfig.Volume = 0.3f;
    SafeConfig.Priority = 0.2f;
    ThreatAudioConfigs.Add(EAudio_ThreatLevel::Safe, SafeConfig);
    
    FAudio_ThreatAudioConfig CautiousConfig;
    CautiousConfig.Volume = 0.5f;
    CautiousConfig.Priority = 0.5f;
    ThreatAudioConfigs.Add(EAudio_ThreatLevel::Cautious, CautiousConfig);
    
    FAudio_ThreatAudioConfig DangerConfig;
    DangerConfig.Volume = 0.7f;
    DangerConfig.Priority = 0.8f;
    ThreatAudioConfigs.Add(EAudio_ThreatLevel::Danger, DangerConfig);
    
    FAudio_ThreatAudioConfig CombatConfig;
    CombatConfig.Volume = 0.9f;
    CombatConfig.Priority = 1.0f;
    ThreatAudioConfigs.Add(EAudio_ThreatLevel::Combat, CombatConfig);
}

void UAudioManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Transitioning from biome %d to biome %d"), 
               (int32)CurrentBiome, (int32)NewBiome);
        
        EAudio_BiomeType PreviousBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        TransitionToNewBiome(NewBiome);
    }
}

void UAudioManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Threat level changed from %d to %d"), 
               (int32)CurrentThreatLevel, (int32)NewThreatLevel);
        
        CurrentThreatLevel = NewThreatLevel;
        TransitionToNewThreatLevel(NewThreatLevel);
    }
}

void UAudioManager::PlayDinosaurFootsteps(const FVector& Location, float Intensity)
{
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing dinosaur footsteps at location %s with intensity %f"), 
           *Location.ToString(), Intensity);
    
    // For now, log the footstep event - in a full implementation, this would play spatial audio
    // based on the dinosaur type and distance from player
    
    if (UWorld* World = GetWorld())
    {
        // Create a temporary audio component for the footstep sound
        // This is where we would load and play the actual footstep sound
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Footstep audio would play at world location %s"), 
               *Location.ToString());
    }
}

void UAudioManager::PlayEnvironmentalSound(const FString& SoundName, const FVector& Location)
{
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing environmental sound '%s' at location %s"), 
           *SoundName, *Location.ToString());
    
    // This would load and play environmental sounds like wind, water, etc.
    // For now, we log the event
}

void UAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Master volume set to %f"), MasterVolume);
    
    // Update all active audio components
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(MasterVolume * AmbientVolume);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MasterVolume * MusicVolume);
    }
}

void UAudioManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Ambient volume set to %f"), AmbientVolume);
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(MasterVolume * AmbientVolume);
    }
}

void UAudioManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Music volume set to %f"), MusicVolume);
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MasterVolume * MusicVolume);
    }
}

UAudioComponent* UAudioManager::CreateAudioComponent(USoundBase* Sound, const FVector& Location, bool bAutoDestroy)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Cannot create audio component - Sound is null"));
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Cannot create audio component - World is null"));
        return nullptr;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        World,
        Sound,
        Location,
        FRotator::ZeroRotator,
        MasterVolume,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        bAutoDestroy
    );
    
    if (AudioComp && !bAutoDestroy)
    {
        TemporaryAudioComponents.Add(AudioComp);
    }
    
    return AudioComp;
}

void UAudioManager::TransitionToNewBiome(EAudio_BiomeType NewBiome)
{
    if (BiomeAudioConfigs.Contains(NewBiome))
    {
        const FAudio_BiomeAudioConfig& Config = BiomeAudioConfigs[NewBiome];
        
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Starting biome transition with fade time %f"), Config.FadeTime);
        
        // Fade out current ambient sound
        if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->FadeOut(Config.FadeTime, 0.0f);
        }
        
        // In a full implementation, we would:
        // 1. Load the new biome's ambient sound
        // 2. Create new audio component
        // 3. Fade in the new sound
        // 4. Update spatial audio parameters
        
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Biome audio transition completed"));
    }
}

void UAudioManager::TransitionToNewThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (ThreatAudioConfigs.Contains(NewThreatLevel))
    {
        const FAudio_ThreatAudioConfig& Config = ThreatAudioConfigs[NewThreatLevel];
        
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Threat level transition - Volume: %f, Priority: %f"), 
               Config.Volume, Config.Priority);
        
        // Adjust music intensity based on threat level
        if (MusicAudioComponent)
        {
            float NewVolume = MasterVolume * MusicVolume * Config.Volume;
            MusicAudioComponent->SetVolumeMultiplier(NewVolume);
        }
        
        // In a full implementation, we would:
        // 1. Cross-fade between different music tracks
        // 2. Adjust ambient sound filtering
        // 3. Trigger threat-specific audio cues
        // 4. Modify spatial audio parameters for tension
    }
}

void UAudioManager::CleanupTemporaryAudioComponents()
{
    // Remove finished or invalid audio components
    for (int32 i = TemporaryAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* Component = TemporaryAudioComponents[i];
        if (!Component || !IsValid(Component) || !Component->IsPlaying())
        {
            TemporaryAudioComponents.RemoveAt(i);
        }
    }
}