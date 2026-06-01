#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    MasterVolume = 1.0f;
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentAmbienceComponent = nullptr;
    CurrentMusicComponent = nullptr;
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Initializing sound system"));
    
    InitializeCategoryVolumes();
    InitializeDefaultSounds();
    InitializeBiomeAmbiences();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sound system initialized with %d registered sounds"), RegisteredSounds.Num());
}

void UAudio_SoundManager::Deinitialize()
{
    StopAllSounds();
    
    if (CurrentAmbienceComponent && IsValid(CurrentAmbienceComponent))
    {
        CurrentAmbienceComponent->Stop();
        CurrentAmbienceComponent = nullptr;
    }
    
    if (CurrentMusicComponent && IsValid(CurrentMusicComponent))
    {
        CurrentMusicComponent->Stop();
        CurrentMusicComponent = nullptr;
    }
    
    RegisteredSounds.Empty();
    BiomeAmbiences.Empty();
    CategoryVolumes.Empty();
    ActiveAudioComponents.Empty();
    
    Super::Deinitialize();
}

void UAudio_SoundManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.5f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dinosaur, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Environment, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 0.8f);
}

void UAudio_SoundManager::InitializeDefaultSounds()
{
    // Forest ambience
    FAudio_SoundEntry ForestAmbience;
    ForestAmbience.SoundID = TEXT("forest_ambience");
    ForestAmbience.Category = EAudio_SoundCategory::Ambient;
    ForestAmbience.Volume = 0.6f;
    ForestAmbience.bLoop = true;
    ForestAmbience.FadeInTime = 3.0f;
    ForestAmbience.FadeOutTime = 3.0f;
    RegisteredSounds.Add(ForestAmbience.SoundID, ForestAmbience);
    
    // T-Rex roar
    FAudio_SoundEntry TRexRoar;
    TRexRoar.SoundID = TEXT("trex_roar");
    TRexRoar.Category = EAudio_SoundCategory::Dinosaur;
    TRexRoar.Volume = 1.0f;
    TRexRoar.Pitch = 0.8f;
    TRexRoar.bLoop = false;
    RegisteredSounds.Add(TRexRoar.SoundID, TRexRoar);
    
    // Heavy footsteps
    FAudio_SoundEntry HeavyFootsteps;
    HeavyFootsteps.SoundID = TEXT("heavy_footsteps");
    HeavyFootsteps.Category = EAudio_SoundCategory::SFX;
    HeavyFootsteps.Volume = 0.8f;
    HeavyFootsteps.bLoop = false;
    RegisteredSounds.Add(HeavyFootsteps.SoundID, HeavyFootsteps);
    
    // Stone crafting
    FAudio_SoundEntry StoneCrafting;
    StoneCrafting.SoundID = TEXT("stone_crafting");
    StoneCrafting.Category = EAudio_SoundCategory::SFX;
    StoneCrafting.Volume = 0.7f;
    StoneCrafting.bLoop = false;
    RegisteredSounds.Add(StoneCrafting.SoundID, StoneCrafting);
    
    // Player footsteps
    FAudio_SoundEntry PlayerFootsteps;
    PlayerFootsteps.SoundID = TEXT("player_footsteps");
    PlayerFootsteps.Category = EAudio_SoundCategory::SFX;
    PlayerFootsteps.Volume = 0.5f;
    PlayerFootsteps.bLoop = false;
    RegisteredSounds.Add(PlayerFootsteps.SoundID, PlayerFootsteps);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Initialized %d default sounds"), RegisteredSounds.Num());
}

