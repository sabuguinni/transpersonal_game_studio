#include "MetaSoundAudioManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundClass.h"
#include "MetasoundSource.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogMetaSoundAudio);

UMetaSoundAudioManager::UMetaSoundAudioManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // ~60 FPS for smooth parameter updates
    
    // Initialize default values
    GlobalParameterSmoothingTime = 0.5f;
    MaxConcurrentInstances = 32;
    NextInstanceID = 1;
    CurrentMasterVolume = 1.0f;
    
    // Initialize emotional state defaults
    CurrentEmotionalState = EEmotionalState::Calm;
    CurrentTensionLevel = 0.0f;
    CurrentWonderLevel = 0.5f;
    CurrentFearLevel = 0.0f;
    CurrentIsolationLevel = 0.3f;
    
    // Initialize environmental defaults
    CurrentZone = EEnvironmentalZone::Forest;
    CurrentTimeOfDay = ETimeOfDay::Morning;
    CurrentWeatherState = EWeatherState::Clear;
}

void UMetaSoundAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogMetaSoundAudio, Log, TEXT("MetaSound Audio Manager initializing..."));
    
    InitializeMetaSoundSystem();
}

void UMetaSoundAudioManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update parameter smoothing
    UpdateParameterSmoothing(DeltaTime);
    
    // Apply emotional and environmental states to parameters
    ApplyEmotionalStateToParameters();
    ApplyEnvironmentalStateToParameters();
    
    // Cleanup inactive instances
    CleanupInactiveInstances();
}

void UMetaSoundAudioManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyAllInstances();
    Super::EndPlay(EndPlayReason);
}

void UMetaSoundAudioManager::InitializeMetaSoundSystem()
{
    UE_LOG(LogMetaSoundAudio, Log, TEXT("Initializing MetaSound Audio System"));
    
    // Clear any existing instances
    DestroyAllInstances();
    
    // Initialize global parameters with default values
    GlobalParameters.Add(TEXT("MasterVolume"), 1.0f);
    GlobalParameters.Add(TEXT("TensionLevel"), 0.0f);
    GlobalParameters.Add(TEXT("WonderLevel"), 0.5f);
    GlobalParameters.Add(TEXT("FearLevel"), 0.0f);
    GlobalParameters.Add(TEXT("IsolationLevel"), 0.3f);
    GlobalParameters.Add(TEXT("TimeOfDay"), 0.5f);
    GlobalParameters.Add(TEXT("WeatherIntensity"), 0.0f);
    GlobalParameters.Add(TEXT("WindIntensity"), 0.0f);
    GlobalParameters.Add(TEXT("RainIntensity"), 0.0f);
    
    UE_LOG(LogMetaSoundAudio, Log, TEXT("MetaSound Audio System initialized with %d global parameters"), GlobalParameters.Num());
}

