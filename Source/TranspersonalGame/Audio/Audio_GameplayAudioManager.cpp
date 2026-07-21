#include "Audio_GameplayAudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAudio_GameplayAudioManager::UAudio_GameplayAudioManager()
{
    MasterGameplayVolume = 1.0f;
    LastFootstepTime = 0.0f;
    FootstepInterval = 0.5f;
    RunningFootstepInterval = 0.3f;
}

void UAudio_GameplayAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeAudioConfigs();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_GameplayAudioManager initialized"));
}

void UAudio_GameplayAudioManager::Deinitialize()
{
    StopAllGameplayAudio();
    
    Super::Deinitialize();
}

void UAudio_GameplayAudioManager::InitializeAudioConfigs()
{
    // Initialize default audio configurations
    FAudio_GameplayAudioConfig FootstepConfig;
    FootstepConfig.Volume = 0.7f;
    FootstepConfig.bIs3D = true;
    FootstepConfig.MaxDistance = 500.0f;
    AudioConfigs.Add(EAudio_GameplayAudioType::PlayerFootsteps, FootstepConfig);

    FAudio_GameplayAudioConfig CraftingSuccessConfig;
    CraftingSuccessConfig.Volume = 0.8f;
    CraftingSuccessConfig.bIs3D = false;
    AudioConfigs.Add(EAudio_GameplayAudioType::CraftingSuccess, CraftingSuccessConfig);

    FAudio_GameplayAudioConfig CraftingFailConfig;
    CraftingFailConfig.Volume = 0.6f;
    CraftingFailConfig.bIs3D = false;
    AudioConfigs.Add(EAudio_GameplayAudioType::CraftingFail, CraftingFailConfig);

    FAudio_GameplayAudioConfig ItemPickupConfig;
    ItemPickupConfig.Volume = 0.5f;
    ItemPickupConfig.bIs3D = true;
    ItemPickupConfig.MaxDistance = 300.0f;
    AudioConfigs.Add(EAudio_GameplayAudioType::ItemPickup, ItemPickupConfig);

    FAudio_GameplayAudioConfig InventoryOpenConfig;
    InventoryOpenConfig.Volume = 0.4f;
    InventoryOpenConfig.bIs3D = false;
    AudioConfigs.Add(EAudio_GameplayAudioType::InventoryOpen, InventoryOpenConfig);

    FAudio_GameplayAudioConfig InventoryCloseConfig;
    InventoryCloseConfig.Volume = 0.4f;
    InventoryCloseConfig.bIs3D = false;
    AudioConfigs.Add(EAudio_GameplayAudioType::InventoryClose, InventoryCloseConfig);

    FAudio_GameplayAudioConfig HealthLowConfig;
    HealthLowConfig.Volume = 0.9f;
    HealthLowConfig.bIs3D = false;
    AudioConfigs.Add(EAudio_GameplayAudioType::HealthLow, HealthLowConfig);

    FAudio_GameplayAudioConfig StaminaEmptyConfig;
    StaminaEmptyConfig.Volume = 0.7f;
    StaminaEmptyConfig.bIs3D = false;
    AudioConfigs.Add(EAudio_GameplayAudioType::StaminaEmpty, StaminaEmptyConfig);

    FAudio_GameplayAudioConfig HungerAlertConfig;
    HungerAlertConfig.Volume = 0.6f;
    HungerAlertConfig.bIs3D = false;
    AudioConfigs.Add(EAudio_GameplayAudioType::HungerAlert, HungerAlertConfig);

    FAudio_GameplayAudioConfig ThirstAlertConfig;
    ThirstAlertConfig.Volume = 0.6f;
    ThirstAlertConfig.bIs3D = false;
    AudioConfigs.Add(EAudio_GameplayAudioType::ThirstAlert, ThirstAlertConfig);

    FAudio_GameplayAudioConfig DangerDetectedConfig;
    DangerDetectedConfig.Volume = 1.0f;
    DangerDetectedConfig.bIs3D = false;
    AudioConfigs.Add(EAudio_GameplayAudioType::DangerDetected, DangerDetectedConfig);

    FAudio_GameplayAudioConfig SafetyReachedConfig;
    SafetyReachedConfig.Volume = 0.8f;
    SafetyReachedConfig.bIs3D = false;
    AudioConfigs.Add(EAudio_GameplayAudioType::SafetyReached, SafetyReachedConfig);
}

void UAudio_GameplayAudioManager::PlayGameplayAudio(EAudio_GameplayAudioType AudioType, FVector Location, AActor* SourceActor)
{
    const FAudio_GameplayAudioConfig* Config = AudioConfigs.Find(AudioType);
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("No audio config found for gameplay audio type"));
        return;
    }

    // Stop existing audio of this type if it's playing
    StopGameplayAudio(AudioType);

    // Create and play new audio component
    UAudioComponent* AudioComponent = CreateAudioComponent(*Config, Location, SourceActor);
    if (AudioComponent)
    {
        ActiveAudioComponents.Add(AudioType, AudioComponent);
        AudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Playing gameplay audio type: %d"), (int32)AudioType);
    }
}

void UAudio_GameplayAudioManager::StopGameplayAudio(EAudio_GameplayAudioType AudioType)
{
    UAudioComponent** AudioComponentPtr = ActiveAudioComponents.Find(AudioType);
    if (AudioComponentPtr && *AudioComponentPtr)
    {
        UAudioComponent* AudioComponent = *AudioComponentPtr;
        if (AudioComponent && IsValid(AudioComponent))
        {
            AudioComponent->Stop();
            AudioComponent->DestroyComponent();
        }
        ActiveAudioComponents.Remove(AudioType);
    }
}