void UAudio_SoundManager::InitializeBiomeAmbiences()
{
    // Forest biome
    FAudio_BiomeAmbience ForestBiome;
    ForestBiome.BiomeType = EAudio_BiomeType::Forest;
    ForestBiome.CrossfadeTime = 2.5f;
    
    FAudio_SoundEntry ForestBirds;
    ForestBirds.SoundID = TEXT("forest_birds");
    ForestBirds.Category = EAudio_SoundCategory::Ambient;
    ForestBirds.Volume = 0.4f;
    ForestBirds.bLoop = true;
    ForestBiome.AmbientSounds.Add(ForestBirds);
    
    FAudio_SoundEntry ForestWind;
    ForestWind.SoundID = TEXT("forest_wind");
    ForestWind.Category = EAudio_SoundCategory::Ambient;
    ForestWind.Volume = 0.3f;
    ForestWind.bLoop = true;
    ForestBiome.AmbientSounds.Add(ForestWind);
    
    BiomeAmbiences.Add(EAudio_BiomeType::Forest, ForestBiome);
    
    // Savanna biome
    FAudio_BiomeAmbience SavannaBiome;
    SavannaBiome.BiomeType = EAudio_BiomeType::Savanna;
    SavannaBiome.CrossfadeTime = 2.0f;
    
    FAudio_SoundEntry SavannaWind;
    SavannaWind.SoundID = TEXT("savanna_wind");
    SavannaWind.Category = EAudio_SoundCategory::Ambient;
    SavannaWind.Volume = 0.5f;
    SavannaWind.bLoop = true;
    SavannaBiome.AmbientSounds.Add(SavannaWind);
    
    BiomeAmbiences.Add(EAudio_BiomeType::Savanna, SavannaBiome);
    
    // Swamp biome
    FAudio_BiomeAmbience SwampBiome;
    SwampBiome.BiomeType = EAudio_BiomeType::Swamp;
    SwampBiome.CrossfadeTime = 3.0f;
    
    FAudio_SoundEntry SwampBubbles;
    SwampBubbles.SoundID = TEXT("swamp_bubbles");
    SwampBubbles.Category = EAudio_SoundCategory::Ambient;
    SwampBubbles.Volume = 0.4f;
    SwampBubbles.bLoop = true;
    SwampBiome.AmbientSounds.Add(SwampBubbles);
    
    BiomeAmbiences.Add(EAudio_BiomeType::Swamp, SwampBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Initialized %d biome ambiences"), BiomeAmbiences.Num());
}

void UAudio_SoundManager::PlaySound(const FString& SoundID, FVector Location, float VolumeMultiplier)
{
    if (!RegisteredSounds.Contains(SoundID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sound ID '%s' not found"), *SoundID);
        return;
    }
    
    const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundID];
    
    if (!SoundEntry.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Sound cue for '%s' is not valid"), *SoundID);
        return;
    }
    
    UAudioComponent* AudioComp = CreateAudioComponent(SoundEntry, Location);
    if (AudioComp)
    {
        float FinalVolume = SoundEntry.Volume * VolumeMultiplier * MasterVolume;
        if (CategoryVolumes.Contains(SoundEntry.Category))
        {
            FinalVolume *= CategoryVolumes[SoundEntry.Category];
        }
        
        AudioComp->SetVolumeMultiplier(FinalVolume);
        AudioComp->SetPitchMultiplier(SoundEntry.Pitch);
        AudioComp->Play();
        
        ActiveAudioComponents.Add(SoundID, AudioComp);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing sound '%s' at volume %.2f"), *SoundID, FinalVolume);
    }
}

void UAudio_SoundManager::PlaySoundAtLocation(const FString& SoundID, FVector Location, float VolumeMultiplier)
{
    PlaySound(SoundID, Location, VolumeMultiplier);
}

void UAudio_SoundManager::StopSound(const FString& SoundID)
{
    if (ActiveAudioComponents.Contains(SoundID))
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[SoundID];
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
        ActiveAudioComponents.Remove(SoundID);
    }
}

void UAudio_SoundManager::StopAllSounds()
{
    for (auto& Pair : ActiveAudioComponents)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->Stop();
        }
    }
    ActiveAudioComponents.Empty();
}

