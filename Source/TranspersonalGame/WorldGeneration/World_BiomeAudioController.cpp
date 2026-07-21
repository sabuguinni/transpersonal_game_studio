#include "World_BiomeAudioController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UWorld_BiomeAudioController::UWorld_BiomeAudioController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5 seconds
    
    AudioUpdateFrequency = 0.5f;
    AudioTransitionDistance = 500.0f;
    GlobalVolumeMultiplier = 1.0f;
    CurrentActiveZone = -1;
    bIsNightTime = false;
    LastAudioUpdate = 0.0f;
    CurrentPlayerLocation = FVector::ZeroVector;
}

void UWorld_BiomeAudioController::BeginPlay()
{
    Super::BeginPlay();
    
    // Create audio components
    WeatherAudioComponent = NewObject<UAudioComponent>(GetOwner());
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        WeatherAudioComponent->SetVolumeMultiplier(0.0f);
        WeatherAudioComponent->RegisterComponent();
    }

    NightAmbienceComponent = NewObject<UAudioComponent>(GetOwner());
    if (NightAmbienceComponent)
    {
        NightAmbienceComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        NightAmbienceComponent->SetVolumeMultiplier(0.0f);
        NightAmbienceComponent->RegisterComponent();
    }

    LoadDefaultAudioSettings();
    InitializeBiomeAudio();
}

void UWorld_BiomeAudioController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    LastAudioUpdate += DeltaTime;
    if (LastAudioUpdate >= AudioUpdateFrequency)
    {
        UpdateAudioZones();
        LastAudioUpdate = 0.0f;
    }
}

void UWorld_BiomeAudioController::InitializeBiomeAudio()
{
    // Create audio components for each registered zone
    for (int32 i = 0; i < AudioZones.Num(); i++)
    {
        UAudioComponent* NewAudioComponent = NewObject<UAudioComponent>(GetOwner());
        if (NewAudioComponent)
        {
            NewAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
                FAttachmentTransformRules::KeepWorldTransform);
            NewAudioComponent->SetVolumeMultiplier(0.0f);
            NewAudioComponent->RegisterComponent();
            BiomeAudioComponents.Add(NewAudioComponent);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeAudioController: Initialized %d audio zones"), AudioZones.Num());
}

void UWorld_BiomeAudioController::UpdatePlayerLocation(const FVector& PlayerLocation)
{
    CurrentPlayerLocation = PlayerLocation;
}

void UWorld_BiomeAudioController::RegisterAudioZone(const FWorld_BiomeAudioZone& AudioZone)
{
    AudioZones.Add(AudioZone);
    
    // Create new audio component for this zone
    UAudioComponent* NewAudioComponent = NewObject<UAudioComponent>(GetOwner());
    if (NewAudioComponent)
    {
        NewAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        NewAudioComponent->SetVolumeMultiplier(0.0f);
        NewAudioComponent->RegisterComponent();
        BiomeAudioComponents.Add(NewAudioComponent);
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeAudioController: Registered new audio zone at %s"), 
        *AudioZone.Center.ToString());
}

void UWorld_BiomeAudioController::UnregisterAudioZone(int32 ZoneIndex)
{
    if (AudioZones.IsValidIndex(ZoneIndex))
    {
        AudioZones.RemoveAt(ZoneIndex);
        
        if (BiomeAudioComponents.IsValidIndex(ZoneIndex))
        {
            if (BiomeAudioComponents[ZoneIndex])
            {
                BiomeAudioComponents[ZoneIndex]->Stop();
                BiomeAudioComponents[ZoneIndex]->DestroyComponent();
            }
            BiomeAudioComponents.RemoveAt(ZoneIndex);
        }
    }
}

void UWorld_BiomeAudioController::SetGlobalAudioVolume(float Volume)
{
    GlobalVolumeMultiplier = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all active audio components
    for (UAudioComponent* AudioComp : BiomeAudioComponents)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            AudioComp->SetVolumeMultiplier(AudioComp->VolumeMultiplier * GlobalVolumeMultiplier);
        }
    }
}

void UWorld_BiomeAudioController::OnWeatherChanged(const FString& WeatherType, float Intensity)
{
    PlayWeatherAudio(WeatherType, Intensity);
}

void UWorld_BiomeAudioController::PlayWeatherAudio(const FString& WeatherType, float Intensity)
{
    if (!WeatherAudioComponent)
        return;

    TSoftObjectPtr<USoundCue>* WeatherSound = WeatherSounds.Find(WeatherType);
    if (WeatherSound && WeatherSound->IsValid())
    {
        WeatherAudioComponent->SetSound(WeatherSound->Get());
        float Volume = FMath::Clamp(Intensity * 0.8f * GlobalVolumeMultiplier, 0.0f, 1.0f);
        FadeAudioComponent(WeatherAudioComponent, Volume, 2.0f);
        
        if (!WeatherAudioComponent->IsPlaying())
        {
            WeatherAudioComponent->Play();
        }

        UE_LOG(LogTemp, Log, TEXT("BiomeAudioController: Playing weather audio %s at volume %f"), 
            *WeatherType, Volume);
    }
}

void UWorld_BiomeAudioController::StopWeatherAudio()
{
    if (WeatherAudioComponent && WeatherAudioComponent->IsPlaying())
    {
        FadeAudioComponent(WeatherAudioComponent, 0.0f, 1.5f);
    }
}

void UWorld_BiomeAudioController::OnTimeOfDayChanged(float TimeOfDay)
{
    bool bShouldBeNight = (TimeOfDay < 6.0f || TimeOfDay > 20.0f);
    
    if (bShouldBeNight != bIsNightTime)
    {
        bIsNightTime = bShouldBeNight;
        UpdateNightAmbience(bIsNightTime);
    }
}

