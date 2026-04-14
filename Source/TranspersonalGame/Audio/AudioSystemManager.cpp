#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"

UAudioSystemManager::UAudioSystemManager()
{
    MasterVolume = 1.0f;
    bIsInitialized = false;
    BackgroundMusicComponent = nullptr;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing..."));
    
    InitializeCategoryVolumes();
    InitializeEnvironmentPresets();
    LoadAudioDatabase();
    
    // Set up cleanup timer for finished audio components
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UAudioSystemManager::CleanupFinishedAudioComponents,
            5.0f, // Every 5 seconds
            true  // Looping
        );
    }
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initialization complete"));
}

void UAudioSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Deinitializing..."));
    
    // Stop all active audio
    StopAllAmbientSounds(0.5f);
    StopBackgroundMusic(0.5f);
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    // Clean up audio components
    for (auto& Pair : ActiveAudioComponents)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->Stop();
            Pair.Value->DestroyComponent();
        }
    }
    ActiveAudioComponents.Empty();
    
    for (UAudioComponent* AmbientComp : AmbientAudioComponents)
    {
        if (IsValid(AmbientComp))
        {
            AmbientComp->Stop();
            AmbientComp->DestroyComponent();
        }
    }
    AmbientAudioComponents.Empty();
    
    if (IsValid(BackgroundMusicComponent))
    {
        BackgroundMusicComponent->Stop();
        BackgroundMusicComponent->DestroyComponent();
        BackgroundMusicComponent = nullptr;
    }
    
    bIsInitialized = false;
    
    Super::Deinitialize();
}

void UAudioSystemManager::PlaySound(const FString& SoundID, FVector Location, float VolumeMultiplier)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Not initialized, cannot play sound %s"), *SoundID);
        return;
    }
    
    const FAudio_SoundEntry* SoundEntry = RegisteredSounds.Find(SoundID);
    if (!SoundEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Sound ID %s not found in database"), *SoundID);
        return;
    }
    
    UAudioComponent* AudioComp = CreateAudioComponent(*SoundEntry, Location);
    if (AudioComp)
    {
        float FinalVolume = CalculateFinalVolume(*SoundEntry) * VolumeMultiplier;
        AudioComp->SetVolumeMultiplier(FinalVolume);
        AudioComp->Play();
        
        ActiveAudioComponents.Add(SoundID + FString::Printf(TEXT("_%d"), FMath::RandRange(1000, 9999)), AudioComp);
        
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing sound %s at volume %.2f"), *SoundID, FinalVolume);
    }
}

void UAudioSystemManager::StopSound(const FString& SoundID, float FadeOutTime)
{
    TArray<FString> KeysToRemove;
    
    for (auto& Pair : ActiveAudioComponents)
    {
        if (Pair.Key.StartsWith(SoundID))
        {
            if (IsValid(Pair.Value))
            {
                if (FadeOutTime > 0.0f)
                {
                    Pair.Value->FadeOut(FadeOutTime, 0.0f);
                }
                else
                {
                    Pair.Value->Stop();
                }
            }
            KeysToRemove.Add(Pair.Key);
        }
    }
    
    for (const FString& Key : KeysToRemove)
    {
        ActiveAudioComponents.Remove(Key);
    }
}

void UAudioSystemManager::PlayAmbientSound(const FString& SoundID, bool bLooping)
{
    const FAudio_SoundEntry* SoundEntry = RegisteredSounds.Find(SoundID);
    if (!SoundEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Ambient sound ID %s not found"), *SoundID);
        return;
    }
    
    UAudioComponent* AudioComp = CreateAudioComponent(*SoundEntry, FVector::ZeroVector);
    if (AudioComp)
    {
        AudioComp->bIsUISound = true; // Ambient sounds follow the listener
        AudioComp->SetVolumeMultiplier(CalculateFinalVolume(*SoundEntry));
        
        if (bLooping)
        {
            AudioComp->Play();
        }
        else
        {
            AudioComp->Play();
        }
        
        AmbientAudioComponents.Add(AudioComp);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing ambient sound %s"), *SoundID);
    }
}

