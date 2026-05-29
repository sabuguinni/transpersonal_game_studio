#include "Audio_AudioManager.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"

AAudio_AudioManager::AAudio_AudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Create audio components
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    SFXAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SFXAudioComponent"));
    SFXAudioComponent->SetupAttachment(RootComponent);
    SFXAudioComponent->bAutoActivate = false;

    // Initialize default values
    CurrentBiome = EBiomeType::Savanna;
    CurrentThreatLevel = EAudio_ThreatLevel::None;
    ThreatLevelTimer = 0.0f;
    BiomeTransitionTimer = 0.0f;
    bIsTransitioningBiome = false;

    // Initialize volume settings
    MasterVolume = 1.0f;
    MusicVolume = 0.6f;
    SFXVolume = 0.8f;
    AmbientVolume = 0.5f;
    VoiceVolume = 1.0f;
}

void AAudio_AudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeAudio();
    LoadBiomeAudioAssets();
    
    // Set initial biome to Savanna
    SetBiome(EBiomeType::Savanna);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager initialized successfully"));
}

void AAudio_AudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update threat level timer
    if (CurrentThreatLevel != EAudio_ThreatLevel::None)
    {
        ThreatLevelTimer += DeltaTime;
        UpdateMusicForThreatLevel();
    }

    // Handle biome transition
    if (bIsTransitioningBiome)
    {
        BiomeTransitionTimer += DeltaTime;
        TransitionBiomeAudio();
    }
}

void AAudio_AudioManager::SetBiome(EBiomeType NewBiome)
{
    if (NewBiome == CurrentBiome)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Transitioning from biome %d to %d"), 
           (int32)CurrentBiome, (int32)NewBiome);

    CurrentBiome = NewBiome;
    bIsTransitioningBiome = true;
    BiomeTransitionTimer = 0.0f;

    // Start biome transition
    TransitionBiomeAudio();
}

void AAudio_AudioManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (NewThreatLevel == CurrentThreatLevel)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Threat level changed from %d to %d"), 
           (int32)CurrentThreatLevel, (int32)NewThreatLevel);

    CurrentThreatLevel = NewThreatLevel;
    ThreatLevelTimer = 0.0f;

    UpdateMusicForThreatLevel();
}

void AAudio_AudioManager::PlaySFX(const FString& SFXName, FVector Location, float VolumeMultiplier)
{
    if (SFXLibrary.Contains(SFXName))
    {
        const FAudio_SoundEntry& SoundEntry = SFXLibrary[SFXName];
        
        if (SoundEntry.SoundAsset.IsValid())
        {
            USoundBase* SoundAsset = SoundEntry.SoundAsset.LoadSynchronous();
            if (SoundAsset)
            {
                float FinalVolume = SoundEntry.Volume * VolumeMultiplier * SFXVolume * MasterVolume;
                
                if (SoundEntry.bIs3D && Location != FVector::ZeroVector)
                {
                    UGameplayStatics::PlaySoundAtLocation(
                        GetWorld(), 
                        SoundAsset, 
                        Location, 
                        FinalVolume, 
                        SoundEntry.Pitch
                    );
                }
                else
                {
                    UGameplayStatics::PlaySound2D(
                        GetWorld(), 
                        SoundAsset, 
                        FinalVolume, 
                        SoundEntry.Pitch
                    );
                }
                
                UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Played SFX %s at volume %f"), 
                       *SFXName, FinalVolume);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: SFX %s not found in library"), *SFXName);
    }
}

void AAudio_AudioManager::PlayVoiceLine(USoundBase* VoiceClip, float VolumeMultiplier)
{
    if (VoiceClip && SFXAudioComponent)
    {
        float FinalVolume = VolumeMultiplier * VoiceVolume * MasterVolume;
        
        SFXAudioComponent->SetSound(VoiceClip);
        SFXAudioComponent->SetVolumeMultiplier(FinalVolume);
        SFXAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Playing voice line at volume %f"), FinalVolume);
    }
}

void AAudio_AudioManager::StopAllAudio()
{
    if (MusicAudioComponent)
    {
        MusicAudioComponent->Stop();
    }
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->Stop();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: All audio stopped"));
}

void AAudio_AudioManager::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    // Update all active audio components
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
}

void AAudio_AudioManager::SetMusicVolume(float NewVolume)
{
    MusicVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
}