void UWorld_BiomeAudioController::UpdateNightAmbience(bool bIsNight)
{
    if (!NightAmbienceComponent)
        return;

    if (bIsNight)
    {
        if (NightAmbienceSound.IsValid())
        {
            NightAmbienceComponent->SetSound(NightAmbienceSound.Get());
            FadeAudioComponent(NightAmbienceComponent, 0.6f * GlobalVolumeMultiplier, 3.0f);
            
            if (!NightAmbienceComponent->IsPlaying())
            {
                NightAmbienceComponent->Play();
            }
        }
    }
    else
    {
        if (NightAmbienceComponent->IsPlaying())
        {
            FadeAudioComponent(NightAmbienceComponent, 0.0f, 2.0f);
        }
    }
}

void UWorld_BiomeAudioController::UpdateAudioZones()
{
    if (CurrentPlayerLocation == FVector::ZeroVector)
        return;

    int32 NearestZone = FindNearestAudioZone(CurrentPlayerLocation);
    
    if (NearestZone != CurrentActiveZone)
    {
        TransitionToAudioZone(NearestZone);
        CurrentActiveZone = NearestZone;
    }
}

int32 UWorld_BiomeAudioController::FindNearestAudioZone(const FVector& Location)
{
    int32 NearestZone = -1;
    float NearestDistance = FLT_MAX;

    for (int32 i = 0; i < AudioZones.Num(); i++)
    {
        if (!AudioZones[i].bIsActive)
            continue;

        float Distance = FVector::Dist(Location, AudioZones[i].Center);
        
        // Check if within zone radius
        if (Distance <= AudioZones[i].Radius && Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestZone = i;
        }
    }

    return NearestZone;
}

void UWorld_BiomeAudioController::TransitionToAudioZone(int32 ZoneIndex)
{
    // Fade out current zone
    if (CurrentActiveZone >= 0 && BiomeAudioComponents.IsValidIndex(CurrentActiveZone))
    {
        UAudioComponent* CurrentAudio = BiomeAudioComponents[CurrentActiveZone];
        if (CurrentAudio && CurrentAudio->IsPlaying())
        {
            FadeAudioComponent(CurrentAudio, 0.0f, 
                AudioZones.IsValidIndex(CurrentActiveZone) ? 
                AudioZones[CurrentActiveZone].AudioSettings.FadeOutTime : 1.5f);
        }
    }

    // Fade in new zone
    if (ZoneIndex >= 0 && AudioZones.IsValidIndex(ZoneIndex) && BiomeAudioComponents.IsValidIndex(ZoneIndex))
    {
        const FWorld_BiomeAudioZone& Zone = AudioZones[ZoneIndex];
        UAudioComponent* NewAudio = BiomeAudioComponents[ZoneIndex];
        
        if (NewAudio && Zone.AudioSettings.AmbientSound.IsValid())
        {
            NewAudio->SetSound(Zone.AudioSettings.AmbientSound.Get());
            
            float TargetVolume = Zone.AudioSettings.Volume * GlobalVolumeMultiplier;
            FadeAudioComponent(NewAudio, TargetVolume, Zone.AudioSettings.FadeInTime);
            
            if (!NewAudio->IsPlaying())
            {
                NewAudio->Play();
            }

            UE_LOG(LogTemp, Log, TEXT("BiomeAudioController: Transitioned to zone %d"), ZoneIndex);
        }
    }
}

void UWorld_BiomeAudioController::FadeAudioComponent(UAudioComponent* Component, float TargetVolume, float FadeTime)
{
    if (!Component)
        return;

    // Simple immediate volume set for now - could be enhanced with timeline/tween
    Component->SetVolumeMultiplier(TargetVolume);
    
    if (TargetVolume <= 0.0f)
    {
        // Stop after fade out
        FTimerHandle StopTimer;
        GetWorld()->GetTimerManager().SetTimer(StopTimer, [Component]()
        {
            if (Component)
            {
                Component->Stop();
            }
        }, FadeTime, false);
    }
}

void UWorld_BiomeAudioController::LoadDefaultAudioSettings()
{
    // Forest ambient settings
    FWorld_BiomeAudioSettings ForestSettings;
    ForestSettings.Volume = 0.7f;
    ForestSettings.MaxDistance = 3000.0f;
    DefaultAudioSettings.Add(EWorld_BiomeAudioType::Forest_Ambient, ForestSettings);

    // Plains wind settings
    FWorld_BiomeAudioSettings PlainsSettings;
    PlainsSettings.Volume = 0.5f;
    PlainsSettings.MaxDistance = 5000.0f;
    DefaultAudioSettings.Add(EWorld_BiomeAudioType::Plains_Wind, PlainsSettings);

    // River flow settings
    FWorld_BiomeAudioSettings RiverSettings;
    RiverSettings.Volume = 0.8f;
    RiverSettings.MaxDistance = 1500.0f;
    DefaultAudioSettings.Add(EWorld_BiomeAudioType::River_Flow, RiverSettings);

    // Cave echo settings
    FWorld_BiomeAudioSettings CaveSettings;
    CaveSettings.Volume = 0.6f;
    CaveSettings.MaxDistance = 800.0f;
    DefaultAudioSettings.Add(EWorld_BiomeAudioType::Cave_Echo, CaveSettings);

    // Volcanic rumble settings
    FWorld_BiomeAudioSettings VolcanicSettings;
    VolcanicSettings.Volume = 0.9f;
    VolcanicSettings.MaxDistance = 8000.0f;
    DefaultAudioSettings.Add(EWorld_BiomeAudioType::Volcanic_Rumble, VolcanicSettings);

    UE_LOG(LogTemp, Log, TEXT("BiomeAudioController: Loaded default audio settings"));
}