void UAudio_SoundManager::SetBiomeAmbience(EAudio_BiomeType BiomeType)
{
    if (CurrentBiome == BiomeType)
    {
        return;
    }
    
    CurrentBiome = BiomeType;
    
    // Stop current ambience with fade out
    if (CurrentAmbienceComponent && IsValid(CurrentAmbienceComponent))
    {
        CurrentAmbienceComponent->FadeOut(2.0f, 0.0f);
    }
    
    // Start new biome ambience
    if (BiomeAmbiences.Contains(BiomeType))
    {
        const FAudio_BiomeAmbience& BiomeAmbience = BiomeAmbiences[BiomeType];
        
        for (const FAudio_SoundEntry& AmbientSound : BiomeAmbience.AmbientSounds)
        {
            PlaySound(AmbientSound.SoundID, FVector::ZeroVector, 1.0f);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Set biome ambience to %d"), (int32)BiomeType);
    }
}

void UAudio_SoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active audio components
    for (auto& Pair : ActiveAudioComponents)
    {
        if (IsValid(Pair.Value))
        {
            const FString& SoundID = Pair.Key;
            if (RegisteredSounds.Contains(SoundID))
            {
                const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundID];
                float FinalVolume = SoundEntry.Volume * MasterVolume;
                if (CategoryVolumes.Contains(SoundEntry.Category))
                {
                    FinalVolume *= CategoryVolumes[SoundEntry.Category];
                }
                Pair.Value->SetVolumeMultiplier(FinalVolume);
            }
        }
    }
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    
    // Update all active audio components of this category
    for (auto& Pair : ActiveAudioComponents)
    {
        if (IsValid(Pair.Value))
        {
            const FString& SoundID = Pair.Key;
            if (RegisteredSounds.Contains(SoundID))
            {
                const FAudio_SoundEntry& SoundEntry = RegisteredSounds[SoundID];
                if (SoundEntry.Category == Category)
                {
                    float FinalVolume = SoundEntry.Volume * MasterVolume * Volume;
                    Pair.Value->SetVolumeMultiplier(FinalVolume);
                }
            }
        }
    }
}

float UAudio_SoundManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    if (CategoryVolumes.Contains(Category))
    {
        return CategoryVolumes[Category];
    }
    return 1.0f;
}

void UAudio_SoundManager::RegisterSound(const FAudio_SoundEntry& SoundEntry)
{
    RegisteredSounds.Add(SoundEntry.SoundID, SoundEntry);
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Registered sound '%s'"), *SoundEntry.SoundID);
}

bool UAudio_SoundManager::IsSoundPlaying(const FString& SoundID) const
{
    if (ActiveAudioComponents.Contains(SoundID))
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[SoundID];
        return IsValid(AudioComp) && AudioComp->IsPlaying();
    }
    return false;
}

void UAudio_SoundManager::PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location)
{
    FString SoundID = FString::Printf(TEXT("%s_%s"), *DinosaurType.ToLower(), *SoundType.ToLower());
    PlaySoundAtLocation(SoundID, Location, 1.0f);
}

void UAudio_SoundManager::PlayCraftingSound(const FString& MaterialType, FVector Location)
{
    FString SoundID = FString::Printf(TEXT("%s_crafting"), *MaterialType.ToLower());
    PlaySoundAtLocation(SoundID, Location, 1.0f);
}

void UAudio_SoundManager::PlayFootstepSound(const FString& SurfaceType, FVector Location)
{
    FString SoundID = FString::Printf(TEXT("footstep_%s"), *SurfaceType.ToLower());
    PlaySoundAtLocation(SoundID, Location, 0.6f);
}

UAudioComponent* UAudio_SoundManager::CreateAudioComponent(const FAudio_SoundEntry& SoundEntry, FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
    if (!AudioComp)
    {
        return nullptr;
    }
    
    AudioComp->SetWorldLocation(Location);
    AudioComp->bAutoDestroy = !SoundEntry.bLoop;
    AudioComp->bIsUISound = (SoundEntry.Category == EAudio_SoundCategory::UI);
    
    return AudioComp;
}

void UAudio_SoundManager::CleanupFinishedAudioComponents()
{
    TArray<FString> FinishedSounds;
    
    for (auto& Pair : ActiveAudioComponents)
    {
        if (!IsValid(Pair.Value) || !Pair.Value->IsPlaying())
        {
            FinishedSounds.Add(Pair.Key);
        }
    }
    
    for (const FString& SoundID : FinishedSounds)
    {
        ActiveAudioComponents.Remove(SoundID);
    }
}