void UAudioSystemManager::StopAllAmbientSounds(float FadeOutTime)
{
    for (UAudioComponent* AmbientComp : AmbientAudioComponents)
    {
        if (IsValid(AmbientComp))
        {
            if (FadeOutTime > 0.0f)
            {
                AmbientComp->FadeOut(FadeOutTime, 0.0f);
            }
            else
            {
                AmbientComp->Stop();
            }
        }
    }
    
    // Clear the array after a delay if fading out
    if (FadeOutTime > 0.0f && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]()
            {
                AmbientAudioComponents.Empty();
            },
            FadeOutTime + 0.5f,
            false
        );
    }
    else
    {
        AmbientAudioComponents.Empty();
    }
}

void UAudioSystemManager::SetEnvironmentType(EAudio_EnvironmentType NewEnvironment, float TransitionTime)
{
    if (CurrentEnvironment.EnvironmentType == NewEnvironment)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transitioning to environment type %d"), (int32)NewEnvironment);
    
    // Stop current ambient sounds
    StopAllAmbientSounds(TransitionTime * 0.5f);
    
    // Load new environment settings
    const FAudio_EnvironmentSettings* NewSettings = EnvironmentPresets.Find(NewEnvironment);
    if (NewSettings)
    {
        CurrentEnvironment = *NewSettings;
        
        // Start new ambient sounds after transition
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(
                FTimerHandle(),
                [this]()
                {
                    for (const FAudio_SoundEntry& AmbientSound : CurrentEnvironment.AmbientSounds)
                    {
                        PlayAmbientSound(AmbientSound.SoundID, AmbientSound.bLooping);
                    }
                    
                    // Play background music if specified
                    if (!CurrentEnvironment.BackgroundMusic.SoundID.IsEmpty())
                    {
                        PlayBackgroundMusic(CurrentEnvironment.BackgroundMusic.SoundID, true, TransitionTime * 0.5f);
                    }
                },
                TransitionTime * 0.6f,
                false
            );
        }
    }
}

void UAudioSystemManager::UpdateEnvironmentSettings(const FAudio_EnvironmentSettings& NewSettings)
{
    CurrentEnvironment = NewSettings;
    EnvironmentPresets.Add(NewSettings.EnvironmentType, NewSettings);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Updated environment settings for type %d"), (int32)NewSettings.EnvironmentType);
}

void UAudioSystemManager::PlayConsciousnessEvent(EConsciousnessLevel Level, float Intensity)
{
    FString SoundID;
    
    switch (Level)
    {
        case EConsciousnessLevel::Awakening:
            SoundID = TEXT("consciousness_awakening");
            break;
        case EConsciousnessLevel::Aware:
            SoundID = TEXT("consciousness_aware");
            break;
        case EConsciousnessLevel::Enlightened:
            SoundID = TEXT("consciousness_enlightened");
            break;
        case EConsciousnessLevel::Transcendent:
            SoundID = TEXT("consciousness_transcendent");
            break;
        default:
            SoundID = TEXT("consciousness_base");
            break;
    }
    
    PlaySound(SoundID, FVector::ZeroVector, Intensity);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing consciousness event for level %d at intensity %.2f"), (int32)Level, Intensity);
}

void UAudioSystemManager::TriggerMysticalTransition(float Duration)
{
    PlaySound(TEXT("mystical_transition"), FVector::ZeroVector, 1.0f);
    
    // Temporarily reduce other audio during mystical events
    float OriginalVolume = MasterVolume;
    SetMasterVolume(MasterVolume * 0.3f);
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this, OriginalVolume]()
            {
                SetMasterVolume(OriginalVolume);
            },
            Duration,
            false
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Triggered mystical transition for %.2f seconds"), Duration);
}

