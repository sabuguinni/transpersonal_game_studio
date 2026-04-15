#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"

UAudio_AudioSystemManager::UAudio_AudioSystemManager()
{
    bIsInitialized = false;
    CurrentBiome = EAudio_BiomeAmbience::Forest;
    TargetBiome = EAudio_BiomeAmbience::Forest;
    CurrentTimeOfDay = EAudio_TimeOfDay::Midday;
    bIsTransitioningBiome = false;
    BiomeTransitionTime = 0.0f;
    BiomeTransitionDuration = 2.0f;
    
    CurrentMusicComponent = nullptr;
    CurrentAmbienceComponent = nullptr;
    
    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    SFXVolume = 1.0f;
    AmbienceVolume = 0.8f;
    
    RandomSoundTimer = 0.0f;
    NextRandomSoundTime = 10.0f;
}

void UAudio_AudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Initializing..."));
    
    InitializeAudioSystem();
}

void UAudio_AudioSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Shutting down..."));
    
    ShutdownAudioSystem();
    
    Super::Deinitialize();
}

void UAudio_AudioSystemManager::InitializeAudioSystem()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio System Manager: Already initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Setting up audio system..."));
    
    // Initialize biome audio data
    LoadBiomeAudioData();
    
    // Set initial biome
    SetCurrentBiome(EAudio_BiomeAmbience::Forest, 0.0f);
    SetTimeOfDay(EAudio_TimeOfDay::Midday);
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Initialization complete"));
}

void UAudio_AudioSystemManager::ShutdownAudioSystem()
{
    if (!bIsInitialized)
    {
        return;
    }
    
    // Stop all active audio
    StopMusic(0.0f);
    
    // Clean up all audio components
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
    }
    ActiveAudioComponents.Empty();
    
    CurrentMusicComponent = nullptr;
    CurrentAmbienceComponent = nullptr;
    
    bIsInitialized = false;
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Shutdown complete"));
}

void UAudio_AudioSystemManager::UpdateAudioSystem(float DeltaTime)
{
    if (!bIsInitialized)
    {
        return;
    }
    
    // Update biome transitions
    UpdateBiomeAudio(DeltaTime);
    
    // Update random environmental sounds
    UpdateRandomSounds(DeltaTime);
    
    // Clean up finished audio components
    CleanupFinishedAudioComponents();
}

void UAudio_AudioSystemManager::SetCurrentBiome(EAudio_BiomeAmbience NewBiome, float TransitionTime)
{
    if (NewBiome == CurrentBiome && !bIsTransitioningBiome)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Transitioning to biome %d"), (int32)NewBiome);
    
    TargetBiome = NewBiome;
    BiomeTransitionDuration = FMath::Max(TransitionTime, 0.1f);
    BiomeTransitionTime = 0.0f;
    bIsTransitioningBiome = true;
}

void UAudio_AudioSystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay)
{
    if (NewTimeOfDay == CurrentTimeOfDay)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Time of day changed to %d"), (int32)NewTimeOfDay);
    
    CurrentTimeOfDay = NewTimeOfDay;
    
    // Adjust ambient volume based on time of day
    float TimeBasedVolume = AmbienceVolume;
    switch (CurrentTimeOfDay)
    {
        case EAudio_TimeOfDay::Dawn:
        case EAudio_TimeOfDay::Dusk:
            TimeBasedVolume *= 0.8f;
            break;
        case EAudio_TimeOfDay::Night:
            TimeBasedVolume *= 0.6f;
            break;
        default:
            break;
    }
    
    if (CurrentAmbienceComponent && IsValid(CurrentAmbienceComponent))
    {
        CurrentAmbienceComponent->SetVolumeMultiplier(TimeBasedVolume);
    }
}

UAudioComponent* UAudio_AudioSystemManager::PlaySound2D(USoundBase* Sound, const FAudio_SoundSettings& Settings)
{
    if (!Sound || !bIsInitialized)
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(
        GetWorld(),
        Sound,
        Settings.Volume * SFXVolume * MasterVolume,
        Settings.Pitch,
        Settings.FadeInTime
    );
    
    if (AudioComp)
    {
        AudioComp->bAutoDestroy = !Settings.bLooping;
        ActiveAudioComponents.Add(AudioComp);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Audio System Manager: Playing 2D sound"));
    }
    
    return AudioComp;
}

UAudioComponent* UAudio_AudioSystemManager::PlaySound3D(USoundBase* Sound, FVector Location, const FAudio_SoundSettings& Settings)
{
    if (!Sound || !bIsInitialized)
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        FRotator::ZeroRotator,
        Settings.Volume * SFXVolume * MasterVolume,
        Settings.Pitch,
        Settings.FadeInTime,
        nullptr,
        nullptr,
        !Settings.bLooping
    );
    
    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Audio System Manager: Playing 3D sound at location %s"), 
               *Location.ToString());
    }
    
    return AudioComp;
}

