#include "Audio_EnvironmentalSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AAudio_EnvironmentalSoundManager::AAudio_EnvironmentalSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Tick once per second for performance

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    RootComponent = AmbientAudioComponent;
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.7f);

    RandomSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RandomSoundComponent"));
    RandomSoundComponent->SetupAttachment(RootComponent);
    RandomSoundComponent->bAutoActivate = false;
    RandomSoundComponent->SetVolumeMultiplier(0.8f);

    WeatherAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudioComponent"));
    WeatherAudioComponent->SetupAttachment(RootComponent);
    WeatherAudioComponent->bAutoActivate = false;
    WeatherAudioComponent->SetVolumeMultiplier(0.6f);

    // Initialize default biome
    CurrentBiome = EAudio_BiomeType::Forest;
    
    // Set default volumes
    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;
    EffectsVolume = 0.8f;
}

void AAudio_EnvironmentalSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeAudio();
    LoadAudioAssets();
    UpdateAmbientAudio();
    ScheduleNextRandomSound();

    UE_LOG(LogTemp, Warning, TEXT("Audio_EnvironmentalSoundManager: BeginPlay completed for biome %d"), (int32)CurrentBiome);
}

void AAudio_EnvironmentalSoundManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(RandomSoundTimer);
    }
    
    // Stop all audio components
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    
    if (RandomSoundComponent && RandomSoundComponent->IsPlaying())
    {
        RandomSoundComponent->Stop();
    }
    
    if (WeatherAudioComponent && WeatherAudioComponent->IsPlaying())
    {
        WeatherAudioComponent->Stop();
    }

    Super::EndPlay(EndPlayReason);
}

void AAudio_EnvironmentalSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update volume levels based on master volume
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
    
    if (RandomSoundComponent)
    {
        RandomSoundComponent->SetVolumeMultiplier(EffectsVolume * MasterVolume);
    }
    
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->SetVolumeMultiplier(EffectsVolume * MasterVolume * CurrentRainIntensity);
    }
}

void AAudio_EnvironmentalSoundManager::InitializeBiomeAudio()
{
    // Initialize default biome audio data
    FAudio_BiomeAudioData ForestData;
    ForestData.MinRandomInterval = 15.0f;
    ForestData.MaxRandomInterval = 45.0f;
    ForestData.VolumeMultiplier = 1.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Forest, ForestData);

    FAudio_BiomeAudioData SwampData;
    SwampData.MinRandomInterval = 20.0f;
    SwampData.MaxRandomInterval = 60.0f;
    SwampData.VolumeMultiplier = 0.8f;
    BiomeAudioMap.Add(EAudio_BiomeType::Swamp, SwampData);

    FAudio_BiomeAudioData SavannaData;
    SavannaData.MinRandomInterval = 25.0f;
    SavannaData.MaxRandomInterval = 90.0f;
    SavannaData.VolumeMultiplier = 0.9f;
    BiomeAudioMap.Add(EAudio_BiomeType::Savanna, SavannaData);

    FAudio_BiomeAudioData DesertData;
    DesertData.MinRandomInterval = 30.0f;
    DesertData.MaxRandomInterval = 120.0f;
    DesertData.VolumeMultiplier = 0.6f;
    BiomeAudioMap.Add(EAudio_BiomeType::Desert, DesertData);

    FAudio_BiomeAudioData MountainData;
    MountainData.MinRandomInterval = 20.0f;
    MountainData.MaxRandomInterval = 80.0f;
    MountainData.VolumeMultiplier = 0.7f;
    BiomeAudioMap.Add(EAudio_BiomeType::Mountain, MountainData);

    FAudio_BiomeAudioData CaveData;
    CaveData.MinRandomInterval = 10.0f;
    CaveData.MaxRandomInterval = 30.0f;
    CaveData.VolumeMultiplier = 0.5f;
    BiomeAudioMap.Add(EAudio_BiomeType::Cave, CaveData);
}

void AAudio_EnvironmentalSoundManager::LoadAudioAssets()
{
    // This would normally load actual sound assets
    // For now, we set up the framework for asset loading
    UE_LOG(LogTemp, Warning, TEXT("Audio_EnvironmentalSoundManager: LoadAudioAssets called - ready for sound asset integration"));
}

