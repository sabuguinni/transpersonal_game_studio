#include "Audio_MetaSoundManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "MetasoundSource.h"
#include "Engine/Engine.h"

UAudio_MetaSoundManager::UAudio_MetaSoundManager()
{
    CurrentAmbienceType = EAudio_AmbienceType::Forest;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    MasterVolume = 1.0f;
    AmbienceVolume = 0.8f;
    SFXVolume = 1.0f;
    VoiceVolume = 1.0f;
    NarrationComponent = nullptr;
}

void UAudio_MetaSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Initializing adaptive audio system"));
    
    InitializeAmbienceConfigs();
    InitializeDinosaurSounds();
    InitializeNarrationSounds();
    
    // Create narration component
    NarrationComponent = CreateAudioComponent();
    if (NarrationComponent)
    {
        NarrationComponent->SetVolumeMultiplier(VoiceVolume * MasterVolume);
    }
}

void UAudio_MetaSoundManager::Deinitialize()
{
    // Clean up audio components
    for (UAudioComponent* Component : AmbienceComponents)
    {
        if (Component && IsValid(Component))
        {
            Component->Stop();
            Component->DestroyComponent();
        }
    }
    AmbienceComponents.Empty();
    
    if (NarrationComponent && IsValid(NarrationComponent))
    {
        NarrationComponent->Stop();
        NarrationComponent->DestroyComponent();
        NarrationComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudio_MetaSoundManager::SetAmbienceType(EAudio_AmbienceType NewAmbienceType, float FadeTime)
{
    if (CurrentAmbienceType == NewAmbienceType)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Changing ambience from %d to %d"), 
           (int32)CurrentAmbienceType, (int32)NewAmbienceType);
    
    CrossfadeToNewAmbience(NewAmbienceType, FadeTime);
    CurrentAmbienceType = NewAmbienceType;
}

void UAudio_MetaSoundManager::SetThreatLevel(EAudio_ThreatLevel ThreatLevel, float FadeTime)
{
    if (CurrentThreatLevel == ThreatLevel)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Threat level changed to %d"), (int32)ThreatLevel);
    
    UpdateThreatLevelMix(ThreatLevel, FadeTime);
    CurrentThreatLevel = ThreatLevel;
}

void UAudio_MetaSoundManager::PlayDinosaurSound(const FString& DinosaurType, const FVector& Location, float Volume)
{
    if (DinosaurSounds.Contains(DinosaurType))
    {
        TSoftObjectPtr<UMetaSoundSource>* SoundPtr = DinosaurSounds.Find(DinosaurType);
        if (SoundPtr && SoundPtr->IsValid())
        {
            UMetaSoundSource* MetaSound = SoundPtr->LoadSynchronous();
            if (MetaSound)
            {
                UGameplayStatics::PlaySoundAtLocation(
                    GetWorld(),
                    MetaSound,
                    Location,
                    Volume * SFXVolume * MasterVolume
                );
                
                UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Playing %s sound at location"), *DinosaurType);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Dinosaur sound %s not found"), *DinosaurType);
    }
}

void UAudio_MetaSoundManager::PlayNarrationLine(const FString& NarrationKey, float Volume)
{
    if (!NarrationComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_MetaSoundManager: Narration component not initialized"));
        return;
    }
    
    if (NarrationSounds.Contains(NarrationKey))
    {
        TSoftObjectPtr<USoundBase>* SoundPtr = NarrationSounds.Find(NarrationKey);
        if (SoundPtr && SoundPtr->IsValid())
        {
            USoundBase* Sound = SoundPtr->LoadSynchronous();
            if (Sound)
            {
                NarrationComponent->SetSound(Sound);
                NarrationComponent->SetVolumeMultiplier(Volume * VoiceVolume * MasterVolume);
                NarrationComponent->Play();
                
                UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Playing narration: %s"), *NarrationKey);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Narration %s not found"), *NarrationKey);
    }
}

void UAudio_MetaSoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active components
    for (UAudioComponent* Component : AmbienceComponents)
    {
        if (Component && IsValid(Component))
        {
            Component->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
        }
    }
    
    if (NarrationComponent && IsValid(NarrationComponent))
    {
        NarrationComponent->SetVolumeMultiplier(VoiceVolume * MasterVolume);
    }
}

void UAudio_MetaSoundManager::SetAmbienceVolume(float Volume)
{
    AmbienceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    for (UAudioComponent* Component : AmbienceComponents)
    {
        if (Component && IsValid(Component))
        {
            Component->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
        }
    }
}

void UAudio_MetaSoundManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudio_MetaSoundManager::SetVoiceVolume(float Volume)
{
    VoiceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (NarrationComponent && IsValid(NarrationComponent))
    {
        NarrationComponent->SetVolumeMultiplier(VoiceVolume * MasterVolume);
    }
}

