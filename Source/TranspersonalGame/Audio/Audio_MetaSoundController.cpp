#include "Audio_MetaSoundController.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Kismet/GameplayStatics.h"

UAudio_MetaSoundController::UAudio_MetaSoundController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    AudioComponent = nullptr;
    CurrentMetaSound = nullptr;
    MasterVolume = 1.0f;
    CurrentDangerLevel = 0.0f;
    CurrentHeartbeatIntensity = 0.0f;
}

void UAudio_MetaSoundController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponent();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundController initialized for %s"), 
           *GetOwner()->GetName());
}

void UAudio_MetaSoundController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Continuously apply parameter updates to active MetaSound
    if (AudioComponent && AudioComponent->IsPlaying() && CurrentMetaSound)
    {
        ApplyParametersToMetaSound();
    }
}

void UAudio_MetaSoundController::PlayMetaSound(UMetaSoundSource* MetaSound, FVector Location)
{
    if (!MetaSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to play null MetaSound"));
        return;
    }

    StopMetaSound();
    
    CurrentMetaSound = MetaSound;
    
    if (!AudioComponent)
    {
        InitializeAudioComponent();
    }

    if (AudioComponent)
    {
        if (Location != FVector::ZeroVector)
        {
            AudioComponent->SetWorldLocation(Location);
        }
        
        AudioComponent->SetSound(MetaSound);
        AudioComponent->SetVolumeMultiplier(MasterVolume);
        AudioComponent->Play();
        
        // Apply any existing parameters
        ApplyParametersToMetaSound();
        
        UE_LOG(LogTemp, Log, TEXT("Playing MetaSound: %s"), *MetaSound->GetName());
    }
}

void UAudio_MetaSoundController::StopMetaSound()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
    
    CurrentMetaSound = nullptr;
    ActiveParameters.Empty();
}

void UAudio_MetaSoundController::SetFloatParameter(FName ParameterName, float Value)
{
    UpdateParameterValue(ParameterName, Value);
    
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->SetFloatParameter(ParameterName, Value);
    }
}

void UAudio_MetaSoundController::SetIntParameter(FName ParameterName, int32 Value)
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->SetIntParameter(ParameterName, Value);
    }
    
    // Store in ActiveParameters for later application
    FAudio_MetaSoundParameter NewParam;
    NewParam.ParameterName = ParameterName;
    NewParam.IntValue = Value;
    ActiveParameters.Add(NewParam);
}

void UAudio_MetaSoundController::SetBoolParameter(FName ParameterName, bool Value)
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->SetBoolParameter(ParameterName, Value);
    }
    
    // Store in ActiveParameters for later application
    FAudio_MetaSoundParameter NewParam;
    NewParam.ParameterName = ParameterName;
    NewParam.BoolValue = Value;
    ActiveParameters.Add(NewParam);
}

void UAudio_MetaSoundController::TriggerEvent(EAudio_MetaSoundTrigger TriggerType, float Intensity)
{
    UMetaSoundSource* MetaSound = GetMetaSoundForTrigger(TriggerType);
    if (MetaSound)
    {
        PlayMetaSound(MetaSound);
        SetFloatParameter(FName("Intensity"), Intensity);
        
        UE_LOG(LogTemp, Log, TEXT("Triggered MetaSound event: %d with intensity %f"), 
               (int32)TriggerType, Intensity);
    }
}

void UAudio_MetaSoundController::UpdateDangerLevel(float DangerLevel)
{
    CurrentDangerLevel = FMath::Clamp(DangerLevel, 0.0f, 1.0f);
    SetFloatParameter(FName("DangerLevel"), CurrentDangerLevel);
    
    // Also update heartbeat based on danger
    float HeartbeatRate = FMath::Lerp(60.0f, 120.0f, CurrentDangerLevel);
    SetFloatParameter(FName("HeartbeatRate"), HeartbeatRate);
}