int32 UMetaSoundAudioManager::CreateMetaSoundInstance(UMetaSoundSource* MetaSoundAsset, EAudioLayer LayerType, bool bAutoStart)
{
    if (!MetaSoundAsset)
    {
        UE_LOG(LogMetaSoundAudio, Warning, TEXT("Cannot create MetaSound instance: MetaSoundAsset is null"));
        return -1;
    }
    
    if (ActiveInstances.Num() >= MaxConcurrentInstances)
    {
        UE_LOG(LogMetaSoundAudio, Warning, TEXT("Cannot create MetaSound instance: Maximum concurrent instances (%d) reached"), MaxConcurrentInstances);
        return -1;
    }
    
    // Create audio component
    UAudioComponent* AudioComp = CreateAudioComponentForMetaSound(MetaSoundAsset);
    if (!AudioComp)
    {
        UE_LOG(LogMetaSoundAudio, Error, TEXT("Failed to create AudioComponent for MetaSound"));
        return -1;
    }
    
    // Create instance data
    FMetaSoundInstanceData InstanceData;
    InstanceData.AudioComponent = AudioComp;
    InstanceData.bIsActive = bAutoStart;
    InstanceData.StartTime = GetWorld()->GetTimeSeconds();
    InstanceData.LayerType = LayerType;
    
    // Initialize parameters with defaults
    if (LayerConfigs.Contains(LayerType))
    {
        const FMetaSoundLayerConfig& LayerConfig = LayerConfigs[LayerType];
        for (const FMetaSoundParameterConfig& ParamConfig : LayerConfig.Parameters)
        {
            InstanceData.CurrentParameterValues.Add(ParamConfig.ParameterName, ParamConfig.DefaultValue);
            InstanceData.TargetParameterValues.Add(ParamConfig.ParameterName, ParamConfig.DefaultValue);
            InstanceData.ParameterVelocities.Add(ParamConfig.ParameterName, 0.0f);
            
            // Set initial parameter value
            SetParameterOnAudioComponent(AudioComp, ParamConfig.ParameterName, ParamConfig.DefaultValue);
        }
    }
    
    // Store instance
    int32 InstanceID = NextInstanceID++;
    ActiveInstances.Add(InstanceID, InstanceData);
    
    // Add to layer tracking
    if (!LayerInstances.Contains(LayerType))
    {
        LayerInstances.Add(LayerType, TArray<int32>());
    }
    LayerInstances[LayerType].Add(InstanceID);
    
    // Start playback if requested
    if (bAutoStart)
    {
        AudioComp->Play();
    }
    
    UE_LOG(LogMetaSoundAudio, Log, TEXT("Created MetaSound instance %d for layer %d"), InstanceID, (int32)LayerType);
    return InstanceID;
}

bool UMetaSoundAudioManager::DestroyMetaSoundInstance(int32 InstanceID)
{
    if (!ActiveInstances.Contains(InstanceID))
    {
        return false;
    }
    
    FMetaSoundInstanceData& InstanceData = ActiveInstances[InstanceID];
    
    // Stop and destroy audio component
    if (InstanceData.AudioComponent)
    {
        InstanceData.AudioComponent->Stop();
        InstanceData.AudioComponent->DestroyComponent();
    }
    
    // Remove from layer tracking
    if (LayerInstances.Contains(InstanceData.LayerType))
    {
        LayerInstances[InstanceData.LayerType].Remove(InstanceID);
    }
    
    // Remove from active instances
    ActiveInstances.Remove(InstanceID);
    
    UE_LOG(LogMetaSoundAudio, Log, TEXT("Destroyed MetaSound instance %d"), InstanceID);
    return true;
}

void UMetaSoundAudioManager::DestroyAllInstances()
{
    UE_LOG(LogMetaSoundAudio, Log, TEXT("Destroying all MetaSound instances (%d total)"), ActiveInstances.Num());
    
    for (auto& InstancePair : ActiveInstances)
    {
        FMetaSoundInstanceData& InstanceData = InstancePair.Value;
        if (InstanceData.AudioComponent)
        {
            InstanceData.AudioComponent->Stop();
            InstanceData.AudioComponent->DestroyComponent();
        }
    }
    
    ActiveInstances.Empty();
    LayerInstances.Empty();
}

void UMetaSoundAudioManager::RegisterLayerConfig(const FMetaSoundLayerConfig& LayerConfig)
{
    LayerConfigs.Add(LayerConfig.LayerType, LayerConfig);
    UE_LOG(LogMetaSoundAudio, Log, TEXT("Registered layer config for layer %d"), (int32)LayerConfig.LayerType);
}

int32 UMetaSoundAudioManager::CreateLayerInstance(EAudioLayer LayerType)
{
    if (!LayerConfigs.Contains(LayerType))
    {
        UE_LOG(LogMetaSoundAudio, Warning, TEXT("Cannot create layer instance: No config found for layer %d"), (int32)LayerType);
        return -1;
    }
    
    const FMetaSoundLayerConfig& LayerConfig = LayerConfigs[LayerType];
    return CreateMetaSoundInstance(LayerConfig.MetaSoundAsset, LayerType, LayerConfig.bAutoStart);
}

