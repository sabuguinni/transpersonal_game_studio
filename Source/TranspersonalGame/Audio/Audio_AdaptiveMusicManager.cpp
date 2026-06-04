#include "Audio_AdaptiveMusicManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAudio_AdaptiveMusicManager::UAudio_AdaptiveMusicManager()
{
    // Initialize default music state
    CurrentMusicState.CurrentLayer = EAudio_MusicLayer::Ambient;
    CurrentMusicState.CurrentBiome = EAudio_BiomeType::Forest;
    CurrentMusicState.IntensityLevel = 0.0f;
    CurrentMusicState.CrossfadeTime = 2.0f;
    CurrentMusicState.bIsInCombat = false;
    CurrentMusicState.bDangerNearby = false;

    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    AmbienceVolume = 0.5f;
    LastIntensityUpdate = 0.0f;
    IntensityUpdateInterval = 1.0f;
}

void UAudio_AdaptiveMusicManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: Initializing adaptive music system"));

    // Create audio components
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UWorld* World = GameInstance->GetWorld())
        {
            // Create music audio component
            MusicAudioComponent = NewObject<UAudioComponent>(this);
            if (MusicAudioComponent)
            {
                MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
                MusicAudioComponent->bAutoActivate = false;
                MusicAudioComponent->RegisterComponent();
            }

            // Create ambience audio component
            AmbienceAudioComponent = NewObject<UAudioComponent>(this);
            if (AmbienceAudioComponent)
            {
                AmbienceAudioComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
                AmbienceAudioComponent->bAutoActivate = false;
                AmbienceAudioComponent->RegisterComponent();
            }

            UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: Audio components created"));
        }
    }

    // Initialize with forest ambience
    SetBiome(EAudio_BiomeType::Forest);
    SetMusicLayer(EAudio_MusicLayer::Ambient);
}

void UAudio_AdaptiveMusicManager::Deinitialize()
{
    if (MusicAudioComponent)
    {
        MusicAudioComponent->Stop();
        MusicAudioComponent = nullptr;
    }

    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->Stop();
        AmbienceAudioComponent = nullptr;
    }

    Super::Deinitialize();
}

void UAudio_AdaptiveMusicManager::SetMusicLayer(EAudio_MusicLayer NewLayer, float CrossfadeTime)
{
    if (CurrentMusicState.CurrentLayer == NewLayer)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: Changing music layer to %d"), (int32)NewLayer);

    CurrentMusicState.CurrentLayer = NewLayer;
    CurrentMusicState.CrossfadeTime = CrossfadeTime;

    CrossfadeToLayer(NewLayer, CrossfadeTime);
}

void UAudio_AdaptiveMusicManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentMusicState.CurrentBiome == NewBiome)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: Changing biome to %d"), (int32)NewBiome);

    CurrentMusicState.CurrentBiome = NewBiome;
    UpdateBiomeAmbience();
}

void UAudio_AdaptiveMusicManager::SetIntensity(float NewIntensity)
{
    NewIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    
    if (FMath::Abs(CurrentMusicState.IntensityLevel - NewIntensity) < 0.1f)
    {
        return;
    }

    CurrentMusicState.IntensityLevel = NewIntensity;

    // Automatically adjust music layer based on intensity
    if (NewIntensity > 0.8f && !CurrentMusicState.bIsInCombat)
    {
        SetMusicLayer(EAudio_MusicLayer::Danger);
    }
    else if (NewIntensity > 0.6f)
    {
        SetMusicLayer(EAudio_MusicLayer::Tension);
    }
    else if (NewIntensity > 0.3f)
    {
        SetMusicLayer(EAudio_MusicLayer::Exploration);
    }
    else
    {
        SetMusicLayer(EAudio_MusicLayer::Ambient);
    }

    // Adjust volume based on intensity
    if (MusicAudioComponent)
    {
        float IntensityVolume = FMath::Lerp(0.3f, 1.0f, NewIntensity);
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume * IntensityVolume);
    }
}

void UAudio_AdaptiveMusicManager::OnCombatStart()
{
    UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: Combat started"));

    CurrentMusicState.bIsInCombat = true;
    SetMusicLayer(EAudio_MusicLayer::Combat, 0.5f); // Fast transition to combat music
    SetIntensity(1.0f);
}

void UAudio_AdaptiveMusicManager::OnCombatEnd()
{
    UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: Combat ended"));

    CurrentMusicState.bIsInCombat = false;
    
    // Return to appropriate layer based on current situation
    if (CurrentMusicState.bDangerNearby)
    {
        SetMusicLayer(EAudio_MusicLayer::Tension, 3.0f);
        SetIntensity(0.7f);
    }
    else
    {
        SetMusicLayer(EAudio_MusicLayer::Ambient, 4.0f);
        SetIntensity(0.2f);
    }
}

void UAudio_AdaptiveMusicManager::OnDangerDetected(bool bDangerous)
{
    CurrentMusicState.bDangerNearby = bDangerous;

    if (bDangerous && !CurrentMusicState.bIsInCombat)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: Danger detected"));
        SetMusicLayer(EAudio_MusicLayer::Tension);
        SetIntensity(0.8f);
    }
    else if (!bDangerous && !CurrentMusicState.bIsInCombat)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: Danger cleared"));
        SetMusicLayer(EAudio_MusicLayer::Exploration);
        SetIntensity(0.4f);
    }
}

void UAudio_AdaptiveMusicManager::UpdateMusicState(float DeltaTime)
{
    LastIntensityUpdate += DeltaTime;

    if (LastIntensityUpdate >= IntensityUpdateInterval)
    {
        CalculateAdaptiveIntensity();
        LastIntensityUpdate = 0.0f;
    }
}

void UAudio_AdaptiveMusicManager::CrossfadeToLayer(EAudio_MusicLayer NewLayer, float CrossfadeTime)
{
    if (!MusicAudioComponent)
    {
        return;
    }

    // Find the sound cue for the new layer
    if (USoundCue** FoundCue = MusicLayers.Find(NewLayer))
    {
        if (USoundCue* NewCue = FoundCue->LoadSynchronous())
        {
            // Stop current music
            MusicAudioComponent->Stop();
            
            // Set new sound and play
            MusicAudioComponent->SetSound(NewCue);
            MusicAudioComponent->Play();

            UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: Started playing new music layer"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: No sound cue found for layer %d"), (int32)NewLayer);
    }
}

void UAudio_AdaptiveMusicManager::UpdateBiomeAmbience()
{
    if (!AmbienceAudioComponent)
    {
        return;
    }

    // Find the ambience sound for the current biome
    if (USoundCue** FoundCue = BiomeAmbience.Find(CurrentMusicState.CurrentBiome))
    {
        if (USoundCue* NewCue = FoundCue->LoadSynchronous())
        {
            // Stop current ambience
            AmbienceAudioComponent->Stop();
            
            // Set new ambience and play
            AmbienceAudioComponent->SetSound(NewCue);
            AmbienceAudioComponent->Play();

            UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: Started playing biome ambience"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_AdaptiveMusicManager: No ambience sound found for biome %d"), (int32)CurrentMusicState.CurrentBiome);
    }
}

void UAudio_AdaptiveMusicManager::CalculateAdaptiveIntensity()
{
    // This would normally calculate intensity based on game state
    // For now, we maintain current intensity unless explicitly changed
    
    // Example: Check for nearby enemies, player health, etc.
    // float NewIntensity = CalculateIntensityFromGameState();
    // SetIntensity(NewIntensity);
}