void UAudioSystemManager::PlayBackgroundMusic(const FString& MusicID, bool bCrossfade, float CrossfadeTime)
{
    const FAudio_SoundEntry* MusicEntry = RegisteredSounds.Find(MusicID);
    if (!MusicEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Music ID %s not found"), *MusicID);
        return;
    }
    
    // Stop current music if crossfading
    if (IsValid(BackgroundMusicComponent) && bCrossfade)
    {
        BackgroundMusicComponent->FadeOut(CrossfadeTime, 0.0f);
    }
    else if (IsValid(BackgroundMusicComponent))
    {
        BackgroundMusicComponent->Stop();
        BackgroundMusicComponent->DestroyComponent();
    }
    
    // Create new music component
    BackgroundMusicComponent = CreateAudioComponent(*MusicEntry, FVector::ZeroVector);
    if (BackgroundMusicComponent)
    {
        BackgroundMusicComponent->bIsUISound = true;
        BackgroundMusicComponent->SetVolumeMultiplier(CalculateFinalVolume(*MusicEntry));
        
        if (bCrossfade && CrossfadeTime > 0.0f)
        {
            BackgroundMusicComponent->FadeIn(CrossfadeTime, CalculateFinalVolume(*MusicEntry));
        }
        else
        {
            BackgroundMusicComponent->Play();
        }
        
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing background music %s"), *MusicID);
    }
}

void UAudioSystemManager::StopBackgroundMusic(float FadeOutTime)
{
    if (IsValid(BackgroundMusicComponent))
    {
        if (FadeOutTime > 0.0f)
        {
            BackgroundMusicComponent->FadeOut(FadeOutTime, 0.0f);
        }
        else
        {
            BackgroundMusicComponent->Stop();
        }
        
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopping background music"));
    }
}

void UAudioSystemManager::SetMusicVolume(float Volume, float FadeTime)
{
    if (IsValid(BackgroundMusicComponent))
    {
        if (FadeTime > 0.0f)
        {
            // Implement gradual volume change
            BackgroundMusicComponent->SetVolumeMultiplier(Volume);
        }
        else
        {
            BackgroundMusicComponent->SetVolumeMultiplier(Volume);
        }
    }
}

void UAudioSystemManager::RegisterSoundEntry(const FAudio_SoundEntry& SoundEntry)
{
    RegisteredSounds.Add(SoundEntry.SoundID, SoundEntry);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered sound %s"), *SoundEntry.SoundID);
}

void UAudioSystemManager::LoadAudioDatabase()
{
    // Register default consciousness sounds
    FAudio_SoundEntry ConsciousnessAwakening;
    ConsciousnessAwakening.SoundID = TEXT("consciousness_awakening");
    ConsciousnessAwakening.Category = EAudio_SoundCategory::Consciousness;
    ConsciousnessAwakening.Volume = 0.8f;
    ConsciousnessAwakening.bLooping = false;
    RegisterSoundEntry(ConsciousnessAwakening);
    
    FAudio_SoundEntry MysticalTransition;
    MysticalTransition.SoundID = TEXT("mystical_transition");
    MysticalTransition.Category = EAudio_SoundCategory::SFX;
    MysticalTransition.Volume = 1.0f;
    MysticalTransition.FadeInTime = 1.0f;
    MysticalTransition.FadeOutTime = 2.0f;
    RegisterSoundEntry(MysticalTransition);
    
    // Register forest ambient sounds
    FAudio_SoundEntry ForestAmbient;
    ForestAmbient.SoundID = TEXT("forest_ambient");
    ForestAmbient.Category = EAudio_SoundCategory::Ambient;
    ForestAmbient.Volume = 0.6f;
    ForestAmbient.bLooping = true;
    RegisterSoundEntry(ForestAmbient);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Loaded default audio database"));
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active audio components
    for (auto& Pair : ActiveAudioComponents)
    {
        if (IsValid(Pair.Value))
        {
            // Recalculate volume based on new master volume
            // Note: This is a simplified approach - in production you'd store original volumes
            Pair.Value->SetVolumeMultiplier(Pair.Value->VolumeMultiplier * MasterVolume);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set master volume to %.2f"), MasterVolume);
}

void UAudioSystemManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set category %d volume to %.2f"), (int32)Category, Volume);
}

float UAudioSystemManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    const float* Volume = CategoryVolumes.Find(Category);
    return Volume ? *Volume : 1.0f;
}

