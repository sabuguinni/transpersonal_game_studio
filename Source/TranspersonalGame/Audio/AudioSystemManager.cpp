#include "AudioSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initializing"));
    
    // Initialize category volumes
    CategoryVolumes.Add(EAudio_AudioCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_AudioCategory::Music, 0.5f);
    CategoryVolumes.Add(EAudio_AudioCategory::SFX, 0.8f);
    CategoryVolumes.Add(EAudio_AudioCategory::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_AudioCategory::UI, 0.6f);
    CategoryVolumes.Add(EAudio_AudioCategory::Dinosaur, 0.9f);
    
    InitializeBiomeConfigs();
    InitializeDefaultSounds();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initialization complete"));
}

void UAudioSystemManager::Deinitialize()
{
    StopAllAmbient();
    
    if (CurrentAmbientComponent)
    {
        CurrentAmbientComponent->DestroyComponent();
        CurrentAmbientComponent = nullptr;
    }
    
    if (CurrentMusicComponent)
    {
        CurrentMusicComponent->DestroyComponent();
        CurrentMusicComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudioSystemManager::PlaySound2D(const FString& SoundName, float VolumeMultiplier)
{
    FAudio_SoundEntry* SoundEntry = FindSound(SoundName);
    if (!SoundEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Sound not found: %s"), *SoundName);
        return;
    }
    
    USoundBase* Sound = SoundEntry->SoundAsset.LoadSynchronous();
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Failed to load sound asset: %s"), *SoundName);
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    float* CategoryVolume = CategoryVolumes.Find(SoundEntry->Category);
    float FinalVolume = SoundEntry->Volume * VolumeMultiplier * MasterVolume;
    if (CategoryVolume)
    {
        FinalVolume *= *CategoryVolume;
    }
    
    UGameplayStatics::PlaySound2D(World, Sound, FinalVolume, SoundEntry->Pitch);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Played 2D sound: %s"), *SoundName);
}

void UAudioSystemManager::PlaySound3D(const FString& SoundName, const FVector& Location, float VolumeMultiplier)
{
    FAudio_SoundEntry* SoundEntry = FindSound(SoundName);
    if (!SoundEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Sound not found: %s"), *SoundName);
        return;
    }
    
    USoundBase* Sound = SoundEntry->SoundAsset.LoadSynchronous();
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Failed to load sound asset: %s"), *SoundName);
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    float* CategoryVolume = CategoryVolumes.Find(SoundEntry->Category);
    float FinalVolume = SoundEntry->Volume * VolumeMultiplier * MasterVolume;
    if (CategoryVolume)
    {
        FinalVolume *= *CategoryVolume;
    }
    
    UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, FinalVolume, SoundEntry->Pitch);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Played 3D sound: %s at location: %s"), *SoundName, *Location.ToString());
}

void UAudioSystemManager::PlayDinosaurSound(const FString& DinosaurType, const FVector& Location)
{
    FString SoundName = FString::Printf(TEXT("Dinosaur_%s_Roar"), *DinosaurType);
    PlaySound3D(SoundName, Location, 1.0f);
}

void UAudioSystemManager::SetBiomeAmbient(EAudio_BiomeType BiomeType)
{
    StopAllAmbient();
    
    FAudio_BiomeAudioConfig* BiomeConfig = BiomeConfigs.Find(BiomeType);
    if (!BiomeConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Biome config not found for type: %d"), (int32)BiomeType);
        return;
    }
    
    if (BiomeConfig->AmbientSounds.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No ambient sounds configured for biome: %d"), (int32)BiomeType);
        return;
    }
    
    // Play first ambient sound (can be extended to random selection)
    const FAudio_SoundEntry& AmbientSound = BiomeConfig->AmbientSounds[0];
    USoundBase* Sound = AmbientSound.SoundAsset.LoadSynchronous();
    
    if (Sound)
    {
        CurrentAmbientComponent = CreateAudioComponent();
        if (CurrentAmbientComponent)
        {
            CurrentAmbientComponent->SetSound(Sound);
            CurrentAmbientComponent->SetVolumeMultiplier(BiomeConfig->AmbientVolume * MasterVolume);
            CurrentAmbientComponent->SetPitchMultiplier(AmbientSound.Pitch);
            CurrentAmbientComponent->bAutoActivate = true;
            CurrentAmbientComponent->Play();
            
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Started biome ambient for type: %d"), (int32)BiomeType);
        }
    }
}

void UAudioSystemManager::StopAllAmbient()
{
    if (CurrentAmbientComponent && CurrentAmbientComponent->IsPlaying())
    {
        CurrentAmbientComponent->Stop();
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopped ambient audio"));
    }
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update currently playing ambient
    if (CurrentAmbientComponent)
    {
        CurrentAmbientComponent->SetVolumeMultiplier(MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set master volume to: %f"), MasterVolume);
}

void UAudioSystemManager::SetCategoryVolume(EAudio_AudioCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set category volume for %d to: %f"), (int32)Category, Volume);
}

void UAudioSystemManager::RegisterSound(const FAudio_SoundEntry& SoundEntry)
{
    RegisteredSounds.Add(SoundEntry.SoundName, SoundEntry);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered sound: %s"), *SoundEntry.SoundName);
}

void UAudioSystemManager::InitializeDefaultSounds()
{
    // Register default dinosaur sounds (placeholders - will be replaced with real assets)
    FAudio_SoundEntry TRexRoar;
    TRexRoar.SoundName = TEXT("Dinosaur_TRex_Roar");
    TRexRoar.Category = EAudio_AudioCategory::Dinosaur;
    TRexRoar.Volume = 0.9f;
    TRexRoar.bIs3D = true;
    RegisterSound(TRexRoar);
    
    FAudio_SoundEntry RaptorCall;
    RaptorCall.SoundName = TEXT("Dinosaur_Raptor_Call");
    RaptorCall.Category = EAudio_AudioCategory::Dinosaur;
    RaptorCall.Volume = 0.8f;
    RaptorCall.bIs3D = true;
    RegisterSound(RaptorCall);
    
    FAudio_SoundEntry Footsteps;
    Footsteps.SoundName = TEXT("Player_Footsteps");
    Footsteps.Category = EAudio_AudioCategory::SFX;
    Footsteps.Volume = 0.6f;
    Footsteps.bIs3D = true;
    RegisterSound(Footsteps);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Default sounds registered"));
}

void UAudioSystemManager::InitializeBiomeConfigs()
{
    // Savana biome
    FAudio_BiomeAudioConfig SavanaConfig;
    SavanaConfig.BiomeType = EAudio_BiomeType::Savana;
    SavanaConfig.AmbientVolume = 0.7f;
    SavanaConfig.MusicVolume = 0.5f;
    BiomeConfigs.Add(EAudio_BiomeType::Savana, SavanaConfig);
    
    // Forest biome
    FAudio_BiomeAudioConfig ForestConfig;
    ForestConfig.BiomeType = EAudio_BiomeType::Forest;
    ForestConfig.AmbientVolume = 0.8f;
    ForestConfig.MusicVolume = 0.4f;
    BiomeConfigs.Add(EAudio_BiomeType::Forest, ForestConfig);
    
    // Desert biome
    FAudio_BiomeAudioConfig DesertConfig;
    DesertConfig.BiomeType = EAudio_BiomeType::Desert;
    DesertConfig.AmbientVolume = 0.6f;
    DesertConfig.MusicVolume = 0.5f;
    BiomeConfigs.Add(EAudio_BiomeType::Desert, DesertConfig);
    
    // Swamp biome
    FAudio_BiomeAudioConfig SwampConfig;
    SwampConfig.BiomeType = EAudio_BiomeType::Swamp;
    SwampConfig.AmbientVolume = 0.9f;
    SwampConfig.MusicVolume = 0.3f;
    BiomeConfigs.Add(EAudio_BiomeType::Swamp, SwampConfig);
    
    // Mountain biome
    FAudio_BiomeAudioConfig MountainConfig;
    MountainConfig.BiomeType = EAudio_BiomeType::Mountain;
    MountainConfig.AmbientVolume = 0.5f;
    MountainConfig.MusicVolume = 0.6f;
    BiomeConfigs.Add(EAudio_BiomeType::Mountain, MountainConfig);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Biome configs initialized"));
}

UAudioComponent* UAudioSystemManager::CreateAudioComponent()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
    if (AudioComp)
    {
        AudioComp->bAutoActivate = false;
        AudioComp->bStopWhenOwnerDestroyed = false;
        AudioComp->RegisterComponent();
    }
    
    return AudioComp;
}

FAudio_SoundEntry* UAudioSystemManager::FindSound(const FString& SoundName)
{
    return RegisteredSounds.Find(SoundName);
}