void UAudio_MetaSoundManager::InitializeAmbienceConfigs()
{
    // Forest ambience configuration
    FAudio_AmbienceConfig ForestConfig;
    ForestConfig.AmbienceType = EAudio_AmbienceType::Forest;
    ForestConfig.CrossfadeTime = 3.0f;
    ForestConfig.MaxDistance = 5000.0f;
    
    FAudio_SoundLayer ForestLayer1;
    ForestLayer1.Volume = 0.6f;
    ForestLayer1.FadeTime = 2.0f;
    ForestLayer1.bIsLooping = true;
    ForestConfig.SoundLayers.Add(ForestLayer1);
    
    AmbienceConfigs.Add(EAudio_AmbienceType::Forest, ForestConfig);
    
    // Danger zone configuration
    FAudio_AmbienceConfig DangerConfig;
    DangerConfig.AmbienceType = EAudio_AmbienceType::Danger;
    DangerConfig.CrossfadeTime = 1.5f;
    DangerConfig.MaxDistance = 3000.0f;
    
    FAudio_SoundLayer DangerLayer1;
    DangerLayer1.Volume = 0.8f;
    DangerLayer1.FadeTime = 1.0f;
    DangerLayer1.bIsLooping = true;
    DangerConfig.SoundLayers.Add(DangerLayer1);
    
    AmbienceConfigs.Add(EAudio_AmbienceType::Danger, DangerConfig);
    
    // Peaceful area configuration
    FAudio_AmbienceConfig PeacefulConfig;
    PeacefulConfig.AmbienceType = EAudio_AmbienceType::Peaceful;
    PeacefulConfig.CrossfadeTime = 4.0f;
    PeacefulConfig.MaxDistance = 8000.0f;
    
    FAudio_SoundLayer PeacefulLayer1;
    PeacefulLayer1.Volume = 0.4f;
    PeacefulLayer1.FadeTime = 3.0f;
    PeacefulLayer1.bIsLooping = true;
    PeacefulConfig.SoundLayers.Add(PeacefulLayer1);
    
    AmbienceConfigs.Add(EAudio_AmbienceType::Peaceful, PeacefulConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Initialized %d ambience configurations"), AmbienceConfigs.Num());
}

void UAudio_MetaSoundManager::InitializeDinosaurSounds()
{
    // Initialize dinosaur sound mappings
    // These would be populated with actual MetaSound assets in a real project
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Dinosaur sounds initialized (placeholder)"));
}

void UAudio_MetaSoundManager::InitializeNarrationSounds()
{
    // Initialize narration sound mappings
    // These would be populated with actual sound assets
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Narration sounds initialized (placeholder)"));
}

void UAudio_MetaSoundManager::CrossfadeToNewAmbience(EAudio_AmbienceType NewType, float FadeTime)
{
    // Fade out current ambience components
    for (UAudioComponent* Component : AmbienceComponents)
    {
        if (Component && IsValid(Component) && Component->IsPlaying())
        {
            Component->FadeOut(FadeTime, 0.0f);
        }
    }
    
    // Clear old components after fade
    AmbienceComponents.Empty();
    
    // Get new ambience config
    if (AmbienceConfigs.Contains(NewType))
    {
        const FAudio_AmbienceConfig& Config = AmbienceConfigs[NewType];
        
        // Create new audio components for each layer
        for (const FAudio_SoundLayer& Layer : Config.SoundLayers)
        {
            UAudioComponent* NewComponent = CreateAudioComponent();
            if (NewComponent)
            {
                NewComponent->SetVolumeMultiplier(0.0f); // Start at 0 for fade in
                
                // Fade in the new component
                NewComponent->FadeIn(FadeTime, Layer.Volume * AmbienceVolume * MasterVolume);
                
                AmbienceComponents.Add(NewComponent);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Crossfaded to %d with %d layers"), 
               (int32)NewType, Config.SoundLayers.Num());
    }
}

void UAudio_MetaSoundManager::UpdateThreatLevelMix(EAudio_ThreatLevel ThreatLevel, float FadeTime)
{
    float ThreatIntensity = 0.0f;
    
    switch (ThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:
            ThreatIntensity = 0.0f;
            break;
        case EAudio_ThreatLevel::Caution:
            ThreatIntensity = 0.3f;
            break;
        case EAudio_ThreatLevel::Danger:
            ThreatIntensity = 0.7f;
            break;
        case EAudio_ThreatLevel::Extreme:
            ThreatIntensity = 1.0f;
            break;
    }
    
    // Adjust ambience volume based on threat level
    float AdjustedVolume = AmbienceVolume * (1.0f - ThreatIntensity * 0.3f);
    
    for (UAudioComponent* Component : AmbienceComponents)
    {
        if (Component && IsValid(Component))
        {
            Component->SetVolumeMultiplier(AdjustedVolume * MasterVolume);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Updated threat level mix - intensity: %f"), ThreatIntensity);
}

UAudioComponent* UAudio_MetaSoundManager::CreateAudioComponent()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_MetaSoundManager: Cannot create audio component - no world"));
        return nullptr;
    }
    
    UAudioComponent* NewComponent = NewObject<UAudioComponent>(this);
    if (NewComponent)
    {
        NewComponent->bAutoActivate = false;
        NewComponent->bStopWhenOwnerDestroyed = true;
        NewComponent->SetWorldLocation(FVector::ZeroVector);
        
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Created new audio component"));
    }
    
    return NewComponent;
}