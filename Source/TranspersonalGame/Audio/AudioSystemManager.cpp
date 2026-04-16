#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"

UAudioSystemManager::UAudioSystemManager()
{
    CurrentMusicState = EAudio_MusicState::Exploration;
    CurrentBiome = EEng_BiomeType::Grassland;
    CurrentTimeOfDay = 12.0f; // Noon
    MasterVolume = 1.0f;
    MusicComponent = nullptr;
    AmbienceComponent = nullptr;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initializing audio system"));
    
    InitializeVolumeSettings();
    LoadAudioAssets();
    
    // Create persistent audio components
    MusicComponent = CreateAudioComponent();
    AmbienceComponent = CreateAudioComponent();
    
    if (MusicComponent)
    {
        MusicComponent->bAutoDestroy = false;
        MusicComponent->SetVolumeMultiplier(GetCategoryVolume(EAudio_SoundCategory::Music));
    }
    
    if (AmbienceComponent)
    {
        AmbienceComponent->bAutoDestroy = false;
        AmbienceComponent->SetVolumeMultiplier(GetCategoryVolume(EAudio_SoundCategory::Ambience));
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Audio system initialized successfully"));
}

void UAudioSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Shutting down audio system"));
    
    StopAllSounds();
    
    if (MusicComponent && IsValid(MusicComponent))
    {
        MusicComponent->Stop();
        MusicComponent->DestroyComponent();
        MusicComponent = nullptr;
    }
    
    if (AmbienceComponent && IsValid(AmbienceComponent))
    {
        AmbienceComponent->Stop();
        AmbienceComponent->DestroyComponent();
        AmbienceComponent = nullptr;
    }
    
    // Clear all active sounds
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value && IsValid(SoundPair.Value))
        {
            SoundPair.Value->Stop();
            SoundPair.Value->DestroyComponent();
        }
    }
    ActiveSounds.Empty();
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeVolumeSettings()
{
    // Set default category volumes
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Ambience, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Footsteps, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dinosaurs, 1.0f);
}

void UAudioSystemManager::LoadAudioAssets()
{
    // Load music tracks (placeholder paths - replace with actual assets)
    // MusicTracks.Add(EAudio_MusicState::Exploration, LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Music/Exploration")));
    // MusicTracks.Add(EAudio_MusicState::Combat, LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Music/Combat")));
    // MusicTracks.Add(EAudio_MusicState::Danger, LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Music/Danger")));
    
    // Load ambience tracks
    // AmbienceTracks.Add(EEng_BiomeType::Forest, LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Ambience/Forest")));
    // AmbienceTracks.Add(EEng_BiomeType::Grassland, LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Ambience/Grassland")));
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Audio assets loading initiated"));
}

UAudioComponent* UAudioSystemManager::CreateAudioComponent()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No valid world found"));
        return nullptr;
    }
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
    if (AudioComp)
    {
        AudioComp->bAutoDestroy = true;
        AudioComp->bStopWhenOwnerDestroyed = false;
        AudioComp->SetWorldLocation(FVector::ZeroVector);
    }
    
    return AudioComp;
}

void UAudioSystemManager::PlaySound2D(USoundBase* Sound, const FAudio_SoundConfig& Config)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Attempted to play null sound"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    float FinalVolume = Config.Volume * GetCategoryVolume(Config.Category) * MasterVolume;
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(
        World,
        Sound,
        FinalVolume,
        Config.Pitch,
        0.0f, // Start time
        nullptr, // Concurrency settings
        false, // Persist across level transitions
        true   // Auto destroy
    );
    
    if (AudioComp && Config.bLooping)
    {
        AudioComp->SetUISound(true);
        ActiveSounds.Add(Config.Category, AudioComp);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing 2D sound with volume %.2f"), FinalVolume);
}

void UAudioSystemManager::PlaySound3D(USoundBase* Sound, const FVector& Location, const FAudio_SoundConfig& Config)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Attempted to play null 3D sound"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    float FinalVolume = Config.Volume * GetCategoryVolume(Config.Category) * MasterVolume;
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        World,
        Sound,
        Location,
        FRotator::ZeroRotator,
        FinalVolume,
        Config.Pitch,
        0.0f, // Start time
        Config.AttenuationDistance,
        nullptr, // Concurrency settings
        nullptr, // Owner
        true     // Auto destroy
    );
    
    if (AudioComp && Config.bLooping)
    {
        ActiveSounds.Add(Config.Category, AudioComp);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing 3D sound at location (%s) with volume %.2f"), 
           *Location.ToString(), FinalVolume);
}