void AAudio_EnvironmentalSoundManager::UpdateAmbientAudio()
{
    if (!AmbientAudioComponent)
    {
        return;
    }

    // Get current biome data
    const FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentBiome);
    if (!BiomeData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_EnvironmentalSoundManager: No audio data found for biome %d"), (int32)CurrentBiome);
        return;
    }

    // Stop current ambient sound
    if (AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }

    // Load and play new ambient sound if available
    if (!BiomeData->AmbientLoop.IsNull())
    {
        USoundCue* AmbientSound = BiomeData->AmbientLoop.LoadSynchronous();
        if (AmbientSound)
        {
            AmbientAudioComponent->SetSound(AmbientSound);
            AmbientAudioComponent->SetVolumeMultiplier(BiomeData->VolumeMultiplier * AmbientVolume * MasterVolume);
            AmbientAudioComponent->Play();
            UE_LOG(LogTemp, Warning, TEXT("Audio_EnvironmentalSoundManager: Playing ambient sound for biome %d"), (int32)CurrentBiome);
        }
    }
}

void AAudio_EnvironmentalSoundManager::ScheduleNextRandomSound()
{
    if (!GetWorld())
    {
        return;
    }

    const FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentBiome);
    if (!BiomeData)
    {
        return;
    }

    // Calculate random interval
    float RandomInterval = FMath::RandRange(BiomeData->MinRandomInterval, BiomeData->MaxRandomInterval);
    
    // Schedule next random sound
    GetWorld()->GetTimerManager().SetTimer(
        RandomSoundTimer,
        this,
        &AAudio_EnvironmentalSoundManager::PlayRandomBiomeSound,
        RandomInterval,
        false
    );
}

void AAudio_EnvironmentalSoundManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        UpdateAmbientAudio();
        
        // Reset random sound timer for new biome
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(RandomSoundTimer);
            ScheduleNextRandomSound();
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Audio_EnvironmentalSoundManager: Changed to biome %d"), (int32)NewBiome);
    }
}

void AAudio_EnvironmentalSoundManager::PlayRandomBiomeSound()
{
    if (!RandomSoundComponent)
    {
        ScheduleNextRandomSound();
        return;
    }

    const FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentBiome);
    if (!BiomeData || BiomeData->RandomSounds.Num() == 0)
    {
        ScheduleNextRandomSound();
        return;
    }

    // Pick random sound from biome collection
    int32 RandomIndex = FMath::RandRange(0, BiomeData->RandomSounds.Num() - 1);
    const TSoftObjectPtr<USoundWave>& RandomSoundRef = BiomeData->RandomSounds[RandomIndex];
    
    if (!RandomSoundRef.IsNull())
    {
        USoundWave* RandomSound = RandomSoundRef.LoadSynchronous();
        if (RandomSound)
        {
            RandomSoundComponent->SetSound(RandomSound);
            RandomSoundComponent->SetVolumeMultiplier(BiomeData->VolumeMultiplier * EffectsVolume * MasterVolume);
            RandomSoundComponent->Play();
        }
    }

    // Schedule next random sound
    ScheduleNextRandomSound();
}

void AAudio_EnvironmentalSoundManager::StartRain(float Intensity)
{
    if (!WeatherAudioComponent)
    {
        return;
    }

    CurrentRainIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    bIsRaining = true;

    if (!RainSound.IsNull())
    {
        USoundCue* RainSoundCue = RainSound.LoadSynchronous();
        if (RainSoundCue)
        {
            WeatherAudioComponent->SetSound(RainSoundCue);
            WeatherAudioComponent->SetVolumeMultiplier(CurrentRainIntensity * EffectsVolume * MasterVolume);
            WeatherAudioComponent->Play();
            UE_LOG(LogTemp, Warning, TEXT("Audio_EnvironmentalSoundManager: Rain started with intensity %f"), Intensity);
        }
    }
}

void AAudio_EnvironmentalSoundManager::StopRain()
{
    bIsRaining = false;
    CurrentRainIntensity = 0.0f;

    if (WeatherAudioComponent && WeatherAudioComponent->IsPlaying())
    {
        WeatherAudioComponent->Stop();
        UE_LOG(LogTemp, Warning, TEXT("Audio_EnvironmentalSoundManager: Rain stopped"));
    }
}

void AAudio_EnvironmentalSoundManager::PlayThunder()
{
    if (!RandomSoundComponent)
    {
        return;
    }

    if (!ThunderSound.IsNull())
    {
        USoundCue* ThunderSoundCue = ThunderSound.LoadSynchronous();
        if (ThunderSoundCue)
        {
            RandomSoundComponent->SetSound(ThunderSoundCue);
            RandomSoundComponent->SetVolumeMultiplier(EffectsVolume * MasterVolume);
            RandomSoundComponent->Play();
            UE_LOG(LogTemp, Warning, TEXT("Audio_EnvironmentalSoundManager: Thunder played"));
        }
    }
}

void AAudio_EnvironmentalSoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void AAudio_EnvironmentalSoundManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void AAudio_EnvironmentalSoundManager::SetEffectsVolume(float Volume)
{
    EffectsVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}