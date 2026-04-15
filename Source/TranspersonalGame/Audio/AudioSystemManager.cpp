#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"

UAudioSystemManager::UAudioSystemManager()
{
    MasterVolume = 1.0f;
    
    // Initialize category volumes
    CategoryVolumes.Add(EAudio_SoundCategory::Environment, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Creatures, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Crafting, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Narration, 0.9f);
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing audio system for prehistoric survival game"));
    
    InitializeAudioComponents();
    InitializeSoundDatabase();
    InitializeBiomeAmbientSounds();
    InitializeDangerLevelMusic();
    
    // Set default music state
    CurrentMusicState.DangerLevel = EAudio_DangerLevel::Safe;
    CurrentMusicState.CurrentBiome = EEng_BiomeType::Forest;
    CurrentMusicState.IntensityLevel = 0.0f;
    CurrentMusicState.bInCombat = false;
    CurrentMusicState.bNearWater = false;
    CurrentMusicState.TimeOfDay = 12.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Audio system initialized successfully"));
}

void UAudioSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Shutting down audio system"));
    
    // Stop all audio components
    for (auto& ComponentPair : CategoryAudioComponents)
    {
        if (ComponentPair.Value && IsValid(ComponentPair.Value))
        {
            ComponentPair.Value->Stop();
        }
    }
    
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
    }
    
    if (AmbienceAudioComponent && IsValid(AmbienceAudioComponent))
    {
        AmbienceAudioComponent->Stop();
    }
    
    CategoryAudioComponents.Empty();
    SoundDatabase.Empty();
    BiomeAmbientSounds.Empty();
    DangerLevelMusic.Empty();
    
    Super::Deinitialize();
}

void UAudioSystemManager::InitializeAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: No world context available for audio components"));
        return;
    }
    
    // Create main music audio component
    MusicAudioComponent = NewObject<UAudioComponent>(this);
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(CategoryVolumes[EAudio_SoundCategory::Music]);
        MusicAudioComponent->bAutoActivate = false;
    }
    
    // Create ambience audio component
    AmbienceAudioComponent = NewObject<UAudioComponent>(this);
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(CategoryVolumes[EAudio_SoundCategory::Environment]);
        AmbienceAudioComponent->bAutoActivate = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Audio components initialized"));
}

void UAudioSystemManager::InitializeSoundDatabase()
{
    // Initialize basic sound database with placeholder entries
    // These would be populated with actual sound assets in a full implementation
    
    FAudio_SoundData WindSound;
    WindSound.Category = EAudio_SoundCategory::Environment;
    WindSound.Volume = 0.6f;
    WindSound.bLooping = true;
    WindSound.bIs3D = false;
    SoundDatabase.Add(TEXT("Wind_Forest"), WindSound);
    
    FAudio_SoundData FootstepsHeavy;
    FootstepsHeavy.Category = EAudio_SoundCategory::Creatures;
    FootstepsHeavy.Volume = 0.8f;
    FootstepsHeavy.AttenuationDistance = 2000.0f;
    SoundDatabase.Add(TEXT("Footsteps_Heavy"), FootstepsHeavy);
    
    FAudio_SoundData RoarTyrannosaurus;
    RoarTyrannosaurus.Category = EAudio_SoundCategory::Creatures;
    RoarTyrannosaurus.Volume = 1.0f;
    RoarTyrannosaurus.AttenuationDistance = 5000.0f;
    SoundDatabase.Add(TEXT("Roar_TRex"), RoarTyrannosaurus);
    
    FAudio_SoundData CraftingStone;
    CraftingStone.Category = EAudio_SoundCategory::Crafting;
    CraftingStone.Volume = 0.7f;
    CraftingStone.AttenuationDistance = 500.0f;
    SoundDatabase.Add(TEXT("Crafting_Stone"), CraftingStone);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Sound database initialized with %d entries"), SoundDatabase.Num());
}

