#include "AudioManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

UAudioManager::UAudioManager()
{
    // Initialize default values
    CurrentBiome = EAudio_BiomeAmbience::Forest;
    CurrentTension = EAudio_TensionLevel::Calm;
    CurrentTimeOfDay = EAudio_TimeOfDay::Morning;
    
    MasterVolume = 1.0f;
    AmbienceVolume = 0.8f;
    EffectsVolume = 1.0f;
    MusicVolume = 0.6f;
}

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Initializing prehistoric audio system"));
    
    // Create audio components
    if (UWorld* World = GetWorld())
    {
        // Create ambience component
        AmbienceComponent = NewObject<UAudioComponent>(this);
        if (AmbienceComponent)
        {
            AmbienceComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
            AmbienceComponent->bAutoActivate = false;
        }
        
        // Create music component
        MusicComponent = NewObject<UAudioComponent>(this);
        if (MusicComponent)
        {
            MusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
            MusicComponent->bAutoActivate = false;
        }
        
        // Create tension component
        TensionComponent = NewObject<UAudioComponent>(this);
        if (TensionComponent)
        {
            TensionComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
            TensionComponent->bAutoActivate = false;
        }
        
        // Setup default biome settings
        FAudio_BiomeSettings ForestSettings;
        ForestSettings.BiomeType = EAudio_BiomeAmbience::Forest;
        ForestSettings.RandomElementChance = 0.15f;
        BiomeSettings.Add(EAudio_BiomeAmbience::Forest, ForestSettings);
        
        FAudio_BiomeSettings SwampSettings;
        SwampSettings.BiomeType = EAudio_BiomeAmbience::Swamp;
        SwampSettings.RandomElementChance = 0.1f;
        BiomeSettings.Add(EAudio_BiomeAmbience::Swamp, SwampSettings);
        
        // Setup tension layers with different intensity
        FAudio_SoundLayer CalmLayer;
        CalmLayer.Volume = 0.3f;
        CalmLayer.FadeInTime = 5.0f;
        CalmLayer.FadeOutTime = 3.0f;
        TensionLayers.Add(EAudio_TensionLevel::Calm, CalmLayer);
        
        FAudio_SoundLayer DangerLayer;
        DangerLayer.Volume = 0.8f;
        DangerLayer.Pitch = 1.2f;
        DangerLayer.FadeInTime = 1.0f;
        DangerLayer.FadeOutTime = 2.0f;
        TensionLayers.Add(EAudio_TensionLevel::Danger, DangerLayer);
        
        FAudio_SoundLayer TerrorLayer;
        TerrorLayer.Volume = 1.0f;
        TerrorLayer.Pitch = 1.5f;
        TerrorLayer.FadeInTime = 0.5f;
        TerrorLayer.FadeOutTime = 1.0f;
        TensionLayers.Add(EAudio_TensionLevel::Terror, TerrorLayer);
        
        // Start with forest ambience
        SetCurrentBiome(EAudio_BiomeAmbience::Forest);
        
        // Setup random element timer
        if (FTimerManager* TimerManager = &World->GetTimerManager())
        {
            TimerManager->SetTimer(RandomElementTimer, this, &UAudioManager::PlayRandomAmbienceElement, 
                                 FMath::RandRange(10.0f, 30.0f), true);
        }
    }
}

void UAudioManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Shutting down audio system"));
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        if (FTimerManager* TimerManager = &World->GetTimerManager())
        {
            TimerManager->ClearTimer(RandomElementTimer);
            TimerManager->ClearTimer(TensionUpdateTimer);
        }
    }
    
    // Stop all audio components
    if (AmbienceComponent && AmbienceComponent->IsPlaying())
    {
        AmbienceComponent->Stop();
    }
    
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        MusicComponent->Stop();
    }
    
    if (TensionComponent && TensionComponent->IsPlaying())
    {
        TensionComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UAudioManager::SetCurrentBiome(EAudio_BiomeAmbience NewBiome)
{
    if (CurrentBiome == NewBiome)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Changing biome from %d to %d"), 
           (int32)CurrentBiome, (int32)NewBiome);
    
    CurrentBiome = NewBiome;
    UpdateAmbienceLayer();
}

void UAudioManager::SetTensionLevel(EAudio_TensionLevel NewTension)
{
    if (CurrentTension == NewTension)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Tension level changed from %d to %d"), 
           (int32)CurrentTension, (int32)NewTension);
    
    CurrentTension = NewTension;
    UpdateTensionLayer();
}

void UAudioManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (CurrentTimeOfDay == NewTime)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Time of day changed to %d"), (int32)NewTime);
    
    CurrentTimeOfDay = NewTime;
    UpdateAmbienceLayer();
}

void UAudioManager::PlayDinosaurSound(const FString& DinosaurType, const FVector& Location)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Playing dinosaur sound for %s at location"), *DinosaurType);
    
    // For now, just log the request - actual sound cues would be loaded here
    if (DinosaurType.Contains(TEXT("TRex")))
    {
        // Trigger high tension when T-Rex is nearby
        SetTensionLevel(EAudio_TensionLevel::Terror);
    }
    else if (DinosaurType.Contains(TEXT("Raptor")))
    {
        // Raptors create danger level tension
        SetTensionLevel(EAudio_TensionLevel::Danger);
    }
    else if (DinosaurType.Contains(TEXT("Brachiosaurus")))
    {
        // Gentle giants might actually reduce tension
        if (CurrentTension > EAudio_TensionLevel::Cautious)
        {
            SetTensionLevel(EAudio_TensionLevel::Cautious);
        }
    }
}