UAudioComponent* UAudioSystemManager::CreateAudioComponent(const FAudio_SoundEntry& SoundEntry, FVector Location)
{
    if (!SoundEntry.SoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Invalid sound asset for %s"), *SoundEntry.SoundID);
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No valid world"));
        return nullptr;
    }
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(World);
    if (!AudioComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Failed to create audio component"));
        return nullptr;
    }
    
    AudioComp->SetSound(SoundEntry.SoundAsset.LoadSynchronous());
    AudioComp->SetWorldLocation(Location);
    AudioComp->SetPitchMultiplier(SoundEntry.Pitch);
    AudioComp->bAutoDestroy = !SoundEntry.bLooping;
    
    return AudioComp;
}

void UAudioSystemManager::CleanupFinishedAudioComponents()
{
    TArray<FString> KeysToRemove;
    
    for (auto& Pair : ActiveAudioComponents)
    {
        if (!IsValid(Pair.Value) || !Pair.Value->IsPlaying())
        {
            KeysToRemove.Add(Pair.Key);
            if (IsValid(Pair.Value))
            {
                Pair.Value->DestroyComponent();
            }
        }
    }
    
    for (const FString& Key : KeysToRemove)
    {
        ActiveAudioComponents.Remove(Key);
    }
    
    // Clean up ambient components
    AmbientAudioComponents.RemoveAll([](UAudioComponent* Comp)
    {
        if (!IsValid(Comp) || !Comp->IsPlaying())
        {
            if (IsValid(Comp))
            {
                Comp->DestroyComponent();
            }
            return true;
        }
        return false;
    });
}

void UAudioSystemManager::InitializeEnvironmentPresets()
{
    // Forest Environment
    FAudio_EnvironmentSettings ForestSettings;
    ForestSettings.EnvironmentType = EAudio_EnvironmentType::Forest;
    ForestSettings.MasterVolume = 1.0f;
    ForestSettings.ReverbIntensity = 0.3f;
    
    FAudio_SoundEntry ForestAmbient;
    ForestAmbient.SoundID = TEXT("forest_ambient");
    ForestAmbient.Category = EAudio_SoundCategory::Ambient;
    ForestAmbient.Volume = 0.6f;
    ForestAmbient.bLooping = true;
    ForestSettings.AmbientSounds.Add(ForestAmbient);
    
    EnvironmentPresets.Add(EAudio_EnvironmentType::Forest, ForestSettings);
    
    // Cave Environment
    FAudio_EnvironmentSettings CaveSettings;
    CaveSettings.EnvironmentType = EAudio_EnvironmentType::Cave;
    CaveSettings.MasterVolume = 0.8f;
    CaveSettings.ReverbIntensity = 0.8f;
    EnvironmentPresets.Add(EAudio_EnvironmentType::Cave, CaveSettings);
    
    // Set default environment
    CurrentEnvironment = ForestSettings;
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initialized environment presets"));
}

void UAudioSystemManager::InitializeCategoryVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Consciousness, 1.2f);
}

float UAudioSystemManager::CalculateFinalVolume(const FAudio_SoundEntry& SoundEntry) const
{
    float CategoryVolume = GetCategoryVolume(SoundEntry.Category);
    return SoundEntry.Volume * CategoryVolume * MasterVolume;
}