void AAudio_AudioManager::SetSFXVolume(float NewVolume)
{
    SFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void AAudio_AudioManager::SetAmbientVolume(float NewVolume)
{
    AmbientVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
}

void AAudio_AudioManager::SetVoiceVolume(float NewVolume)
{
    VoiceVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void AAudio_AudioManager::RegisterSFX(const FString& SFXName, USoundBase* SoundAsset, float Volume, float Pitch)
{
    if (SoundAsset)
    {
        FAudio_SoundEntry NewEntry;
        NewEntry.SoundName = SFXName;
        NewEntry.SoundAsset = SoundAsset;
        NewEntry.Volume = Volume;
        NewEntry.Pitch = Pitch;
        
        SFXLibrary.Add(SFXName, NewEntry);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_AudioManager: Registered SFX %s"), *SFXName);
    }
}

void AAudio_AudioManager::InitializeBiomeAudio()
{
    // Initialize biome audio configurations
    BiomeAudioConfigs.Empty();
    
    // Savanna biome
    FAudio_BiomeAudioConfig SavannaConfig;
    SavannaConfig.BiomeType = EBiomeType::Savanna;
    SavannaConfig.MusicVolume = 0.4f;
    SavannaConfig.AmbientVolume = 0.6f;
    BiomeAudioConfigs.Add(SavannaConfig);
    
    // Forest biome
    FAudio_BiomeAudioConfig ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Forest;
    ForestConfig.MusicVolume = 0.3f;
    ForestConfig.AmbientVolume = 0.8f;
    BiomeAudioConfigs.Add(ForestConfig);
    
    // Desert biome
    FAudio_BiomeAudioConfig DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.MusicVolume = 0.5f;
    DesertConfig.AmbientVolume = 0.4f;
    BiomeAudioConfigs.Add(DesertConfig);
    
    // Swamp biome
    FAudio_BiomeAudioConfig SwampConfig;
    SwampConfig.BiomeType = EBiomeType::Swamp;
    SwampConfig.MusicVolume = 0.3f;
    SwampConfig.AmbientVolume = 0.9f;
    BiomeAudioConfigs.Add(SwampConfig);
    
    // Mountain biome
    FAudio_BiomeAudioConfig MountainConfig;
    MountainConfig.BiomeType = EBiomeType::Mountain;
    MountainConfig.MusicVolume = 0.6f;
    MountainConfig.AmbientVolume = 0.5f;
    BiomeAudioConfigs.Add(MountainConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Initialized %d biome audio configs"), BiomeAudioConfigs.Num());
}

void AAudio_AudioManager::UpdateMusicForThreatLevel()
{
    if (!MusicAudioComponent)
    {
        return;
    }
    
    float ThreatVolumeMultiplier = 1.0f;
    float ThreatPitchMultiplier = 1.0f;
    
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::None:
            ThreatVolumeMultiplier = 0.6f;
            ThreatPitchMultiplier = 1.0f;
            break;
        case EAudio_ThreatLevel::Low:
            ThreatVolumeMultiplier = 0.8f;
            ThreatPitchMultiplier = 1.1f;
            break;
        case EAudio_ThreatLevel::Medium:
            ThreatVolumeMultiplier = 1.0f;
            ThreatPitchMultiplier = 1.2f;
            break;
        case EAudio_ThreatLevel::High:
            ThreatVolumeMultiplier = 1.2f;
            ThreatPitchMultiplier = 1.3f;
            break;
        case EAudio_ThreatLevel::Extreme:
            ThreatVolumeMultiplier = 1.5f;
            ThreatPitchMultiplier = 1.4f;
            break;
    }
    
    float FinalVolume = MusicVolume * ThreatVolumeMultiplier * MasterVolume;
    MusicAudioComponent->SetVolumeMultiplier(FinalVolume);
    MusicAudioComponent->SetPitchMultiplier(ThreatPitchMultiplier);
}

void AAudio_AudioManager::TransitionBiomeAudio()
{
    FAudio_BiomeAudioConfig* BiomeConfig = GetBiomeConfig(CurrentBiome);
    if (!BiomeConfig)
    {
        return;
    }
    
    // Transition duration: 3 seconds
    const float TransitionDuration = 3.0f;
    float TransitionAlpha = FMath::Clamp(BiomeTransitionTimer / TransitionDuration, 0.0f, 1.0f);
    
    if (TransitionAlpha >= 1.0f)
    {
        bIsTransitioningBiome = false;
        BiomeTransitionTimer = 0.0f;
        
        // Set final volumes
        if (MusicAudioComponent)
        {
            MusicAudioComponent->SetVolumeMultiplier(BiomeConfig->MusicVolume * MasterVolume);
        }
        
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(BiomeConfig->AmbientVolume * MasterVolume);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Biome transition complete"));
    }
    else
    {
        // Interpolate volumes during transition
        float CurrentMusicVolume = FMath::Lerp(MusicVolume, BiomeConfig->MusicVolume, TransitionAlpha);
        float CurrentAmbientVolume = FMath::Lerp(AmbientVolume, BiomeConfig->AmbientVolume, TransitionAlpha);
        
        if (MusicAudioComponent)
        {
            MusicAudioComponent->SetVolumeMultiplier(CurrentMusicVolume * MasterVolume);
        }
        
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(CurrentAmbientVolume * MasterVolume);
        }
    }
}

void AAudio_AudioManager::LoadBiomeAudioAssets()
{
    // Register common SFX
    RegisterSFX(TEXT("Footstep_Dirt"), nullptr, 0.7f, 1.0f);
    RegisterSFX(TEXT("Footstep_Stone"), nullptr, 0.8f, 1.2f);
    RegisterSFX(TEXT("Footstep_Water"), nullptr, 0.6f, 0.9f);
    RegisterSFX(TEXT("DinosaurRoar_TRex"), nullptr, 1.5f, 0.8f);
    RegisterSFX(TEXT("DinosaurRoar_Raptor"), nullptr, 1.2f, 1.1f);
    RegisterSFX(TEXT("CraftingSound_Stone"), nullptr, 0.9f, 1.0f);
    RegisterSFX(TEXT("Fire_Crackling"), nullptr, 0.5f, 1.0f);
    RegisterSFX(TEXT("Wind_Forest"), nullptr, 0.4f, 1.0f);
    RegisterSFX(TEXT("Wind_Desert"), nullptr, 0.6f, 1.2f);
    RegisterSFX(TEXT("Water_River"), nullptr, 0.7f, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_AudioManager: Loaded %d SFX entries"), SFXLibrary.Num());
}

FAudio_BiomeAudioConfig* AAudio_AudioManager::GetBiomeConfig(EBiomeType BiomeType)
{
    for (FAudio_BiomeAudioConfig& Config : BiomeAudioConfigs)
    {
        if (Config.BiomeType == BiomeType)
        {
            return &Config;
        }
    }
    return nullptr;
}