void UMetaSoundAudioManager::SetLayerVolume(EAudioLayer LayerType, float Volume, float FadeTime)
{
    if (!LayerInstances.Contains(LayerType))
    {
        return;
    }
    
    for (int32 InstanceID : LayerInstances[LayerType])
    {
        SetMetaSoundParameter(InstanceID, TEXT("Volume"), Volume, true);
    }
}

void UMetaSoundAudioManager::MuteLayer(EAudioLayer LayerType, float FadeTime)
{
    SetLayerVolume(LayerType, 0.0f, FadeTime);
}

void UMetaSoundAudioManager::UnmuteLayer(EAudioLayer LayerType, float FadeTime)
{
    if (LayerConfigs.Contains(LayerType))
    {
        SetLayerVolume(LayerType, LayerConfigs[LayerType].BaseVolume, FadeTime);
    }
    else
    {
        SetLayerVolume(LayerType, 1.0f, FadeTime);
    }
}

void UMetaSoundAudioManager::SetMetaSoundParameter(int32 InstanceID, FName ParameterName, float Value, bool bUseSmoothing)
{
    if (!ActiveInstances.Contains(InstanceID))
    {
        return;
    }
    
    FMetaSoundInstanceData& InstanceData = ActiveInstances[InstanceID];
    
    if (bUseSmoothing)
    {
        InstanceData.TargetParameterValues.Add(ParameterName, Value);
    }
    else
    {
        InstanceData.CurrentParameterValues.Add(ParameterName, Value);
        InstanceData.TargetParameterValues.Add(ParameterName, Value);
        SetParameterOnAudioComponent(InstanceData.AudioComponent, ParameterName, Value);
    }
}

void UMetaSoundAudioManager::SetLayerParameter(EAudioLayer LayerType, FName ParameterName, float Value, bool bUseSmoothing)
{
    if (!LayerInstances.Contains(LayerType))
    {
        return;
    }
    
    for (int32 InstanceID : LayerInstances[LayerType])
    {
        SetMetaSoundParameter(InstanceID, ParameterName, Value, bUseSmoothing);
    }
}

void UMetaSoundAudioManager::SetGlobalParameter(FName ParameterName, float Value, bool bUseSmoothing)
{
    GlobalParameters.Add(ParameterName, Value);
    
    // Apply to all active instances
    for (auto& InstancePair : ActiveInstances)
    {
        SetMetaSoundParameter(InstancePair.Key, ParameterName, Value, bUseSmoothing);
    }
}

float UMetaSoundAudioManager::GetMetaSoundParameter(int32 InstanceID, FName ParameterName)
{
    if (!ActiveInstances.Contains(InstanceID))
    {
        return 0.0f;
    }
    
    const FMetaSoundInstanceData& InstanceData = ActiveInstances[InstanceID];
    if (InstanceData.CurrentParameterValues.Contains(ParameterName))
    {
        return InstanceData.CurrentParameterValues[ParameterName];
    }
    
    return 0.0f;
}

void UMetaSoundAudioManager::UpdateEmotionalState(EEmotionalState EmotionalState, float Intensity)
{
    CurrentEmotionalState = EmotionalState;
    
    // Map emotional state to parameter values
    switch (EmotionalState)
    {
        case EEmotionalState::Calm:
            SetGlobalParameter(TEXT("TensionLevel"), 0.0f * Intensity);
            SetGlobalParameter(TEXT("WonderLevel"), 0.7f * Intensity);
            SetGlobalParameter(TEXT("FearLevel"), 0.0f);
            break;
            
        case EEmotionalState::Tense:
            SetGlobalParameter(TEXT("TensionLevel"), 0.8f * Intensity);
            SetGlobalParameter(TEXT("WonderLevel"), 0.2f);
            SetGlobalParameter(TEXT("FearLevel"), 0.3f * Intensity);
            break;
            
        case EEmotionalState::Danger:
            SetGlobalParameter(TEXT("TensionLevel"), 1.0f * Intensity);
            SetGlobalParameter(TEXT("WonderLevel"), 0.0f);
            SetGlobalParameter(TEXT("FearLevel"), 0.9f * Intensity);
            break;
            
        case EEmotionalState::Wonder:
            SetGlobalParameter(TEXT("TensionLevel"), 0.1f);
            SetGlobalParameter(TEXT("WonderLevel"), 1.0f * Intensity);
            SetGlobalParameter(TEXT("FearLevel"), 0.0f);
            break;
            
        case EEmotionalState::Fear:
            SetGlobalParameter(TEXT("TensionLevel"), 0.9f * Intensity);
            SetGlobalParameter(TEXT("WonderLevel"), 0.0f);
            SetGlobalParameter(TEXT("FearLevel"), 1.0f * Intensity);
            break;
            
        case EEmotionalState::Discovery:
            SetGlobalParameter(TEXT("TensionLevel"), 0.3f);
            SetGlobalParameter(TEXT("WonderLevel"), 0.9f * Intensity);
            SetGlobalParameter(TEXT("FearLevel"), 0.1f);
            break;
    }
}

