#include "AudioManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"

UAudioManager::UAudioManager()
{
    CurrentBiome = EAudio_BiomeType::Savana;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    MasterVolume = 1.0f;
    AmbientVolume = 0.8f;
    EffectsVolume = 1.0f;
    bAdaptiveMusicActive = false;
}

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Initializing audio subsystem"));
    
    // Initialize biome and dinosaur settings
    InitializeBiomeSettings();
    InitializeDinosaurProfiles();
    
    // Create audio components
    if (UWorld* World = GetWorld())
    {
        // Create ambient audio component
        AmbientAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
            World, 
            nullptr, 
            FVector::ZeroVector, 
            FRotator::ZeroRotator, 
            0.0f, 
            1.0f, 
            0.0f, 
            nullptr, 
            nullptr, 
            true
        );
        
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->bAutoDestroy = false;
            UE_LOG(LogTemp, Warning, TEXT("AudioManager: Ambient audio component created"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Initialization complete"));
}

void UAudioManager::Deinitialize()
{
    if (bAdaptiveMusicActive)
    {
        StopAdaptiveMusic();
    }
    
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
    }
    
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
    }
    
    if (NarrativeAudioComponent && IsValid(NarrativeAudioComponent))
    {
        NarrativeAudioComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UAudioManager::InitializeBiomeSettings()
{
    // Initialize biome audio settings with default values
    FAudio_BiomeAudioSettings SavanaSettings;
    SavanaSettings.BaseVolume = 0.7f;
    SavanaSettings.FadeInTime = 2.0f;
    SavanaSettings.FadeOutTime = 1.5f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Savana, SavanaSettings);
    
    FAudio_BiomeAudioSettings ForestSettings;
    ForestSettings.BaseVolume = 0.8f;
    ForestSettings.FadeInTime = 3.0f;
    ForestSettings.FadeOutTime = 2.0f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Forest, ForestSettings);
    
    FAudio_BiomeAudioSettings SwampSettings;
    SwampSettings.BaseVolume = 0.6f;
    SwampSettings.FadeInTime = 2.5f;
    SwampSettings.FadeOutTime = 2.0f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Swamp, SwampSettings);
    
    FAudio_BiomeAudioSettings DesertSettings;
    DesertSettings.BaseVolume = 0.5f;
    DesertSettings.FadeInTime = 1.5f;
    DesertSettings.FadeOutTime = 1.0f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Desert, DesertSettings);
    
    FAudio_BiomeAudioSettings MountainSettings;
    MountainSettings.BaseVolume = 0.9f;
    MountainSettings.FadeInTime = 2.0f;
    MountainSettings.FadeOutTime = 1.5f;
    BiomeAudioSettings.Add(EAudio_BiomeType::Mountain, MountainSettings);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Biome settings initialized"));
}

void UAudioManager::InitializeDinosaurProfiles()
{
    // T-Rex audio profile
    FAudio_DinosaurSoundProfile TRexProfile;
    TRexProfile.FootstepVolume = 1.5f;
    TRexProfile.RoarVolume = 2.0f;
    TRexProfile.MaxHearingDistance = 8000.0f;
    DinosaurSoundProfiles.Add(TEXT("TRex"), TRexProfile);
    
    // Velociraptor audio profile
    FAudio_DinosaurSoundProfile RaptorProfile;
    RaptorProfile.FootstepVolume = 0.3f;
    RaptorProfile.RoarVolume = 0.8f;
    RaptorProfile.MaxHearingDistance = 3000.0f;
    DinosaurSoundProfiles.Add(TEXT("Velociraptor"), RaptorProfile);
    
    // Brachiosaurus audio profile
    FAudio_DinosaurSoundProfile BrachioProfile;
    BrachioProfile.FootstepVolume = 1.8f;
    BrachioProfile.RoarVolume = 1.5f;
    BrachioProfile.MaxHearingDistance = 10000.0f;
    DinosaurSoundProfiles.Add(TEXT("Brachiosaurus"), BrachioProfile);
    
    // Triceratops audio profile
    FAudio_DinosaurSoundProfile TriceratopsProfile;
    TriceratopsProfile.FootstepVolume = 1.2f;
    TriceratopsProfile.RoarVolume = 1.3f;
    TriceratopsProfile.MaxHearingDistance = 6000.0f;
    DinosaurSoundProfiles.Add(TEXT("Triceratops"), TriceratopsProfile);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Dinosaur sound profiles initialized"));
}

void UAudioManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Transitioning from biome %d to %d"), 
            (int32)CurrentBiome, (int32)NewBiome);
        
        EAudio_BiomeType OldBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        
        TransitionBiomeAudio(NewBiome);
    }
}

void UAudioManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Threat level changed to %d"), (int32)NewThreatLevel);
        CurrentThreatLevel = NewThreatLevel;
        UpdateMusicIntensity();
    }
}

void UAudioManager::TransitionBiomeAudio(EAudio_BiomeType NewBiome)
{
    if (!BiomeAudioSettings.Contains(NewBiome))
    {
        UE_LOG(LogTemp, Error, TEXT("AudioManager: No audio settings found for biome %d"), (int32)NewBiome);
        return;
    }
    
    const FAudio_BiomeAudioSettings& Settings = BiomeAudioSettings[NewBiome];
    
    // Fade out current ambient sound
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->FadeOut(Settings.FadeOutTime, 0.0f);
    }
    
    // TODO: Load and fade in new biome ambient sound
    // This would require actual sound assets to be loaded
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Biome audio transition complete"));
}