void UAudioSystemManager::StopSound(EAudio_SoundCategory Category)
{
    if (UAudioComponent** FoundSound = ActiveSounds.Find(Category))
    {
        if (*FoundSound && IsValid(*FoundSound))
        {
            (*FoundSound)->Stop();
            (*FoundSound)->DestroyComponent();
        }
        ActiveSounds.Remove(Category);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopped sounds in category %d"), (int32)Category);
}

void UAudioSystemManager::StopAllSounds()
{
    for (auto& SoundPair : ActiveSounds)
    {
        if (SoundPair.Value && IsValid(SoundPair.Value))
        {
            SoundPair.Value->Stop();
            SoundPair.Value->DestroyComponent();
        }
    }
    ActiveSounds.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopped all active sounds"));
}

void UAudioSystemManager::SetMusicState(EAudio_MusicState NewState)
{
    if (CurrentMusicState == NewState)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transitioning music from %d to %d"), 
           (int32)CurrentMusicState, (int32)NewState);
    
    TransitionMusic(NewState);
    CurrentMusicState = NewState;
}

void UAudioSystemManager::TransitionMusic(EAudio_MusicState NewState)
{
    if (!MusicComponent)
    {
        return;
    }
    
    // Stop current music
    if (MusicComponent->IsPlaying())
    {
        MusicComponent->FadeOut(2.0f, 0.0f);
    }
    
    // Find and play new music
    if (USoundBase** NewTrack = MusicTracks.Find(NewState))
    {
        if (*NewTrack)
        {
            MusicComponent->SetSound(*NewTrack);
            MusicComponent->FadeIn(2.0f, GetCategoryVolume(EAudio_SoundCategory::Music) * MasterVolume);
        }
    }
}

void UAudioSystemManager::SetMusicVolume(float Volume)
{
    SetCategoryVolume(EAudio_SoundCategory::Music, FMath::Clamp(Volume, 0.0f, 1.0f));
    
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        MusicComponent->SetVolumeMultiplier(GetCategoryVolume(EAudio_SoundCategory::Music) * MasterVolume);
    }
}

void UAudioSystemManager::FadeOutMusic(float FadeTime)
{
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        MusicComponent->FadeOut(FadeTime, 0.0f);
    }
}

void UAudioSystemManager::FadeInMusic(float FadeTime)
{
    if (MusicComponent)
    {
        MusicComponent->FadeIn(FadeTime, GetCategoryVolume(EAudio_SoundCategory::Music) * MasterVolume);
    }
}

void UAudioSystemManager::SetAmbienceState(EEng_BiomeType BiomeType)
{
    if (CurrentBiome == BiomeType)
    {
        return;
    }
    
    CurrentBiome = BiomeType;
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Setting ambience to biome %d"), (int32)BiomeType);
    
    if (!AmbienceComponent)
    {
        return;
    }
    
    // Fade out current ambience
    if (AmbienceComponent->IsPlaying())
    {
        AmbienceComponent->FadeOut(3.0f, 0.0f);
    }
    
    // Find and play new ambience
    if (USoundBase** NewAmbience = AmbienceTracks.Find(BiomeType))
    {
        if (*NewAmbience)
        {
            AmbienceComponent->SetSound(*NewAmbience);
            AmbienceComponent->FadeIn(3.0f, GetCategoryVolume(EAudio_SoundCategory::Ambience) * MasterVolume);
        }
    }
}

void UAudioSystemManager::UpdateTimeOfDay(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Fmod(TimeOfDay, 24.0f);
    
    // Adjust ambience volume based on time of day
    float AmbienceMultiplier = 1.0f;
    
    // Quieter during night (20:00 - 06:00)
    if (CurrentTimeOfDay >= 20.0f || CurrentTimeOfDay <= 6.0f)
    {
        AmbienceMultiplier = 0.7f;
    }
    
    if (AmbienceComponent)
    {
        float NewVolume = GetCategoryVolume(EAudio_SoundCategory::Ambience) * AmbienceMultiplier * MasterVolume;
        AmbienceComponent->SetVolumeMultiplier(NewVolume);
    }
}

void UAudioSystemManager::PlayDinosaurRoar(const FVector& Location, float Intensity)
{
    if (DinosaurRoars.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No dinosaur roar sounds loaded"));
        return;
    }
    
    // Select random roar
    int32 RandomIndex = FMath::RandRange(0, DinosaurRoars.Num() - 1);
    USoundBase* RoarSound = DinosaurRoars[RandomIndex];
    
    FAudio_SoundConfig Config;
    Config.Category = EAudio_SoundCategory::Dinosaurs;
    Config.Volume = FMath::Clamp(Intensity, 0.1f, 2.0f);
    Config.AttenuationDistance = 5000.0f; // Very far for roars
    Config.Priority = EAudio_Priority::High;
    
    PlaySound3D(RoarSound, Location, Config);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing dinosaur roar at intensity %.2f"), Intensity);
}