void UMetaSoundAudioManager::SetTensionLevel(float TensionLevel, float TransitionTime)
{
    CurrentTensionLevel = FMath::Clamp(TensionLevel, 0.0f, 1.0f);
    SetGlobalParameter(TEXT("TensionLevel"), CurrentTensionLevel, true);
}

void UMetaSoundAudioManager::SetWonderLevel(float WonderLevel, float TransitionTime)
{
    CurrentWonderLevel = FMath::Clamp(WonderLevel, 0.0f, 1.0f);
    SetGlobalParameter(TEXT("WonderLevel"), CurrentWonderLevel, true);
}

void UMetaSoundAudioManager::SetFearLevel(float FearLevel, float TransitionTime)
{
    CurrentFearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    SetGlobalParameter(TEXT("FearLevel"), CurrentFearLevel, true);
}

void UMetaSoundAudioManager::SetIsolationLevel(float IsolationLevel, float TransitionTime)
{
    CurrentIsolationLevel = FMath::Clamp(IsolationLevel, 0.0f, 1.0f);
    SetGlobalParameter(TEXT("IsolationLevel"), CurrentIsolationLevel, true);
}

void UMetaSoundAudioManager::UpdateEnvironmentalZone(EEnvironmentalZone Zone)
{
    CurrentZone = Zone;
    
    // Map zone to environmental parameters
    switch (Zone)
    {
        case EEnvironmentalZone::Forest:
            SetGlobalParameter(TEXT("EnvironmentalDensity"), 0.7f);
            SetGlobalParameter(TEXT("ReverbSize"), 0.5f);
            break;
            
        case EEnvironmentalZone::Cave:
            SetGlobalParameter(TEXT("EnvironmentalDensity"), 0.9f);
            SetGlobalParameter(TEXT("ReverbSize"), 0.8f);
            break;
            
        case EEnvironmentalZone::Plains:
            SetGlobalParameter(TEXT("EnvironmentalDensity"), 0.2f);
            SetGlobalParameter(TEXT("ReverbSize"), 0.1f);
            break;
            
        case EEnvironmentalZone::Swamp:
            SetGlobalParameter(TEXT("EnvironmentalDensity"), 0.8f);
            SetGlobalParameter(TEXT("ReverbSize"), 0.6f);
            break;
    }
}

void UMetaSoundAudioManager::UpdateTimeOfDay(ETimeOfDay TimeOfDay, float BlendFactor)
{
    CurrentTimeOfDay = TimeOfDay;
    
    // Convert time of day to normalized value
    float TimeValue = 0.5f;
    switch (TimeOfDay)
    {
        case ETimeOfDay::Dawn: TimeValue = 0.1f; break;
        case ETimeOfDay::Morning: TimeValue = 0.3f; break;
        case ETimeOfDay::Noon: TimeValue = 0.5f; break;
        case ETimeOfDay::Afternoon: TimeValue = 0.7f; break;
        case ETimeOfDay::Dusk: TimeValue = 0.9f; break;
        case ETimeOfDay::Night: TimeValue = 0.0f; break;
        case ETimeOfDay::DeepNight: TimeValue = 0.05f; break;
    }
    
    SetGlobalParameter(TEXT("TimeOfDay"), TimeValue, true);
}