void UAudioSystemManager::InitializeBiomeAmbientSounds()
{
    // Forest biome ambient sounds
    TArray<FAudio_SoundData> ForestSounds;
    
    FAudio_SoundData ForestWind;
    ForestWind.Category = EAudio_SoundCategory::Environment;
    ForestWind.Volume = 0.5f;
    ForestWind.bLooping = true;
    ForestWind.bIs3D = false;
    ForestSounds.Add(ForestWind);
    
    FAudio_SoundData BirdCalls;
    BirdCalls.Category = EAudio_SoundCategory::Environment;
    BirdCalls.Volume = 0.4f;
    BirdCalls.bLooping = true;
    BirdCalls.bIs3D = false;
    ForestSounds.Add(BirdCalls);
    
    BiomeAmbientSounds.Add(EEng_BiomeType::Forest, ForestSounds);
    
    // Grassland biome ambient sounds
    TArray<FAudio_SoundData> GrasslandSounds;
    
    FAudio_SoundData GrassWind;
    GrassWind.Category = EAudio_SoundCategory::Environment;
    GrassWind.Volume = 0.6f;
    GrassWind.bLooping = true;
    GrassWind.bIs3D = false;
    GrasslandSounds.Add(GrassWind);
    
    BiomeAmbientSounds.Add(EEng_BiomeType::Grassland, GrasslandSounds);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Biome ambient sounds initialized"));
}

void UAudioSystemManager::InitializeDangerLevelMusic()
{
    // Safe level music
    TArray<FAudio_SoundData> SafeMusic;
    
    FAudio_SoundData PeacefulAmbient;
    PeacefulAmbient.Category = EAudio_SoundCategory::Music;
    PeacefulAmbient.Volume = 0.4f;
    PeacefulAmbient.bLooping = true;
    PeacefulAmbient.bIs3D = false;
    SafeMusic.Add(PeacefulAmbient);
    
    DangerLevelMusic.Add(EAudio_DangerLevel::Safe, SafeMusic);
    
    // Dangerous level music
    TArray<FAudio_SoundData> DangerousMusic;
    
    FAudio_SoundData TenseMusic;
    TenseMusic.Category = EAudio_SoundCategory::Music;
    TenseMusic.Volume = 0.6f;
    TenseMusic.bLooping = true;
    TenseMusic.bIs3D = false;
    DangerousMusic.Add(TenseMusic);
    
    DangerLevelMusic.Add(EAudio_DangerLevel::Dangerous, DangerousMusic);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Danger level music initialized"));
}

void UAudioSystemManager::PlaySound3D(const FAudio_SoundData& SoundData, const FVector& Location, AActor* Instigator)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: No world context for 3D sound"));
        return;
    }
    
    if (!SoundData.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Invalid sound cue for 3D playback"));
        return;
    }
    
    float FinalVolume = SoundData.Volume * CategoryVolumes[SoundData.Category] * MasterVolume;
    
    UGameplayStatics::PlaySoundAtLocation(
        World,
        SoundData.SoundCue.Get(),
        Location,
        FinalVolume,
        SoundData.Pitch
    );
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing 3D sound at location %s"), *Location.ToString());
}

void UAudioSystemManager::PlaySound2D(const FAudio_SoundData& SoundData, AActor* Instigator)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: No world context for 2D sound"));
        return;
    }
    
    if (!SoundData.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Invalid sound cue for 2D playback"));
        return;
    }
    
    float FinalVolume = SoundData.Volume * CategoryVolumes[SoundData.Category] * MasterVolume;
    
    UGameplayStatics::PlaySound2D(
        World,
        SoundData.SoundCue.Get(),
        FinalVolume,
        SoundData.Pitch
    );
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing 2D sound"));
}

void UAudioSystemManager::StopAllSoundsOfCategory(EAudio_SoundCategory Category)
{
    UAudioComponent* Component = CategoryAudioComponents.FindRef(Category);
    if (Component && IsValid(Component))
    {
        Component->Stop();
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopped all sounds of category %d"), (int32)Category);
    }
}

void UAudioSystemManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes[Category] = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    UAudioComponent* Component = CategoryAudioComponents.FindRef(Category);
    if (Component && IsValid(Component))
    {
        Component->SetVolumeMultiplier(CategoryVolumes[Category] * MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set category %d volume to %f"), (int32)Category, Volume);
}

void UAudioSystemManager::UpdateEnvironmentalAudio(const FVector& PlayerLocation, EEng_BiomeType Biome)
{
    UpdateAmbienceForBiome(Biome, PlayerLocation);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Updated environmental audio for biome %d"), (int32)Biome);
}

void UAudioSystemManager::PlayWindSound(float WindIntensity, const FVector& Location)
{
    FAudio_SoundData* WindSoundData = SoundDatabase.Find(TEXT("Wind_Forest"));
    if (WindSoundData)
    {
        FAudio_SoundData ModifiedWindSound = *WindSoundData;
        ModifiedWindSound.Volume = FMath::Clamp(WindIntensity, 0.1f, 1.0f);
        ModifiedWindSound.Pitch = FMath::Lerp(0.8f, 1.2f, WindIntensity);
        
        PlaySound3D(ModifiedWindSound, Location);
    }
}

void UAudioSystemManager::PlayWaterSound(const FVector& Location, bool bIsRiver)
{
    // Implementation for water sounds
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing water sound at %s (River: %s)"), 
           *Location.ToString(), bIsRiver ? TEXT("Yes") : TEXT("No"));
}

void UAudioSystemManager::PlayCreatureFootsteps(const FVector& Location, float CreatureSize, float MovementSpeed)
{
    FAudio_SoundData* FootstepData = SoundDatabase.Find(TEXT("Footsteps_Heavy"));
    if (FootstepData)
    {
        FAudio_SoundData ModifiedFootsteps = *FootstepData;
        ModifiedFootsteps.Volume = FMath::Clamp(CreatureSize * 0.1f, 0.3f, 1.0f);
        ModifiedFootsteps.Pitch = FMath::Lerp(0.7f, 1.3f, MovementSpeed / 10.0f);
        
        PlaySound3D(ModifiedFootsteps, Location);
    }
}

void UAudioSystemManager::PlayCreatureRoar(const FVector& Location, float CreatureSize, EAudio_DangerLevel ThreatLevel)
{
    FAudio_SoundData* RoarData = SoundDatabase.Find(TEXT("Roar_TRex"));
    if (RoarData)
    {
        FAudio_SoundData ModifiedRoar = *RoarData;
        ModifiedRoar.Volume = FMath::Clamp(CreatureSize * 0.15f, 0.5f, 1.0f);
        ModifiedRoar.Pitch = FMath::Lerp(0.6f, 1.2f, (float)ThreatLevel / 4.0f);
        
        PlaySound3D(ModifiedRoar, Location);
        
        // Update danger level based on roar
        if (ThreatLevel >= EAudio_DangerLevel::Dangerous)
        {
            SetDangerLevel(ThreatLevel);
        }
    }
}

void UAudioSystemManager::PlayCreatureBreathing(const FVector& Location, float CreatureSize, bool bIsAggressive)
{
    // Implementation for creature breathing sounds
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing creature breathing at %s (Size: %f, Aggressive: %s)"), 
           *Location.ToString(), CreatureSize, bIsAggressive ? TEXT("Yes") : TEXT("No"));
}

void UAudioSystemManager::UpdateMusicState(const FAudio_MusicState& NewState)
{
    bool bStateChanged = false;
    
    if (CurrentMusicState.DangerLevel != NewState.DangerLevel)
    {
        bStateChanged = true;
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Danger level changed from %d to %d"), 
               (int32)CurrentMusicState.DangerLevel, (int32)NewState.DangerLevel);
    }
    
    if (CurrentMusicState.CurrentBiome != NewState.CurrentBiome)
    {
        bStateChanged = true;
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Biome changed from %d to %d"), 
               (int32)CurrentMusicState.CurrentBiome, (int32)NewState.CurrentBiome);
    }
    
    CurrentMusicState = NewState;
    
    if (bStateChanged)
    {
        // Trigger music transition based on new state
        FString MusicTrackName = FString::Printf(TEXT("Music_%s_%s"), 
                                                *UEnum::GetValueAsString(NewState.CurrentBiome),
                                                *UEnum::GetValueAsString(NewState.DangerLevel));
        TransitionToMusic(MusicTrackName, 3.0f);
    }
}

void UAudioSystemManager::SetDangerLevel(EAudio_DangerLevel NewDangerLevel)
{
    if (CurrentMusicState.DangerLevel != NewDangerLevel)
    {
        CurrentMusicState.DangerLevel = NewDangerLevel;
        
        // Find appropriate music for this danger level
        TArray<FAudio_SoundData>* DangerMusic = DangerLevelMusic.Find(NewDangerLevel);
        if (DangerMusic && DangerMusic->Num() > 0)
        {
            // For now, just log the transition
            UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Transitioning to danger level %d music"), (int32)NewDangerLevel);
        }
    }
}

