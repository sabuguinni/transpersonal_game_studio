#include "Audio_EnvironmentalSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AAudio_EnvironmentalSoundManager::AAudio_EnvironmentalSoundManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    RootComponent = AmbientAudioComponent;
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.7f);

    RandomSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RandomSoundComponent"));
    RandomSoundComponent->SetupAttachment(RootComponent);
    RandomSoundComponent->bAutoActivate = false;
    RandomSoundComponent->SetVolumeMultiplier(0.5f);

    // Default settings
    MaxAudibleDistance = 5000.0f;
    VolumeMultiplier = 1.0f;
    CurrentBiome = TEXT("Forest");
}

void AAudio_EnvironmentalSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeAudioData();
    SetBiome(CurrentBiome);
}

void AAudio_EnvironmentalSoundManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopRandomSoundTimer();
    StopAllAmbientSounds();
    Super::EndPlay(EndPlayReason);
}

void AAudio_EnvironmentalSoundManager::InitializeBiomeAudioData()
{
    // Forest biome audio setup
    FAudio_BiomeAudioData ForestAudio;
    ForestAudio.RandomSoundInterval = 12.0f;
    ForestAudio.RandomSoundVariance = 8.0f;
    BiomeAudioMap.Add(TEXT("Forest"), ForestAudio);

    // Swamp biome audio setup
    FAudio_BiomeAudioData SwampAudio;
    SwampAudio.RandomSoundInterval = 18.0f;
    SwampAudio.RandomSoundVariance = 6.0f;
    BiomeAudioMap.Add(TEXT("Swamp"), SwampAudio);

    // Savanna biome audio setup
    FAudio_BiomeAudioData SavannaAudio;
    SavannaAudio.RandomSoundInterval = 20.0f;
    SavannaAudio.RandomSoundVariance = 10.0f;
    BiomeAudioMap.Add(TEXT("Savanna"), SavannaAudio);

    // Desert biome audio setup
    FAudio_BiomeAudioData DesertAudio;
    DesertAudio.RandomSoundInterval = 25.0f;
    DesertAudio.RandomSoundVariance = 12.0f;
    BiomeAudioMap.Add(TEXT("Desert"), DesertAudio);

    // Mountain biome audio setup
    FAudio_BiomeAudioData MountainAudio;
    MountainAudio.RandomSoundInterval = 30.0f;
    MountainAudio.RandomSoundVariance = 15.0f;
    BiomeAudioMap.Add(TEXT("Mountain"), MountainAudio);
}

void AAudio_EnvironmentalSoundManager::SetBiome(const FString& BiomeName)
{
    if (CurrentBiome == BiomeName)
    {
        return;
    }

    // Stop current ambient sounds
    StopAllAmbientSounds();
    StopRandomSoundTimer();

    CurrentBiome = BiomeName;

    // Start new biome audio
    if (BiomeAudioMap.Contains(BiomeName))
    {
        const FAudio_BiomeAudioData& BiomeData = BiomeAudioMap[BiomeName];
        
        // Start ambient loop if available
        if (BiomeData.AmbientLoop && AmbientAudioComponent)
        {
            AmbientAudioComponent->SetSound(BiomeData.AmbientLoop);
            AmbientAudioComponent->SetVolumeMultiplier(0.7f * VolumeMultiplier);
            AmbientAudioComponent->Play();
        }

        // Start random sound timer
        StartRandomSoundTimer();
    }
}

void AAudio_EnvironmentalSoundManager::PlayRandomBiomeSound()
{
    if (!BiomeAudioMap.Contains(CurrentBiome))
    {
        return;
    }

    const FAudio_BiomeAudioData& BiomeData = BiomeAudioMap[CurrentBiome];
    
    if (BiomeData.RandomSounds.Num() > 0 && RandomSoundComponent)
    {
        int32 RandomIndex = FMath::RandRange(0, BiomeData.RandomSounds.Num() - 1);
        USoundCue* RandomSound = BiomeData.RandomSounds[RandomIndex];
        
        if (RandomSound)
        {
            RandomSoundComponent->SetSound(RandomSound);
            RandomSoundComponent->SetVolumeMultiplier(0.5f * VolumeMultiplier);
            RandomSoundComponent->Play();
        }
    }
}

void AAudio_EnvironmentalSoundManager::SetMasterVolume(float Volume)
{
    VolumeMultiplier = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(0.7f * VolumeMultiplier);
    }
    
    if (RandomSoundComponent)
    {
        RandomSoundComponent->SetVolumeMultiplier(0.5f * VolumeMultiplier);
    }
}

void AAudio_EnvironmentalSoundManager::StopAllAmbientSounds()
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    
    if (RandomSoundComponent && RandomSoundComponent->IsPlaying())
    {
        RandomSoundComponent->Stop();
    }
}

void AAudio_EnvironmentalSoundManager::StartRandomSoundTimer()
{
    if (!BiomeAudioMap.Contains(CurrentBiome))
    {
        return;
    }

    const FAudio_BiomeAudioData& BiomeData = BiomeAudioMap[CurrentBiome];
    float TimerInterval = BiomeData.RandomSoundInterval + FMath::RandRange(-BiomeData.RandomSoundVariance, BiomeData.RandomSoundVariance);
    TimerInterval = FMath::Max(TimerInterval, 5.0f); // Minimum 5 seconds between sounds

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RandomSoundTimer,
            this,
            &AAudio_EnvironmentalSoundManager::PlayRandomBiomeSound,
            TimerInterval,
            false
        );
    }
}

void AAudio_EnvironmentalSoundManager::StopRandomSoundTimer()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RandomSoundTimer);
    }
}