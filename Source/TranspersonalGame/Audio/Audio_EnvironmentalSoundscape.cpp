#include "Audio_EnvironmentalSoundscape.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Engine/World.h"

UAudio_EnvironmentalSoundscape::UAudio_EnvironmentalSoundscape()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentBiome = EAudio_BiomeType::Forest;
    bIsTransitioning = false;
    TransitionTime = 3.0f;
    MaxAudibleDistance = 2000.0f;

    // Initialize default soundscape data
    FAudio_SoundscapeData ForestData;
    ForestData.BiomeType = EAudio_BiomeType::Forest;
    ForestData.Volume = 0.8f;
    ForestData.FadeTime = 2.0f;
    BiomeSoundscapes.Add(ForestData);

    FAudio_SoundscapeData PlainsData;
    PlainsData.BiomeType = EAudio_BiomeType::Plains;
    PlainsData.Volume = 0.6f;
    PlainsData.FadeTime = 2.5f;
    BiomeSoundscapes.Add(PlainsData);

    FAudio_SoundscapeData MountainData;
    MountainData.BiomeType = EAudio_BiomeType::Mountains;
    MountainData.Volume = 0.7f;
    MountainData.FadeTime = 3.0f;
    BiomeSoundscapes.Add(MountainData);

    FAudio_SoundscapeData RiverData;
    RiverData.BiomeType = EAudio_BiomeType::River;
    RiverData.Volume = 0.9f;
    RiverData.FadeTime = 1.5f;
    BiomeSoundscapes.Add(RiverData);

    FAudio_SoundscapeData CaveData;
    CaveData.BiomeType = EAudio_BiomeType::Cave;
    CaveData.Volume = 0.5f;
    CaveData.FadeTime = 4.0f;
    BiomeSoundscapes.Add(CaveData);
}

void UAudio_EnvironmentalSoundscape::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    StartAmbientAudio();
}

void UAudio_EnvironmentalSoundscape::InitializeAudioComponents()
{
    if (!AmbientAudioComponent)
    {
        AmbientAudioComponent = NewObject<UAudioComponent>(this, TEXT("AmbientAudio"));
        if (AmbientAudioComponent && GetOwner())
        {
            AmbientAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
                FAttachmentTransformRules::KeepWorldTransform);
            AmbientAudioComponent->bAutoActivate = false;
            AmbientAudioComponent->SetVolumeMultiplier(0.8f);
        }
    }

    if (!MusicAudioComponent)
    {
        MusicAudioComponent = NewObject<UAudioComponent>(this, TEXT("MusicAudio"));
        if (MusicAudioComponent && GetOwner())
        {
            MusicAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
                FAttachmentTransformRules::KeepWorldTransform);
            MusicAudioComponent->bAutoActivate = false;
            MusicAudioComponent->SetVolumeMultiplier(0.6f);
        }
    }
}

void UAudio_EnvironmentalSoundscape::SetBiomeSoundscape(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome || bIsTransitioning)
    {
        return;
    }

    FadeToNewSoundscape(NewBiome);
}

void UAudio_EnvironmentalSoundscape::StartAmbientAudio()
{
    LoadSoundscapeForBiome(CurrentBiome);
    
    if (AmbientAudioComponent && AmbientAudioComponent->GetSound())
    {
        AmbientAudioComponent->Play();
    }

    if (MusicAudioComponent && MusicAudioComponent->GetSound())
    {
        MusicAudioComponent->Play();
    }
}

void UAudio_EnvironmentalSoundscape::StopAmbientAudio()
{
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
    }

    if (MusicAudioComponent)
    {
        MusicAudioComponent->Stop();
    }
}

void UAudio_EnvironmentalSoundscape::FadeToNewSoundscape(EAudio_BiomeType NewBiome)
{
    if (bIsTransitioning)
    {
        return;
    }

    bIsTransitioning = true;
    
    // Fade out current audio
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->FadeOut(TransitionTime * 0.5f, 0.0f);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->FadeOut(TransitionTime * 0.5f, 0.0f);
    }

    // Set timer to complete transition
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(TransitionTimerHandle, 
            [this, NewBiome]()
            {
                CurrentBiome = NewBiome;
                LoadSoundscapeForBiome(CurrentBiome);
                
                FAudio_SoundscapeData* SoundscapeData = GetSoundscapeData(CurrentBiome);
                float TargetVolume = SoundscapeData ? SoundscapeData->Volume : 0.8f;
                
                if (AmbientAudioComponent && AmbientAudioComponent->GetSound())
                {
                    AmbientAudioComponent->Play();
                    AmbientAudioComponent->FadeIn(TransitionTime * 0.5f, TargetVolume);
                }
                
                if (MusicAudioComponent && MusicAudioComponent->GetSound())
                {
                    MusicAudioComponent->Play();
                    MusicAudioComponent->FadeIn(TransitionTime * 0.5f, TargetVolume * 0.7f);
                }
                
                CompleteTransition();
            }, 
            TransitionTime * 0.5f, false);
    }
}

void UAudio_EnvironmentalSoundscape::LoadSoundscapeForBiome(EAudio_BiomeType BiomeType)
{
    FAudio_SoundscapeData* SoundscapeData = GetSoundscapeData(BiomeType);
    
    if (!SoundscapeData)
    {
        UE_LOG(LogTemp, Warning, TEXT("No soundscape data found for biome type"));
        return;
    }

    // Load ambient sound
    if (!SoundscapeData->AmbientSound.IsNull())
    {
        USoundCue* AmbientCue = SoundscapeData->AmbientSound.LoadSynchronous();
        if (AmbientCue && AmbientAudioComponent)
        {
            AmbientAudioComponent->SetSound(AmbientCue);
            AmbientAudioComponent->SetVolumeMultiplier(SoundscapeData->Volume);
        }
    }

    // Load music track
    if (!SoundscapeData->MusicTrack.IsNull())
    {
        USoundCue* MusicCue = SoundscapeData->MusicTrack.LoadSynchronous();
        if (MusicCue && MusicAudioComponent)
        {
            MusicAudioComponent->SetSound(MusicCue);
            MusicAudioComponent->SetVolumeMultiplier(SoundscapeData->Volume * 0.7f);
        }
    }
}

FAudio_SoundscapeData* UAudio_EnvironmentalSoundscape::GetSoundscapeData(EAudio_BiomeType BiomeType)
{
    for (FAudio_SoundscapeData& Data : BiomeSoundscapes)
    {
        if (Data.BiomeType == BiomeType)
        {
            return &Data;
        }
    }
    return nullptr;
}

void UAudio_EnvironmentalSoundscape::CompleteTransition()
{
    bIsTransitioning = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TransitionTimerHandle);
    }
}