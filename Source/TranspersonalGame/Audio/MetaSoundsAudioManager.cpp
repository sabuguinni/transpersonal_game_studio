#include "MetaSoundsAudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"

UMetaSoundsAudioManager::UMetaSoundsAudioManager()
{
    CurrentBiome = EAudio_BiomeType::Savana;
    CurrentMusicState = EAudio_MusicState::Exploration;
    CurrentIntensity = EAudio_IntensityLevel::Low;
    
    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;
    MusicVolume = 0.5f;
    SFXVolume = 0.8f;
    
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
    TransitionDuration = 0.0f;
    TransitionTargetBiome = EAudio_BiomeType::Savana;
}

void UMetaSoundsAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Initializing prehistoric survival audio system"));
    
    // Initialize biome and music configurations
    InitializeBiomeConfigs();
    InitializeMusicConfigs();
    
    // Create audio components
    if (UWorld* World = GetWorld())
    {
        // Ambient audio component
        AmbientAudioComponent = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass());
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->bAutoActivate = false;
            AmbientAudioComponent->VolumeMultiplier = AmbientVolume * MasterVolume;
            UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Ambient audio component created"));
        }
        
        // Music audio component
        MusicAudioComponent = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass());
        if (MusicAudioComponent)
        {
            MusicAudioComponent->bAutoActivate = false;
            MusicAudioComponent->VolumeMultiplier = MusicVolume * MasterVolume;
            UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Music audio component created"));
        }
        
        // Narrative audio component
        NarrativeAudioComponent = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass());
        if (NarrativeAudioComponent)
        {
            NarrativeAudioComponent->bAutoActivate = false;
            NarrativeAudioComponent->VolumeMultiplier = 1.0f * MasterVolume;
            UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Narrative audio component created"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MetaSoundsAudioManager: System initialized for prehistoric survival game"));
}

void UMetaSoundsAudioManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Shutting down audio system"));
    
    // Stop all audio components
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->Stop();
    }
    
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UMetaSoundsAudioManager::InitializeBiomeConfigs()
{
    // Savana biome configuration
    FAudio_BiomeAudioConfig SavanaConfig;
    SavanaConfig.BiomeType = EAudio_BiomeType::Savana;
    SavanaConfig.AmbientVolume = 0.6f;
    SavanaConfig.MusicVolume = 0.4f;
    SavanaConfig.FadeInTime = 2.0f;
    SavanaConfig.FadeOutTime = 1.5f;
    BiomeConfigs.Add(EAudio_BiomeType::Savana, SavanaConfig);
    
    // Forest biome configuration
    FAudio_BiomeAudioConfig ForestConfig;
    ForestConfig.BiomeType = EAudio_BiomeType::Forest;
    ForestConfig.AmbientVolume = 0.8f;
    ForestConfig.MusicVolume = 0.3f;
    ForestConfig.FadeInTime = 3.0f;
    ForestConfig.FadeOutTime = 2.0f;
    BiomeConfigs.Add(EAudio_BiomeType::Forest, ForestConfig);
    
    // Desert biome configuration
    FAudio_BiomeAudioConfig DesertConfig;
    DesertConfig.BiomeType = EAudio_BiomeType::Desert;
    DesertConfig.AmbientVolume = 0.5f;
    DesertConfig.MusicVolume = 0.6f;
    DesertConfig.FadeInTime = 1.5f;
    DesertConfig.FadeOutTime = 1.0f;
    BiomeConfigs.Add(EAudio_BiomeType::Desert, DesertConfig);
    
    // Swamp biome configuration
    FAudio_BiomeAudioConfig SwampConfig;
    SwampConfig.BiomeType = EAudio_BiomeType::Swamp;
    SwampConfig.AmbientVolume = 0.9f;
    SwampConfig.MusicVolume = 0.2f;
    SwampConfig.FadeInTime = 2.5f;
    SwampConfig.FadeOutTime = 2.0f;
    BiomeConfigs.Add(EAudio_BiomeType::Swamp, SwampConfig);
    
    // Mountain biome configuration
    FAudio_BiomeAudioConfig MountainConfig;
    MountainConfig.BiomeType = EAudio_BiomeType::Mountain;
    MountainConfig.AmbientVolume = 0.4f;
    MountainConfig.MusicVolume = 0.7f;
    MountainConfig.FadeInTime = 3.5f;
    MountainConfig.FadeOutTime = 2.5f;
    BiomeConfigs.Add(EAudio_BiomeType::Mountain, MountainConfig);
    
    UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Biome configurations initialized for 5 biomes"));
}