void UAudioSystemManager::TransitionToMusic(const FString& MusicTrackName, float FadeTime)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transitioning to music track '%s' with fade time %f"), 
           *MusicTrackName, FadeTime);
    
    // Implementation would handle crossfading between music tracks
    // For now, just log the transition
}

void UAudioSystemManager::PlayWeaponSound(const FVector& Location, const FString& WeaponType)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing weapon sound '%s' at %s"), 
           *WeaponType, *Location.ToString());
}

void UAudioSystemManager::PlayImpactSound(const FVector& Location, const FString& MaterialType)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing impact sound on '%s' at %s"), 
           *MaterialType, *Location.ToString());
}

void UAudioSystemManager::PlayCraftingSound(const FVector& Location, const FString& CraftingAction)
{
    FAudio_SoundData* CraftingData = SoundDatabase.Find(TEXT("Crafting_Stone"));
    if (CraftingData)
    {
        PlaySound3D(*CraftingData, Location);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing crafting sound '%s' at %s"), 
               *CraftingAction, *Location.ToString());
    }
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all audio components with new master volume
    for (auto& ComponentPair : CategoryAudioComponents)
    {
        if (ComponentPair.Value && IsValid(ComponentPair.Value))
        {
            ComponentPair.Value->SetVolumeMultiplier(CategoryVolumes[ComponentPair.Key] * MasterVolume);
        }
    }
    
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->SetVolumeMultiplier(CategoryVolumes[EAudio_SoundCategory::Music] * MasterVolume);
    }
    
    if (AmbienceAudioComponent && IsValid(AmbienceAudioComponent))
    {
        AmbienceAudioComponent->SetVolumeMultiplier(CategoryVolumes[EAudio_SoundCategory::Environment] * MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set master volume to %f"), Volume);
}

void UAudioSystemManager::SetSFXVolume(float Volume)
{
    SetCategoryVolume(EAudio_SoundCategory::Environment, Volume);
    SetCategoryVolume(EAudio_SoundCategory::Creatures, Volume);
    SetCategoryVolume(EAudio_SoundCategory::Combat, Volume);
    SetCategoryVolume(EAudio_SoundCategory::Crafting, Volume);
}

void UAudioSystemManager::SetMusicVolume(float Volume)
{
    SetCategoryVolume(EAudio_SoundCategory::Music, Volume);
}

void UAudioSystemManager::SetVoiceVolume(float Volume)
{
    SetCategoryVolume(EAudio_SoundCategory::Narration, Volume);
}

UAudioComponent* UAudioSystemManager::GetOrCreateAudioComponent(EAudio_SoundCategory Category)
{
    UAudioComponent* Component = CategoryAudioComponents.FindRef(Category);
    if (!Component || !IsValid(Component))
    {
        Component = NewObject<UAudioComponent>(this);
        if (Component)
        {
            Component->SetVolumeMultiplier(CategoryVolumes[Category] * MasterVolume);
            Component->bAutoActivate = false;
            CategoryAudioComponents.Add(Category, Component);
        }
    }
    return Component;
}

void UAudioSystemManager::UpdateAmbienceForBiome(EEng_BiomeType Biome, const FVector& Location)
{
    TArray<FAudio_SoundData>* BiomeSounds = BiomeAmbientSounds.Find(Biome);
    if (BiomeSounds && BiomeSounds->Num() > 0)
    {
        // Play the first ambient sound for this biome
        const FAudio_SoundData& AmbientSound = (*BiomeSounds)[0];
        
        if (AmbienceAudioComponent && IsValid(AmbienceAudioComponent))
        {
            // Stop current ambience
            AmbienceAudioComponent->Stop();
            
            // Start new ambience (would need actual sound cue)
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Updated ambience for biome %d"), (int32)Biome);
        }
    }
}

void UAudioSystemManager::CrossfadeMusic(USoundCue* NewMusic, float FadeTime)
{
    if (!NewMusic || !MusicAudioComponent || !IsValid(MusicAudioComponent))
    {
        return;
    }
    
    // Implementation would handle smooth crossfading between music tracks
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Crossfading to new music over %f seconds"), FadeTime);
}