void UAudioSystemManager::PlayDinosaurFootsteps(const FVector& Location, float Weight)
{
    if (DinosaurFootsteps.Num() == 0)
    {
        return;
    }
    
    int32 RandomIndex = FMath::RandRange(0, DinosaurFootsteps.Num() - 1);
    USoundBase* FootstepSound = DinosaurFootsteps[RandomIndex];
    
    FAudio_SoundConfig Config;
    Config.Category = EAudio_SoundCategory::Footsteps;
    Config.Volume = FMath::Clamp(Weight, 0.1f, 2.0f);
    Config.Pitch = FMath::RandRange(0.9f, 1.1f); // Slight pitch variation
    Config.AttenuationDistance = 2000.0f;
    
    PlaySound3D(FootstepSound, Location, Config);
}

void UAudioSystemManager::PlayPlayerFootstep(const FVector& Location, const FString& SurfaceType)
{
    if (PlayerFootsteps.Num() == 0)
    {
        return;
    }
    
    // Select footstep based on surface type (simplified for now)
    int32 SoundIndex = 0;
    if (SurfaceType.Contains(TEXT("Grass")))
    {
        SoundIndex = 0;
    }
    else if (SurfaceType.Contains(TEXT("Stone")))
    {
        SoundIndex = FMath::Min(1, PlayerFootsteps.Num() - 1);
    }
    else if (SurfaceType.Contains(TEXT("Water")))
    {
        SoundIndex = FMath::Min(2, PlayerFootsteps.Num() - 1);
    }
    
    USoundBase* FootstepSound = PlayerFootsteps[SoundIndex];
    
    FAudio_SoundConfig Config;
    Config.Category = EAudio_SoundCategory::Footsteps;
    Config.Volume = 0.6f;
    Config.Pitch = FMath::RandRange(0.95f, 1.05f);
    Config.AttenuationDistance = 500.0f;
    
    PlaySound3D(FootstepSound, Location, Config);
}

void UAudioSystemManager::PlayPlayerHeartbeat(float Intensity)
{
    // Create heartbeat sound procedurally or use pre-recorded
    FAudio_SoundConfig Config;
    Config.Category = EAudio_SoundCategory::SFX;
    Config.Volume = FMath::Clamp(Intensity, 0.1f, 1.0f);
    Config.bLooping = true;
    Config.b3D = false; // 2D UI sound
    
    // This would need a heartbeat sound asset
    // PlaySound2D(HeartbeatSound, Config);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing heartbeat at intensity %.2f"), Intensity);
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active sounds
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        MusicComponent->SetVolumeMultiplier(GetCategoryVolume(EAudio_SoundCategory::Music) * MasterVolume);
    }
    
    if (AmbienceComponent && AmbienceComponent->IsPlaying())
    {
        AmbienceComponent->SetVolumeMultiplier(GetCategoryVolume(EAudio_SoundCategory::Ambience) * MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Master volume set to %.2f"), MasterVolume);
}

void UAudioSystemManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Category %d volume set to %.2f"), (int32)Category, Volume);
}

float UAudioSystemManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    if (const float* Volume = CategoryVolumes.Find(Category))
    {
        return *Volume;
    }
    return 1.0f; // Default volume
}

void UAudioSystemManager::UpdateAudioOcclusion(const FVector& ListenerLocation, const FVector& SoundLocation)
{
    // Perform line trace to check for occlusion
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = false;
    
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        ListenerLocation,
        SoundLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        // Apply occlusion effect (reduce volume/apply low-pass filter)
        float OcclusionFactor = 0.3f; // 70% volume reduction when occluded
        // This would need to be applied to specific audio components
        UE_LOG(LogTemp, VeryVerbose, TEXT("AudioSystemManager: Audio occlusion detected, factor %.2f"), OcclusionFactor);
    }
}

void UAudioSystemManager::SetReverbZone(const FString& ReverbPreset)
{
    // Apply reverb settings based on environment
    // This would integrate with UE5's audio reverb system
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Setting reverb zone to %s"), *ReverbPreset);
}

void UAudioSystemManager::CleanupFinishedSounds()
{
    TArray<EAudio_SoundCategory> CategoriesToRemove;
    
    for (auto& SoundPair : ActiveSounds)
    {
        if (!SoundPair.Value || !IsValid(SoundPair.Value) || !SoundPair.Value->IsPlaying())
        {
            if (SoundPair.Value && IsValid(SoundPair.Value))
            {
                SoundPair.Value->DestroyComponent();
            }
            CategoriesToRemove.Add(SoundPair.Key);
        }
    }
    
    for (EAudio_SoundCategory Category : CategoriesToRemove)
    {
        ActiveSounds.Remove(Category);
    }
}