void UAudioManager::PlayDinosaurFootstep(const FString& DinosaurType, const FVector& Location)
{
    if (!DinosaurSoundProfiles.Contains(DinosaurType))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: No sound profile found for dinosaur type: %s"), *DinosaurType);
        return;
    }
    
    const FAudio_DinosaurSoundProfile& Profile = DinosaurSoundProfiles[DinosaurType];
    
    // Calculate distance-based volume
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
            float VolumeMultiplier = FMath::Clamp(1.0f - (Distance / Profile.MaxHearingDistance), 0.0f, 1.0f);
            float FinalVolume = Profile.FootstepVolume * VolumeMultiplier * EffectsVolume * MasterVolume;
            
            if (FinalVolume > 0.01f)
            {
                // TODO: Play actual footstep sound at location
                UE_LOG(LogTemp, Warning, TEXT("AudioManager: Playing %s footstep at volume %.2f"), 
                    *DinosaurType, FinalVolume);
            }
        }
    }
}

void UAudioManager::PlayDinosaurRoar(const FString& DinosaurType, const FVector& Location)
{
    if (!DinosaurSoundProfiles.Contains(DinosaurType))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: No sound profile found for dinosaur type: %s"), *DinosaurType);
        return;
    }
    
    const FAudio_DinosaurSoundProfile& Profile = DinosaurSoundProfiles[DinosaurType];
    
    // Calculate distance-based volume
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
            float VolumeMultiplier = FMath::Clamp(1.0f - (Distance / Profile.MaxHearingDistance), 0.0f, 1.0f);
            float FinalVolume = Profile.RoarVolume * VolumeMultiplier * EffectsVolume * MasterVolume;
            
            if (FinalVolume > 0.01f)
            {
                // TODO: Play actual roar sound at location
                UE_LOG(LogTemp, Warning, TEXT("AudioManager: Playing %s roar at volume %.2f"), 
                    *DinosaurType, FinalVolume);
                
                // Increase threat level temporarily
                if (CurrentThreatLevel < EAudio_ThreatLevel::Danger)
                {
                    SetThreatLevel(EAudio_ThreatLevel::Danger);
                }
            }
        }
    }
}

void UAudioManager::TriggerEnvironmentalSound(const FString& SoundName, const FVector& Location, float Volume)
{
    if (UWorld* World = GetWorld())
    {
        float FinalVolume = Volume * EffectsVolume * MasterVolume;
        
        // TODO: Load and play environmental sound
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Playing environmental sound '%s' at volume %.2f"), 
            *SoundName, FinalVolume);
    }
}

void UAudioManager::PlayNarrativeClip(const FString& ClipName)
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }
    
    // TODO: Load and play narrative audio clip
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Playing narrative clip '%s'"), *ClipName);
}

void UAudioManager::StopNarrativeClip()
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Stopped narrative clip"));
    }
}

void UAudioManager::StartAdaptiveMusic()
{
    if (!bAdaptiveMusicActive)
    {
        bAdaptiveMusicActive = true;
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                MusicUpdateTimer, 
                this, 
                &UAudioManager::UpdateMusicIntensity, 
                2.0f, 
                true
            );
        }
        
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Adaptive music started"));
    }
}

void UAudioManager::StopAdaptiveMusic()
{
    if (bAdaptiveMusicActive)
    {
        bAdaptiveMusicActive = false;
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(MusicUpdateTimer);
        }
        
        if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
        {
            MusicAudioComponent->FadeOut(2.0f, 0.0f);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Adaptive music stopped"));
    }
}

void UAudioManager::UpdateMusicIntensity()
{
    if (!bAdaptiveMusicActive)
        return;
    
    // Calculate music intensity based on threat level and biome
    float Intensity = 0.5f; // Base intensity
    
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:
            Intensity = 0.3f;
            break;
        case EAudio_ThreatLevel::Caution:
            Intensity = 0.6f;
            break;
        case EAudio_ThreatLevel::Danger:
            Intensity = 0.9f;
            break;
        case EAudio_ThreatLevel::Extreme:
            Intensity = 1.2f;
            break;
    }
    
    // Adjust for biome
    switch (CurrentBiome)
    {
        case EAudio_BiomeType::Forest:
            Intensity *= 1.1f; // Forest is slightly more intense
            break;
        case EAudio_BiomeType::Swamp:
            Intensity *= 1.2f; // Swamp is more dangerous
            break;
        case EAudio_BiomeType::Desert:
            Intensity *= 0.8f; // Desert is more sparse
            break;
        case EAudio_BiomeType::Mountain:
            Intensity *= 1.3f; // Mountains are treacherous
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Music intensity updated to %.2f"), Intensity);
}

void UAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Master volume set to %.2f"), MasterVolume);
}

void UAudioManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Ambient volume set to %.2f"), AmbientVolume);
}

void UAudioManager::SetEffectsVolume(float Volume)
{
    EffectsVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Effects volume set to %.2f"), EffectsVolume);
}