void UMetaSoundsAudioManager::InitializeMusicConfigs()
{
    // Exploration music configuration
    FAudio_DynamicMusicConfig ExplorationConfig;
    ExplorationConfig.MusicState = EAudio_MusicState::Exploration;
    ExplorationConfig.IntensityLevel = EAudio_IntensityLevel::Low;
    ExplorationConfig.CrossfadeTime = 4.0f;
    ExplorationConfig.bLoopTrack = true;
    MusicConfigs.Add(EAudio_MusicState::Exploration, ExplorationConfig);
    
    // Combat music configuration
    FAudio_DynamicMusicConfig CombatConfig;
    CombatConfig.MusicState = EAudio_MusicState::Combat;
    CombatConfig.IntensityLevel = EAudio_IntensityLevel::High;
    CombatConfig.CrossfadeTime = 1.0f;
    CombatConfig.bLoopTrack = true;
    MusicConfigs.Add(EAudio_MusicState::Combat, CombatConfig);
    
    // Stealth music configuration
    FAudio_DynamicMusicConfig StealthConfig;
    StealthConfig.MusicState = EAudio_MusicState::Stealth;
    StealthConfig.IntensityLevel = EAudio_IntensityLevel::Medium;
    StealthConfig.CrossfadeTime = 2.0f;
    StealthConfig.bLoopTrack = true;
    MusicConfigs.Add(EAudio_MusicState::Stealth, StealthConfig);
    
    // Discovery music configuration
    FAudio_DynamicMusicConfig DiscoveryConfig;
    DiscoveryConfig.MusicState = EAudio_MusicState::Discovery;
    DiscoveryConfig.IntensityLevel = EAudio_IntensityLevel::Medium;
    DiscoveryConfig.CrossfadeTime = 3.0f;
    DiscoveryConfig.bLoopTrack = false;
    MusicConfigs.Add(EAudio_MusicState::Discovery, DiscoveryConfig);
    
    // Survival music configuration
    FAudio_DynamicMusicConfig SurvivalConfig;
    SurvivalConfig.MusicState = EAudio_MusicState::Survival;
    SurvivalConfig.IntensityLevel = EAudio_IntensityLevel::Critical;
    SurvivalConfig.CrossfadeTime = 0.5f;
    SurvivalConfig.bLoopTrack = true;
    MusicConfigs.Add(EAudio_MusicState::Survival, SurvivalConfig);
    
    UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Dynamic music configurations initialized for 5 states"));
}

void UMetaSoundsAudioManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome == NewBiome)
    {
        return;
    }
    
    EAudio_BiomeType PreviousBiome = CurrentBiome;
    CurrentBiome = NewBiome;
    
    UE_LOG(LogTemp, Warning, TEXT("MetaSoundsAudioManager: Biome changed from %d to %d"), 
           (int32)PreviousBiome, (int32)NewBiome);
    
    // Update audio components with new biome settings
    UpdateAudioComponents();
}

void UMetaSoundsAudioManager::TransitionToBiome(EAudio_BiomeType TargetBiome, float TransitionTime)
{
    if (CurrentBiome == TargetBiome || bIsTransitioning)
    {
        return;
    }
    
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
    TransitionDuration = TransitionTime;
    TransitionTargetBiome = TargetBiome;
    
    UE_LOG(LogTemp, Warning, TEXT("MetaSoundsAudioManager: Starting biome transition to %d over %.1f seconds"), 
           (int32)TargetBiome, TransitionTime);
    
    // Begin transition
    HandleBiomeTransition(CurrentBiome, TargetBiome, TransitionTime);
}

void UMetaSoundsAudioManager::SetMusicState(EAudio_MusicState NewState, EAudio_IntensityLevel Intensity)
{
    if (CurrentMusicState == NewState && CurrentIntensity == Intensity)
    {
        return;
    }
    
    EAudio_MusicState PreviousState = CurrentMusicState;
    EAudio_IntensityLevel PreviousIntensity = CurrentIntensity;
    
    CurrentMusicState = NewState;
    CurrentIntensity = Intensity;
    
    UE_LOG(LogTemp, Warning, TEXT("MetaSoundsAudioManager: Music state changed from %d/%d to %d/%d"), 
           (int32)PreviousState, (int32)PreviousIntensity, (int32)NewState, (int32)Intensity);
    
    // Apply music configuration
    if (FAudio_DynamicMusicConfig* Config = MusicConfigs.Find(NewState))
    {
        if (MusicAudioComponent)
        {
            float VolumeMultiplier = MusicVolume * MasterVolume;
            
            // Adjust volume based on intensity
            switch (Intensity)
            {
                case EAudio_IntensityLevel::Low:
                    VolumeMultiplier *= 0.6f;
                    break;
                case EAudio_IntensityLevel::Medium:
                    VolumeMultiplier *= 0.8f;
                    break;
                case EAudio_IntensityLevel::High:
                    VolumeMultiplier *= 1.0f;
                    break;
                case EAudio_IntensityLevel::Critical:
                    VolumeMultiplier *= 1.2f;
                    break;
            }
            
            MusicAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
        }
    }
}