void UAudio_AudioSystemManager::StopSound(UAudioComponent* AudioComponent, float FadeOutTime)
{
    if (!AudioComponent || !IsValid(AudioComponent))
    {
        return;
    }
    
    if (FadeOutTime > 0.0f)
    {
        AudioComponent->FadeOut(FadeOutTime, 0.0f);
    }
    else
    {
        AudioComponent->Stop();
    }
    
    // Remove from active components list
    ActiveAudioComponents.Remove(AudioComponent);
}

void UAudio_AudioSystemManager::PlayMusic(UMetaSoundSource* MusicTrack, float FadeInTime)
{
    if (!MusicTrack || !bIsInitialized)
    {
        return;
    }
    
    // Stop current music if playing
    if (CurrentMusicComponent && IsValid(CurrentMusicComponent))
    {
        StopMusic(FadeInTime * 0.5f);
    }
    
    // Play new music
    CurrentMusicComponent = UGameplayStatics::SpawnSound2D(
        GetWorld(),
        MusicTrack,
        MusicVolume * MasterVolume,
        1.0f,
        FadeInTime
    );
    
    if (CurrentMusicComponent)
    {
        CurrentMusicComponent->bAutoDestroy = false;
        ActiveAudioComponents.Add(CurrentMusicComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Playing music track"));
    }
}

void UAudio_AudioSystemManager::StopMusic(float FadeOutTime)
{
    if (CurrentMusicComponent && IsValid(CurrentMusicComponent))
    {
        if (FadeOutTime > 0.0f)
        {
            CurrentMusicComponent->FadeOut(FadeOutTime, 0.0f);
        }
        else
        {
            CurrentMusicComponent->Stop();
        }
        
        ActiveAudioComponents.Remove(CurrentMusicComponent);
        CurrentMusicComponent = nullptr;
        
        UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Stopped music"));
    }
}

void UAudio_AudioSystemManager::CrossfadeMusic(UMetaSoundSource* NewTrack, float CrossfadeTime)
{
    if (!NewTrack || !bIsInitialized)
    {
        return;
    }
    
    // Start new track with fade in
    PlayMusic(NewTrack, CrossfadeTime);
}

void UAudio_AudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active audio components
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp))
        {
            // Note: This is a simplified approach - in a full implementation,
            // we'd need to track original volumes and apply master volume as a multiplier
            AudioComp->SetVolumeMultiplier(MasterVolume);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Master volume set to %f"), MasterVolume);
}

void UAudio_AudioSystemManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (CurrentMusicComponent && IsValid(CurrentMusicComponent))
    {
        CurrentMusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Music volume set to %f"), MusicVolume);
}

void UAudio_AudioSystemManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: SFX volume set to %f"), SFXVolume);
}

void UAudio_AudioSystemManager::SetAmbienceVolume(float Volume)
{
    AmbienceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (CurrentAmbienceComponent && IsValid(CurrentAmbienceComponent))
    {
        CurrentAmbienceComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Ambience volume set to %f"), AmbienceVolume);
}

void UAudio_AudioSystemManager::UpdateBiomeAudio(float DeltaTime)
{
    if (!bIsTransitioningBiome)
    {
        return;
    }
    
    BiomeTransitionTime += DeltaTime;
    float TransitionAlpha = BiomeTransitionTime / BiomeTransitionDuration;
    
    if (TransitionAlpha >= 1.0f)
    {
        // Transition complete
        CurrentBiome = TargetBiome;
        bIsTransitioningBiome = false;
        TransitionBiomeAudio();
        
        UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Biome transition complete"));
    }
    else
    {
        // Update transition volumes
        if (CurrentAmbienceComponent && IsValid(CurrentAmbienceComponent))
        {
            float FadeOutVolume = (1.0f - TransitionAlpha) * AmbienceVolume * MasterVolume;
            CurrentAmbienceComponent->SetVolumeMultiplier(FadeOutVolume);
        }
    }
}

void UAudio_AudioSystemManager::UpdateRandomSounds(float DeltaTime)
{
    RandomSoundTimer += DeltaTime;
    
    if (RandomSoundTimer >= NextRandomSoundTime)
    {
        // Play a random environmental sound for current biome
        if (BiomeAudioData.Contains(CurrentBiome))
        {
            const FAudio_BiomeAudioData& BiomeData = BiomeAudioData[CurrentBiome];
            if (BiomeData.RandomSounds.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, BiomeData.RandomSounds.Num() - 1);
                USoundCue* RandomSound = BiomeData.RandomSounds[RandomIndex].LoadSynchronous();
                
                if (RandomSound)
                {
                    FAudio_SoundSettings Settings;
                    Settings.Volume = FMath::RandRange(0.3f, 0.8f);
                    Settings.b3DSound = true;
                    
                    // Play at a random location around the player
                    FVector PlayerLocation = FVector::ZeroVector;
                    if (GetWorld() && GetWorld()->GetFirstPlayerController())
                    {
                        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
                        if (PlayerPawn)
                        {
                            PlayerLocation = PlayerPawn->GetActorLocation();
                        }
                    }
                    
                    FVector RandomOffset = FVector(
                        FMath::RandRange(-500.0f, 500.0f),
                        FMath::RandRange(-500.0f, 500.0f),
                        FMath::RandRange(-100.0f, 100.0f)
                    );
                    
                    PlaySound3D(RandomSound, PlayerLocation + RandomOffset, Settings);
                }
            }
            
            // Set next random sound time
            NextRandomSoundTime = FMath::RandRange(
                BiomeData.RandomSoundInterval * 0.5f,
                BiomeData.RandomSoundInterval * 1.5f
            );
        }
        else
        {
            NextRandomSoundTime = 10.0f; // Default interval
        }
        
        RandomSoundTimer = 0.0f;
    }
}