void UAudio_GameplayAudioManager::StopAllGameplayAudio()
{
    for (auto& AudioPair : ActiveAudioComponents)
    {
        UAudioComponent* AudioComponent = AudioPair.Value;
        if (AudioComponent && IsValid(AudioComponent))
        {
            AudioComponent->Stop();
            AudioComponent->DestroyComponent();
        }
    }
    ActiveAudioComponents.Empty();
}

void UAudio_GameplayAudioManager::SetGameplayAudioVolume(EAudio_GameplayAudioType AudioType, float Volume)
{
    FAudio_GameplayAudioConfig* Config = AudioConfigs.Find(AudioType);
    if (Config)
    {
        Config->Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
        
        // Update active audio component if playing
        UAudioComponent** AudioComponentPtr = ActiveAudioComponents.Find(AudioType);
        if (AudioComponentPtr && *AudioComponentPtr)
        {
            (*AudioComponentPtr)->SetVolumeMultiplier(Config->Volume * MasterGameplayVolume);
        }
    }
}

void UAudio_GameplayAudioManager::SetMasterGameplayVolume(float Volume)
{
    MasterGameplayVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active audio components
    for (auto& AudioPair : ActiveAudioComponents)
    {
        UAudioComponent* AudioComponent = AudioPair.Value;
        const FAudio_GameplayAudioConfig* Config = AudioConfigs.Find(AudioPair.Key);
        if (AudioComponent && IsValid(AudioComponent) && Config)
        {
            AudioComponent->SetVolumeMultiplier(Config->Volume * MasterGameplayVolume);
        }
    }
}

void UAudio_GameplayAudioManager::SetGameplayAudioConfig(EAudio_GameplayAudioType AudioType, const FAudio_GameplayAudioConfig& Config)
{
    AudioConfigs.Add(AudioType, Config);
}

FAudio_GameplayAudioConfig UAudio_GameplayAudioManager::GetGameplayAudioConfig(EAudio_GameplayAudioType AudioType) const
{
    const FAudio_GameplayAudioConfig* Config = AudioConfigs.Find(AudioType);
    return Config ? *Config : FAudio_GameplayAudioConfig();
}

void UAudio_GameplayAudioManager::TriggerPlayerFootstep(FVector Location, bool bIsRunning)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float RequiredInterval = bIsRunning ? RunningFootstepInterval : FootstepInterval;
    
    if (CurrentTime - LastFootstepTime >= RequiredInterval)
    {
        PlayGameplayAudio(EAudio_GameplayAudioType::PlayerFootsteps, Location);
        LastFootstepTime = CurrentTime;
    }
}

void UAudio_GameplayAudioManager::TriggerCraftingResult(bool bSuccess, FVector Location)
{
    EAudio_GameplayAudioType AudioType = bSuccess ? 
        EAudio_GameplayAudioType::CraftingSuccess : 
        EAudio_GameplayAudioType::CraftingFail;
    
    PlayGameplayAudio(AudioType, Location);
}

void UAudio_GameplayAudioManager::TriggerSurvivalAlert(EAudio_GameplayAudioType AlertType, float Severity)
{
    // Adjust volume based on severity
    FAudio_GameplayAudioConfig Config = GetGameplayAudioConfig(AlertType);
    Config.Volume *= FMath::Clamp(Severity, 0.1f, 2.0f);
    
    SetGameplayAudioConfig(AlertType, Config);
    PlayGameplayAudio(AlertType);
}

void UAudio_GameplayAudioManager::TriggerDangerState(bool bInDanger, FVector ThreatLocation)
{
    if (bInDanger)
    {
        PlayGameplayAudio(EAudio_GameplayAudioType::DangerDetected, ThreatLocation);
    }
    else
    {
        StopGameplayAudio(EAudio_GameplayAudioType::DangerDetected);
        PlayGameplayAudio(EAudio_GameplayAudioType::SafetyReached);
    }
}

UAudioComponent* UAudio_GameplayAudioManager::CreateAudioComponent(const FAudio_GameplayAudioConfig& Config, FVector Location, AActor* SourceActor)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UAudioComponent* AudioComponent = NewObject<UAudioComponent>(this);
    if (!AudioComponent)
    {
        return nullptr;
    }

    // Load sound cue if available
    if (Config.SoundCue.IsValid())
    {
        USoundCue* SoundCue = Config.SoundCue.LoadSynchronous();
        if (SoundCue)
        {
            AudioComponent->SetSound(SoundCue);
        }
    }

    // Configure audio component
    AudioComponent->SetVolumeMultiplier(Config.Volume * MasterGameplayVolume);
    AudioComponent->SetPitchMultiplier(Config.Pitch);
    
    if (Config.bIs3D)
    {
        AudioComponent->bAllowSpatialization = true;
        AudioComponent->SetWorldLocation(Location);
        if (SourceActor)
        {
            AudioComponent->AttachToComponent(SourceActor->GetRootComponent(), 
                FAttachmentTransformRules::KeepWorldTransform);
        }
    }
    else
    {
        AudioComponent->bAllowSpatialization = false;
    }

    return AudioComponent;
}

void UAudio_GameplayAudioManager::CleanupFinishedAudioComponents()
{
    TArray<EAudio_GameplayAudioType> FinishedAudio;
    
    for (auto& AudioPair : ActiveAudioComponents)
    {
        UAudioComponent* AudioComponent = AudioPair.Value;
        if (!AudioComponent || !IsValid(AudioComponent) || !AudioComponent->IsPlaying())
        {
            FinishedAudio.Add(AudioPair.Key);
            if (AudioComponent && IsValid(AudioComponent))
            {
                AudioComponent->DestroyComponent();
            }
        }
    }
    
    for (EAudio_GameplayAudioType AudioType : FinishedAudio)
    {
        ActiveAudioComponents.Remove(AudioType);
    }
}