void UMetaSoundsAudioManager::CrossfadeToTrack(USoundCue* NewTrack, float CrossfadeTime)
{
    if (!NewTrack || !MusicAudioComponent)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Crossfading to new music track over %.1f seconds"), CrossfadeTime);
    
    // Simple crossfade implementation
    if (MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(CrossfadeTime, 0.0f);
    }
    
    // Set new track and fade in
    MusicAudioComponent->SetSound(NewTrack);
    MusicAudioComponent->FadeIn(CrossfadeTime, MusicVolume * MasterVolume);
}

void UMetaSoundsAudioManager::PlaySpatialSound(USoundCue* SoundCue, FVector Location, float VolumeMultiplier)
{
    if (!SoundCue)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    float FinalVolume = SFXVolume * MasterVolume * VolumeMultiplier;
    
    UGameplayStatics::PlaySoundAtLocation(
        World, 
        SoundCue, 
        Location, 
        FinalVolume, 
        1.0f, // Pitch
        0.0f, // Start time
        nullptr, // Attenuation settings (use sound's default)
        nullptr, // Concurrency settings
        nullptr  // Owner
    );
    
    UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Played spatial sound at location %s with volume %.2f"), 
           *Location.ToString(), FinalVolume);
}

void UMetaSoundsAudioManager::PlayDinosaurRoar(FVector Location, float IntensityMultiplier)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // This would typically load a dinosaur roar sound cue
    // For now, we log the action for system integration
    float FinalVolume = SFXVolume * MasterVolume * IntensityMultiplier;
    
    UE_LOG(LogTemp, Warning, TEXT("MetaSoundsAudioManager: Dinosaur roar triggered at %s with intensity %.2f"), 
           *Location.ToString(), IntensityMultiplier);
    
    // TODO: Load and play actual dinosaur roar sound cue
    // USoundCue* DinosaurRoar = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/SFX/DinosaurRoars/TRex_Roar"));
    // if (DinosaurRoar)
    // {
    //     PlaySpatialSound(DinosaurRoar, Location, IntensityMultiplier);
    // }
}

void UMetaSoundsAudioManager::PlayNarrativeClip(USoundWave* VoiceClip, float VolumeMultiplier)
{
    if (!VoiceClip || !NarrativeAudioComponent)
    {
        return;
    }
    
    // Stop any currently playing narrative audio
    if (NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }
    
    float FinalVolume = MasterVolume * VolumeMultiplier;
    
    NarrativeAudioComponent->SetSound(VoiceClip);
    NarrativeAudioComponent->SetVolumeMultiplier(FinalVolume);
    NarrativeAudioComponent->Play();
    
    UE_LOG(LogTemp, Warning, TEXT("MetaSoundsAudioManager: Playing narrative audio clip with volume %.2f"), FinalVolume);
}

void UMetaSoundsAudioManager::StopNarrativeAudio()
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
        UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Stopped narrative audio"));
    }
}

void UMetaSoundsAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UpdateAudioComponents();
    UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Master volume set to %.2f"), MasterVolume);
}

void UMetaSoundsAudioManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
    UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Ambient volume set to %.2f"), AmbientVolume);
}

void UMetaSoundsAudioManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: Music volume set to %.2f"), MusicVolume);
}

void UMetaSoundsAudioManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("MetaSoundsAudioManager: SFX volume set to %.2f"), SFXVolume);
}

void UMetaSoundsAudioManager::UpdateAudioComponents()
{
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetVolumeMultiplier(MasterVolume);
    }
}

void UMetaSoundsAudioManager::HandleBiomeTransition(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome, float TransitionTime)
{
    // Get biome configurations
    FAudio_BiomeAudioConfig* FromConfig = BiomeConfigs.Find(FromBiome);
    FAudio_BiomeAudioConfig* ToConfig = BiomeConfigs.Find(ToBiome);
    
    if (!FromConfig || !ToConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("MetaSoundsAudioManager: Invalid biome configuration for transition"));
        bIsTransitioning = false;
        return;
    }
    
    // Fade out current biome audio
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->FadeOut(FromConfig->FadeOutTime, 0.0f);
    }
    
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(FromConfig->FadeOutTime, 0.0f);
    }
    
    // TODO: Implement delayed fade-in for target biome
    // This would typically use a timer to delay the fade-in
    
    // For now, immediately set the new biome
    CurrentBiome = ToBiome;
    bIsTransitioning = false;
    
    UE_LOG(LogTemp, Warning, TEXT("MetaSoundsAudioManager: Biome transition completed to %d"), (int32)ToBiome);
}