void UAudioManager::PlayFootstepSound(const FString& SurfaceType, const FVector& Location)
{
    // Play appropriate footstep sound based on surface
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing footstep on %s"), *SurfaceType);
    
    // This would play the actual footstep sound effect
    // UGameplayStatics::PlaySoundAtLocation(GetWorld(), FootstepSound, Location);
}

void UAudioManager::PlayCraftingSound(const FString& MaterialType, const FVector& Location)
{
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing crafting sound for %s"), *MaterialType);
    
    // Play crafting sound based on material type (stone, wood, bone, etc.)
}

void UAudioManager::OnPlayerHealthChanged(float HealthPercentage)
{
    // Adjust audio based on health - heartbeat intensity, etc.
    if (HealthPercentage < 0.2f)
    {
        // Critical health - increase tension
        if (CurrentTension < EAudio_TensionLevel::Alert)
        {
            SetTensionLevel(EAudio_TensionLevel::Alert);
        }
    }
}

void UAudioManager::OnPlayerFearChanged(float FearLevel)
{
    // Adjust tension based on fear level
    if (FearLevel > 0.8f)
    {
        SetTensionLevel(EAudio_TensionLevel::Terror);
    }
    else if (FearLevel > 0.6f)
    {
        SetTensionLevel(EAudio_TensionLevel::Danger);
    }
    else if (FearLevel > 0.3f)
    {
        SetTensionLevel(EAudio_TensionLevel::Alert);
    }
    else if (FearLevel > 0.1f)
    {
        SetTensionLevel(EAudio_TensionLevel::Cautious);
    }
    else
    {
        SetTensionLevel(EAudio_TensionLevel::Calm);
    }
}

void UAudioManager::OnDinosaurNearby(const FString& DinosaurType, float Distance)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: %s detected at distance %f"), *DinosaurType, Distance);
    
    // Adjust tension based on dinosaur proximity
    if (Distance < 500.0f) // Very close
    {
        PlayDinosaurSound(DinosaurType, FVector::ZeroVector);
    }
    else if (Distance < 2000.0f) // Nearby
    {
        if (DinosaurType.Contains(TEXT("TRex")))
        {
            SetTensionLevel(EAudio_TensionLevel::Danger);
        }
        else if (DinosaurType.Contains(TEXT("Raptor")))
        {
            SetTensionLevel(EAudio_TensionLevel::Alert);
        }
    }
}

void UAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all component volumes
    if (AmbienceComponent)
    {
        AmbienceComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
    }
    if (MusicComponent)
    {
        MusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    if (TensionComponent)
    {
        TensionComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
}

void UAudioManager::SetAmbienceVolume(float Volume)
{
    AmbienceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (AmbienceComponent)
    {
        AmbienceComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
    }
}

void UAudioManager::SetEffectsVolume(float Volume)
{
    EffectsVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    // Effects volume would be applied to individual sound effects
}

void UAudioManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (MusicComponent)
    {
        MusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    if (TensionComponent)
    {
        TensionComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
}

void UAudioManager::UpdateAmbienceLayer()
{
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Updating ambience layer for biome %d"), (int32)CurrentBiome);
    
    // This would load and play the appropriate ambience sound for the current biome
    // Taking into account time of day variations
    
    if (AmbienceComponent)
    {
        // Stop current ambience
        if (AmbienceComponent->IsPlaying())
        {
            AmbienceComponent->FadeOut(2.0f, 0.0f);
        }
        
        // Start new ambience based on biome and time of day
        // This would load the actual sound cue and start playing it
    }
}

void UAudioManager::UpdateTensionLayer()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Updating tension layer to level %d"), (int32)CurrentTension);
    
    if (TensionComponent)
    {
        if (CurrentTension == EAudio_TensionLevel::Calm)
        {
            // Fade out tension music
            if (TensionComponent->IsPlaying())
            {
                TensionComponent->FadeOut(3.0f, 0.0f);
            }
        }
        else
        {
            // Play appropriate tension layer
            const FAudio_SoundLayer* TensionLayer = TensionLayers.Find(CurrentTension);
            if (TensionLayer)
            {
                TensionComponent->SetVolumeMultiplier(TensionLayer->Volume * MusicVolume * MasterVolume);
                TensionComponent->SetPitchMultiplier(TensionLayer->Pitch);
                
                // This would load and play the actual tension music
                // TensionComponent->SetSound(TensionLayer->SoundCue.LoadSynchronous());
                // TensionComponent->FadeIn(TensionLayer->FadeInTime, TensionLayer->Volume);
            }
        }
    }
}

void UAudioManager::UpdateMusicLayer()
{
    // Update background music based on current state
    // This would handle the main musical score
}

void UAudioManager::PlayRandomAmbienceElement()
{
    // Play random ambient sounds like bird calls, insect sounds, etc.
    const FAudio_BiomeSettings* Settings = BiomeSettings.Find(CurrentBiome);
    if (Settings && FMath::RandRange(0.0f, 1.0f) < Settings->RandomElementChance)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Playing random ambience element for %d"), (int32)CurrentBiome);
        
        // This would play a random ambient sound effect
        // based on the current biome and time of day
    }
    
    // Schedule next random element
    if (UWorld* World = GetWorld())
    {
        if (FTimerManager* TimerManager = &World->GetTimerManager())
        {
            TimerManager->SetTimer(RandomElementTimer, this, &UAudioManager::PlayRandomAmbienceElement, 
                                 FMath::RandRange(15.0f, 45.0f), false);
        }
    }
}