void UMetaSoundAudioManager::UpdateWeatherState(EWeatherState WeatherState, float Intensity)
{
    CurrentWeatherState = WeatherState;
    
    // Reset weather parameters
    SetGlobalParameter(TEXT("RainIntensity"), 0.0f);
    SetGlobalParameter(TEXT("WindIntensity"), 0.0f);
    SetGlobalParameter(TEXT("StormIntensity"), 0.0f);
    
    // Set specific weather parameters
    switch (WeatherState)
    {
        case EWeatherState::LightRain:
            SetGlobalParameter(TEXT("RainIntensity"), 0.3f * Intensity);
            break;
            
        case EWeatherState::HeavyRain:
            SetGlobalParameter(TEXT("RainIntensity"), 0.8f * Intensity);
            break;
            
        case EWeatherState::Storm:
            SetGlobalParameter(TEXT("RainIntensity"), 1.0f * Intensity);
            SetGlobalParameter(TEXT("WindIntensity"), 0.9f * Intensity);
            SetGlobalParameter(TEXT("StormIntensity"), 1.0f * Intensity);
            break;
            
        case EWeatherState::Wind:
            SetGlobalParameter(TEXT("WindIntensity"), 0.6f * Intensity);
            break;
    }
}

void UMetaSoundAudioManager::SetWindIntensity(float WindIntensity)
{
    SetGlobalParameter(TEXT("WindIntensity"), FMath::Clamp(WindIntensity, 0.0f, 1.0f), true);
}

void UMetaSoundAudioManager::SetRainIntensity(float RainIntensity)
{
    SetGlobalParameter(TEXT("RainIntensity"), FMath::Clamp(RainIntensity, 0.0f, 1.0f), true);
}

void UMetaSoundAudioManager::SetMasterVolume(float Volume, float FadeTime)
{
    CurrentMasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    SetGlobalParameter(TEXT("MasterVolume"), CurrentMasterVolume, true);
}

void UMetaSoundAudioManager::PauseAllInstances()
{
    for (auto& InstancePair : ActiveInstances)
    {
        FMetaSoundInstanceData& InstanceData = InstancePair.Value;
        if (InstanceData.AudioComponent && InstanceData.bIsActive)
        {
            InstanceData.AudioComponent->SetPaused(true);
        }
    }
}

void UMetaSoundAudioManager::ResumeAllInstances()
{
    for (auto& InstancePair : ActiveInstances)
    {
        FMetaSoundInstanceData& InstanceData = InstancePair.Value;
        if (InstanceData.AudioComponent && InstanceData.bIsActive)
        {
            InstanceData.AudioComponent->SetPaused(false);
        }
    }
}

void UMetaSoundAudioManager::SetGlobalLowPassFilter(float CutoffFrequency, float Q)
{
    SetGlobalParameter(TEXT("LowPassCutoff"), CutoffFrequency, true);
    SetGlobalParameter(TEXT("LowPassQ"), Q, true);
}

void UMetaSoundAudioManager::SetGlobalHighPassFilter(float CutoffFrequency, float Q)
{
    SetGlobalParameter(TEXT("HighPassCutoff"), CutoffFrequency, true);
    SetGlobalParameter(TEXT("HighPassQ"), Q, true);
}

void UMetaSoundAudioManager::DebugPrintActiveInstances()
{
    UE_LOG(LogMetaSoundAudio, Log, TEXT("=== Active MetaSound Instances ==="));
    UE_LOG(LogMetaSoundAudio, Log, TEXT("Total Instances: %d"), ActiveInstances.Num());
    
    for (const auto& InstancePair : ActiveInstances)
    {
        const FMetaSoundInstanceData& InstanceData = InstancePair.Value;
        UE_LOG(LogMetaSoundAudio, Log, TEXT("Instance %d: Layer %d, Active: %s, Parameters: %d"),
            InstancePair.Key,
            (int32)InstanceData.LayerType,
            InstanceData.bIsActive ? TEXT("Yes") : TEXT("No"),
            InstanceData.CurrentParameterValues.Num()
        );
    }
}

