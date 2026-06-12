#include "Audio_MetaSoundController.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "MetasoundSource.h"

UAudio_MetaSoundController::UAudio_MetaSoundController()
{
    PrimaryComponentTick.bCanEverTick = true;
    ParameterInterpolationSpeed = 2.0f;
    CurrentFootstepIntensity = 0.0f;
    CurrentAmbientLevel = 0.5f;
}

void UAudio_MetaSoundController::BeginPlay()
{
    Super::BeginPlay();
    InitializeMetaSounds();
}

void UAudio_MetaSoundController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateParameterInterpolation(DeltaTime);
}

void UAudio_MetaSoundController::InitializeMetaSounds()
{
    // Initialize default MetaSound configurations for prehistoric game
    FAudio_MetaSoundConfig FootstepConfig;
    FootstepConfig.ParameterName = "Intensity";
    FootstepConfig.DefaultValue = 0.5f;
    FootstepConfig.MinValue = 0.0f;
    FootstepConfig.MaxValue = 1.0f;
    MetaSoundConfigs.Add(FootstepConfig);

    FAudio_MetaSoundConfig AmbientConfig;
    AmbientConfig.ParameterName = "AmbientLevel";
    AmbientConfig.DefaultValue = 0.3f;
    AmbientConfig.MinValue = 0.0f;
    AmbientConfig.MaxValue = 1.0f;
    MetaSoundConfigs.Add(AmbientConfig);

    FAudio_MetaSoundConfig RoarConfig;
    RoarConfig.ParameterName = "Aggression";
    RoarConfig.DefaultValue = 0.7f;
    RoarConfig.MinValue = 0.0f;
    RoarConfig.MaxValue = 1.0f;
    MetaSoundConfigs.Add(RoarConfig);

    // Initialize current parameters
    for (const FAudio_MetaSoundConfig& Config : MetaSoundConfigs)
    {
        CurrentParameters.Add(Config.ParameterName, Config.DefaultValue);
        TargetParameters.Add(Config.ParameterName, Config.DefaultValue);
    }
}

void UAudio_MetaSoundController::SetMetaSoundParameter(const FString& ParameterName, float Value)
{
    if (TargetParameters.Contains(ParameterName))
    {
        // Find the config to clamp value
        for (const FAudio_MetaSoundConfig& Config : MetaSoundConfigs)
        {
            if (Config.ParameterName == ParameterName)
            {
                float ClampedValue = FMath::Clamp(Value, Config.MinValue, Config.MaxValue);
                TargetParameters[ParameterName] = ClampedValue;
                break;
            }
        }
    }
}

void UAudio_MetaSoundController::PlayMetaSound(const FString& ConfigName)
{
    UAudioComponent* AudioComp = GetOrCreateAudioComponent(ConfigName);
    if (AudioComp && !AudioComp->IsPlaying())
    {
        AudioComp->Play();
    }
}

void UAudio_MetaSoundController::StopMetaSound(const FString& ConfigName)
{
    if (UAudioComponent** AudioCompPtr = ActiveMetaSounds.Find(ConfigName))
    {
        if (*AudioCompPtr && (*AudioCompPtr)->IsPlaying())
        {
            (*AudioCompPtr)->Stop();
        }
    }
}

void UAudio_MetaSoundController::PlayDinosaurFootsteps(float Intensity, float Speed)
{
    SetMetaSoundParameter("Intensity", Intensity);
    SetMetaSoundParameter("Speed", Speed);
    PlayMetaSound("DinosaurFootsteps");
    
    CurrentFootstepIntensity = Intensity;
}

void UAudio_MetaSoundController::PlayDinosaurRoar(float Aggression, float Distance)
{
    SetMetaSoundParameter("Aggression", Aggression);
    SetMetaSoundParameter("Distance", Distance);
    PlayMetaSound("DinosaurRoar");
}

void UAudio_MetaSoundController::UpdateAmbientForest(float TimeOfDay, float WeatherIntensity)
{
    // Calculate ambient level based on time of day and weather
    float AmbientLevel = 0.3f + (TimeOfDay * 0.4f) + (WeatherIntensity * 0.3f);
    AmbientLevel = FMath::Clamp(AmbientLevel, 0.0f, 1.0f);
    
    SetMetaSoundParameter("AmbientLevel", AmbientLevel);
    SetMetaSoundParameter("WeatherIntensity", WeatherIntensity);
    PlayMetaSound("ForestAmbient");
    
    CurrentAmbientLevel = AmbientLevel;
}

void UAudio_MetaSoundController::PlayCraftingSound(const FString& MaterialType)
{
    if (MaterialType == "Stone")
    {
        SetMetaSoundParameter("MaterialHardness", 0.8f);
    }
    else if (MaterialType == "Wood")
    {
        SetMetaSoundParameter("MaterialHardness", 0.4f);
    }
    else if (MaterialType == "Bone")
    {
        SetMetaSoundParameter("MaterialHardness", 0.6f);
    }
    
    PlayMetaSound("CraftingSound");
}

void UAudio_MetaSoundController::UpdateParameterInterpolation(float DeltaTime)
{
    for (auto& ParamPair : CurrentParameters)
    {
        const FString& ParamName = ParamPair.Key;
        float& CurrentValue = ParamPair.Value;
        
        if (TargetParameters.Contains(ParamName))
        {
            float TargetValue = TargetParameters[ParamName];
            
            if (!FMath::IsNearlyEqual(CurrentValue, TargetValue, 0.01f))
            {
                CurrentValue = FMath::FInterpTo(CurrentValue, TargetValue, DeltaTime, ParameterInterpolationSpeed);
                
                // Apply the interpolated value to active audio components
                for (auto& AudioPair : ActiveMetaSounds)
                {
                    if (AudioPair.Value && AudioPair.Value->IsPlaying())
                    {
                        // Note: Actual MetaSound parameter setting would require MetaSound-specific API
                        // This is a placeholder for the parameter update mechanism
                    }
                }
            }
        }
    }
}

UAudioComponent* UAudio_MetaSoundController::GetOrCreateAudioComponent(const FString& ConfigName)
{
    if (UAudioComponent** ExistingComp = ActiveMetaSounds.Find(ConfigName))
    {
        return *ExistingComp;
    }
    
    // Create new audio component
    UAudioComponent* NewAudioComp = NewObject<UAudioComponent>(GetOwner());
    if (NewAudioComp)
    {
        NewAudioComp->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        
        // Find matching MetaSound asset
        for (const FAudio_MetaSoundConfig& Config : MetaSoundConfigs)
        {
            if (Config.ParameterName.Contains(ConfigName) && Config.MetaSoundAsset.IsValid())
            {
                NewAudioComp->SetSound(Config.MetaSoundAsset.LoadSynchronous());
                break;
            }
        }
        
        ActiveMetaSounds.Add(ConfigName, NewAudioComp);
    }
    
    return NewAudioComp;
}