void UAudio_MetaSoundController::UpdateHeartbeatIntensity(float Intensity)
{
    CurrentHeartbeatIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    SetFloatParameter(FName("HeartbeatIntensity"), CurrentHeartbeatIntensity);
    SetFloatParameter(FName("HeartbeatVolume"), CurrentHeartbeatIntensity * 0.8f);
}

void UAudio_MetaSoundController::UpdateEnvironmentAmbience(const FString& BiomeType, float Intensity)
{
    SetFloatParameter(FName("AmbienceIntensity"), Intensity);
    
    // Set biome-specific parameters
    if (BiomeType == "Forest")
    {
        SetFloatParameter(FName("ForestDensity"), Intensity);
        SetBoolParameter(FName("IsForest"), true);
        SetBoolParameter(FName("IsDesert"), false);
        SetBoolParameter(FName("IsMountain"), false);
    }
    else if (BiomeType == "Desert")
    {
        SetFloatParameter(FName("WindIntensity"), Intensity);
        SetBoolParameter(FName("IsDesert"), true);
        SetBoolParameter(FName("IsForest"), false);
        SetBoolParameter(FName("IsMountain"), false);
    }
    else if (BiomeType == "Mountain")
    {
        SetFloatParameter(FName("EchoIntensity"), Intensity);
        SetBoolParameter(FName("IsMountain"), true);
        SetBoolParameter(FName("IsForest"), false);
        SetBoolParameter(FName("IsDesert"), false);
    }
}

void UAudio_MetaSoundController::RegisterMetaSoundAsset(EAudio_MetaSoundTrigger TriggerType, UMetaSoundSource* MetaSound)
{
    if (MetaSound)
    {
        MetaSoundAssets.Add(TriggerType, MetaSound);
        UE_LOG(LogTemp, Log, TEXT("Registered MetaSound for trigger type: %d"), (int32)TriggerType);
    }
}

bool UAudio_MetaSoundController::IsMetaSoundPlaying() const
{
    return AudioComponent && AudioComponent->IsPlaying();
}

void UAudio_MetaSoundController::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AudioComponent)
    {
        AudioComponent->SetVolumeMultiplier(MasterVolume);
    }
}

void UAudio_MetaSoundController::InitializeAudioComponent()
{
    if (!AudioComponent)
    {
        AudioComponent = GetOwner()->FindComponentByClass<UAudioComponent>();
        
        if (!AudioComponent)
        {
            AudioComponent = NewObject<UAudioComponent>(GetOwner());
            AudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
                                            FAttachmentTransformRules::KeepWorldTransform);
            AudioComponent->RegisterComponent();
        }
        
        AudioComponent->SetVolumeMultiplier(MasterVolume);
    }
}

void UAudio_MetaSoundController::ApplyParametersToMetaSound()
{
    if (!AudioComponent || !AudioComponent->IsPlaying())
    {
        return;
    }
    
    for (const FAudio_MetaSoundParameter& Param : ActiveParameters)
    {
        if (Param.ParameterName != NAME_None)
        {
            AudioComponent->SetFloatParameter(Param.ParameterName, Param.FloatValue);
        }
    }
}

void UAudio_MetaSoundController::UpdateParameterValue(FName ParameterName, float Value)
{
    // Find existing parameter or create new one
    FAudio_MetaSoundParameter* ExistingParam = ActiveParameters.FindByPredicate(
        [ParameterName](const FAudio_MetaSoundParameter& Param)
        {
            return Param.ParameterName == ParameterName;
        });
    
    if (ExistingParam)
    {
        ExistingParam->FloatValue = Value;
    }
    else
    {
        FAudio_MetaSoundParameter NewParam;
        NewParam.ParameterName = ParameterName;
        NewParam.FloatValue = Value;
        ActiveParameters.Add(NewParam);
    }
}

UMetaSoundSource* UAudio_MetaSoundController::GetMetaSoundForTrigger(EAudio_MetaSoundTrigger TriggerType) const
{
    if (const auto* FoundMetaSound = MetaSoundAssets.Find(TriggerType))
    {
        return *FoundMetaSound;
    }
    return nullptr;
}