int32 UMetaSoundAudioManager::GetActiveInstanceCount()
{
    return ActiveInstances.Num();
}

TArray<int32> UMetaSoundAudioManager::GetInstancesByLayer(EAudioLayer LayerType)
{
    if (LayerInstances.Contains(LayerType))
    {
        return LayerInstances[LayerType];
    }
    return TArray<int32>();
}

void UMetaSoundAudioManager::UpdateParameterSmoothing(float DeltaTime)
{
    for (auto& InstancePair : ActiveInstances)
    {
        FMetaSoundInstanceData& InstanceData = InstancePair.Value;
        
        for (auto& ParamPair : InstanceData.TargetParameterValues)
        {
            FName ParamName = ParamPair.Key;
            float TargetValue = ParamPair.Value;
            
            if (InstanceData.CurrentParameterValues.Contains(ParamName))
            {
                float CurrentValue = InstanceData.CurrentParameterValues[ParamName];
                float Difference = TargetValue - CurrentValue;
                
                if (FMath::Abs(Difference) > 0.001f)
                {
                    float SmoothingSpeed = 1.0f / GlobalParameterSmoothingTime;
                    float NewValue = FMath::FInterpTo(CurrentValue, TargetValue, DeltaTime, SmoothingSpeed);
                    
                    InstanceData.CurrentParameterValues[ParamName] = NewValue;
                    SetParameterOnAudioComponent(InstanceData.AudioComponent, ParamName, NewValue);
                }
            }
        }
    }
}

void UMetaSoundAudioManager::ApplyEmotionalStateToParameters()
{
    // This method can be extended to apply complex emotional state mappings
    // For now, the basic mappings are handled in UpdateEmotionalState
}

void UMetaSoundAudioManager::ApplyEnvironmentalStateToParameters()
{
    // This method can be extended to apply complex environmental state mappings
    // For now, the basic mappings are handled in UpdateEnvironmentalZone and UpdateWeatherState
}

void UMetaSoundAudioManager::CleanupInactiveInstances()
{
    TArray<int32> InstancesToRemove;
    
    for (const auto& InstancePair : ActiveInstances)
    {
        const FMetaSoundInstanceData& InstanceData = InstancePair.Value;
        
        if (!InstanceData.AudioComponent || !InstanceData.AudioComponent->IsValidLowLevel())
        {
            InstancesToRemove.Add(InstancePair.Key);
        }
        else if (InstanceData.AudioComponent->HasFinished())
        {
            InstancesToRemove.Add(InstancePair.Key);
        }
    }
    
    for (int32 InstanceID : InstancesToRemove)
    {
        DestroyMetaSoundInstance(InstanceID);
    }
}

UAudioComponent* UMetaSoundAudioManager::CreateAudioComponentForMetaSound(UMetaSoundSource* MetaSoundAsset)
{
    if (!GetOwner())
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(GetOwner());
    if (!AudioComp)
    {
        return nullptr;
    }
    
    AudioComp->SetSound(MetaSoundAsset);
    AudioComp->bAutoActivate = false;
    AudioComp->bStopWhenOwnerDestroyed = true;
    
    if (MasterSoundClass)
    {
        AudioComp->SoundClassOverride = MasterSoundClass;
    }
    
    AudioComp->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
    AudioComp->RegisterComponent();
    
    return AudioComp;
}

void UMetaSoundAudioManager::SetParameterOnAudioComponent(UAudioComponent* AudioComp, FName ParameterName, float Value)
{
    if (!AudioComp || !AudioComp->IsValidLowLevel())
    {
        return;
    }
    
    // Use UE5 MetaSound parameter setting
    AudioComp->SetFloatParameter(ParameterName, Value);
}