void UAudio_AudioSystemManager::TransitionBiomeAudio()
{
    // Stop current ambience
    if (CurrentAmbienceComponent && IsValid(CurrentAmbienceComponent))
    {
        CurrentAmbienceComponent->Stop();
        ActiveAudioComponents.Remove(CurrentAmbienceComponent);
        CurrentAmbienceComponent = nullptr;
    }
    
    // Start new biome ambience
    if (BiomeAudioData.Contains(CurrentBiome))
    {
        const FAudio_BiomeAudioData& BiomeData = BiomeAudioData[CurrentBiome];
        if (BiomeData.AmbienceLoops.Num() > 0)
        {
            // Pick a random ambience loop for this biome
            int32 RandomIndex = FMath::RandRange(0, BiomeData.AmbienceLoops.Num() - 1);
            USoundCue* AmbienceSound = BiomeData.AmbienceLoops[RandomIndex].LoadSynchronous();
            
            if (AmbienceSound)
            {
                CurrentAmbienceComponent = UGameplayStatics::SpawnSound2D(
                    GetWorld(),
                    AmbienceSound,
                    AmbienceVolume * MasterVolume,
                    1.0f,
                    BiomeData.MusicCrossfadeTime
                );
                
                if (CurrentAmbienceComponent)
                {
                    CurrentAmbienceComponent->bAutoDestroy = false;
                    ActiveAudioComponents.Add(CurrentAmbienceComponent);
                }
            }
        }
    }
}

void UAudio_AudioSystemManager::LoadBiomeAudioData()
{
    // Initialize biome audio data with default values
    // In a full implementation, this would load from data assets or configuration files
    
    for (int32 i = 0; i < (int32)EAudio_BiomeAmbience::Cave + 1; ++i)
    {
        EAudio_BiomeAmbience BiomeType = (EAudio_BiomeAmbience)i;
        FAudio_BiomeAudioData BiomeData;
        BiomeData.BiomeType = BiomeType;
        
        switch (BiomeType)
        {
            case EAudio_BiomeAmbience::Forest:
                BiomeData.RandomSoundInterval = 8.0f;
                break;
            case EAudio_BiomeAmbience::Desert:
                BiomeData.RandomSoundInterval = 15.0f;
                break;
            case EAudio_BiomeAmbience::Swamp:
                BiomeData.RandomSoundInterval = 6.0f;
                break;
            case EAudio_BiomeAmbience::Cave:
                BiomeData.RandomSoundInterval = 20.0f;
                break;
            default:
                BiomeData.RandomSoundInterval = 10.0f;
                break;
        }
        
        BiomeAudioData.Add(BiomeType, BiomeData);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Manager: Loaded biome audio data for %d biomes"), 
           BiomeAudioData.Num());
}

void UAudio_AudioSystemManager::CleanupFinishedAudioComponents()
{
    // Remove any audio components that have finished playing
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; --i)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (!AudioComp || !IsValid(AudioComp) || !AudioComp->IsPlaying())
        {
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

UAudioComponent* UAudio_AudioSystemManager::CreateAudioComponent(USoundBase* Sound)
{
    if (!Sound || !GetWorld())
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(this);
    if (AudioComp)
    {
        AudioComp->SetSound(Sound);
        AudioComp->bAutoActivate = false;
        AudioComp->bAutoDestroy = true;
    }
    
    return AudioComp;
}