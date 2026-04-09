// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "EnvironmentalAudioManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"

AEnvironmentalAudioManager::AEnvironmentalAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // 2 FPS for environmental updates

    // Initialize environmental settings
    CurrentZone = EEnvironmentalZone::Forest;
    CurrentTimeOfDay = ETimeOfDay::Morning;
    WeatherIntensity = 0.0f;
    ZoneTransitionTime = 8.0f;

    // Initialize procedural settings
    bEnableProceduralSounds = true;
    RandomSoundFrequency = 0.1f;
    RandomSoundDistanceRange = FVector2D(500.0f, 2000.0f);

    // Initialize spatial audio
    MaxAudibleDistance = 5000.0f;
    bUse3DAudio = true;
    SpatialAudioFalloffExponent = 2.0f;

    // Initialize volume settings
    MasterAmbientVolume = 0.8f;
    CurrentAmbientVolume = 0.8f;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AEnvironmentalAudioManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("EnvironmentalAudioManager: Beginning environmental audio journey"));

    // Initialize ambient layer components
    AmbientLayers.Reserve(MaxAmbientLayers);
    for (int32 i = 0; i < MaxAmbientLayers; i++)
    {
        UAudioComponent* AmbientComp = CreateDefaultSubobject<UAudioComponent>(*FString::Printf(TEXT("AmbientLayer_%d"), i));
        if (AmbientComp)
        {
            AmbientComp->SetupAttachment(RootComponent);
            AmbientComp->bAutoActivate = false;
            AmbientComp->bAllowSpatialization = false; // Ambient layers are typically non-spatial
            AmbientLayers.Add(AmbientComp);
        }
    }

    // Initialize weather layer components
    WeatherLayers.Reserve(MaxWeatherLayers);
    for (int32 i = 0; i < MaxWeatherLayers; i++)
    {
        UAudioComponent* WeatherComp = CreateDefaultSubobject<UAudioComponent>(*FString::Printf(TEXT("WeatherLayer_%d"), i));
        if (WeatherComp)
        {
            WeatherComp->SetupAttachment(RootComponent);
            WeatherComp->bAutoActivate = false;
            WeatherComp->bAllowSpatialization = false;
            WeatherLayers.Add(WeatherComp);
        }
    }

    // Initialize random element components
    RandomElementLayers.Reserve(MaxRandomElements);
    for (int32 i = 0; i < MaxRandomElements; i++)
    {
        UAudioComponent* RandomComp = CreateDefaultSubobject<UAudioComponent>(*FString::Printf(TEXT("RandomElement_%d"), i));
        if (RandomComp)
        {
            RandomComp->SetupAttachment(RootComponent);
            RandomComp->bAutoActivate = false;
            RandomComp->bAllowSpatialization = bUse3DAudio;
            RandomElementLayers.Add(RandomComp);
        }
    }

    // Start procedural sound timer
    if (bEnableProceduralSounds)
    {
        GetWorld()->GetTimerManager().SetTimer(
            ProceduralSoundTimer,
            this,
            &AEnvironmentalAudioManager::ProcessProceduralSounds,
            1.0f, // Every second
            true
        );
    }

    // Start random element timer
    GetWorld()->GetTimerManager().SetTimer(
        RandomElementTimer,
        this,
        &AEnvironmentalAudioManager::PlayRandomElementSound,
        FMath::RandRange(15.0f, 45.0f), // Random interval between 15-45 seconds
        false
    );

    // Start ambient update timer
    GetWorld()->GetTimerManager().SetTimer(
        AmbientUpdateTimer,
        this,
        &AEnvironmentalAudioManager::UpdateAmbientLayers,
        2.0f, // Every 2 seconds
        true
    );

    // Initialize current zone soundscape
    UpdateAmbientLayers();

    UE_LOG(LogTemp, Log, TEXT("EnvironmentalAudioManager: Initialized with %d ambient, %d weather, %d random element layers"), 
           MaxAmbientLayers, MaxWeatherLayers, MaxRandomElements);
}

void AEnvironmentalAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update spatial audio settings
    UpdateSpatialAudioSettings();

    // Clean up finished audio components
    CleanupFinishedAudioComponents();
}

void AEnvironmentalAudioManager::TransitionToZone(EEnvironmentalZone NewZone, float TransitionTime)
{
    if (CurrentZone == NewZone)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("EnvironmentalAudioManager: Transitioning from zone %d to zone %d over %.1f seconds"), 
           (int32)CurrentZone, (int32)NewZone, TransitionTime);

    bIsTransitioningZones = true;
    ZoneTransitionTime = TransitionTime;

    // Fade out current zone sounds
    for (UAudioComponent* AmbientComp : AmbientLayers)
    {
        if (AmbientComp && AmbientComp->IsPlaying())
        {
            AmbientComp->FadeOut(TransitionTime * 0.5f, 0.0f);
        }
    }

    // Set new zone and start new sounds after half the transition time
    FTimerHandle ZoneTransitionTimer;
    GetWorld()->GetTimerManager().SetTimer(
        ZoneTransitionTimer,
        [this, NewZone]()
        {
            CurrentZone = NewZone;
            UpdateAmbientLayers();
            bIsTransitioningZones = false;
        },
        TransitionTime * 0.5f,
        false
    );
}

void AEnvironmentalAudioManager::SetZoneInstantly(EEnvironmentalZone NewZone)
{
    if (CurrentZone == NewZone)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("EnvironmentalAudioManager: Instantly changing to zone %d"), (int32)NewZone);

    // Stop all current ambient sounds
    for (UAudioComponent* AmbientComp : AmbientLayers)
    {
        if (AmbientComp && AmbientComp->IsPlaying())
        {
            AmbientComp->Stop();
        }
    }

    CurrentZone = NewZone;
    UpdateAmbientLayers();
}

void AEnvironmentalAudioManager::UpdatePlayerLocation(FVector PlayerLocation)
{
    LastPlayerLocation = PlayerLocation;
    
    // Update 3D audio component locations relative to player
    SetActorLocation(PlayerLocation);
}

void AEnvironmentalAudioManager::SetWeatherIntensity(float Intensity, float TransitionTime)
{
    float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("EnvironmentalAudioManager: Weather intensity changing from %.2f to %.2f over %.1f seconds"), 
           WeatherIntensity, ClampedIntensity, TransitionTime);

    WeatherIntensity = ClampedIntensity;
    UpdateWeatherLayers();
}

void AEnvironmentalAudioManager::TriggerThunder(float Intensity)
{
    if (ThunderSounds.Num() == 0)
    {
        return;
    }

    USoundBase* ThunderSound = GetRandomSoundFromArray(ThunderSounds);
    if (ThunderSound)
    {
        // Find available weather layer for thunder
        for (UAudioComponent* WeatherComp : WeatherLayers)
        {
            if (WeatherComp && !WeatherComp->IsPlaying())
            {
                WeatherComp->SetSound(ThunderSound);
                WeatherComp->SetVolumeMultiplier(Intensity * MasterAmbientVolume);
                WeatherComp->Play();
                
                UE_LOG(LogTemp, Warning, TEXT("EnvironmentalAudioManager: Thunder triggered with intensity %.2f"), Intensity);
                break;
            }
        }
    }
}

void AEnvironmentalAudioManager::UpdateTimeOfDay(ETimeOfDay NewTimeOfDay)
{
    if (CurrentTimeOfDay != NewTimeOfDay)
    {
        UE_LOG(LogTemp, Log, TEXT("EnvironmentalAudioManager: Time of day changed from %d to %d"), 
               (int32)CurrentTimeOfDay, (int32)NewTimeOfDay);

        CurrentTimeOfDay = NewTimeOfDay;
        UpdateAmbientLayers(); // Different sounds for different times
    }
}

void AEnvironmentalAudioManager::TriggerDistantCreatureCall(FVector Location, float Volume)
{
    if (DistantCreatureCalls.Num() == 0)
    {
        return;
    }

    USoundBase* CreatureCall = GetRandomSoundFromArray(DistantCreatureCalls);
    if (CreatureCall)
    {
        // Find available random element layer
        for (UAudioComponent* RandomComp : RandomElementLayers)
        {
            if (RandomComp && !RandomComp->IsPlaying())
            {
                RandomComp->SetSound(CreatureCall);
                RandomComp->SetWorldLocation(Location);
                
                float Distance = FVector::Dist(Location, LastPlayerLocation);
                float DistanceVolume = CalculateVolumeFromDistance(Distance);
                RandomComp->SetVolumeMultiplier(Volume * DistanceVolume * MasterAmbientVolume);
                
                RandomComp->Play();
                
                UE_LOG(LogTemp, VeryVerbose, TEXT("EnvironmentalAudioManager: Distant creature call at distance %.1f"), Distance);
                break;
            }
        }
    }
}

void AEnvironmentalAudioManager::TriggerVegetationSound(FVector Location, float Volume)
{
    if (VegetationSounds.Num() == 0)
    {
        return;
    }

    USoundBase* VegetationSound = GetRandomSoundFromArray(VegetationSounds);
    if (VegetationSound)
    {
        // Find available random element layer
        for (UAudioComponent* RandomComp : RandomElementLayers)
        {
            if (RandomComp && !RandomComp->IsPlaying())
            {
                RandomComp->SetSound(VegetationSound);
                RandomComp->SetWorldLocation(Location);
                
                float Distance = FVector::Dist(Location, LastPlayerLocation);
                float DistanceVolume = CalculateVolumeFromDistance(Distance);
                RandomComp->SetVolumeMultiplier(Volume * DistanceVolume * MasterAmbientVolume * 0.3f); // Vegetation is quieter
                
                RandomComp->Play();
                break;
            }
        }
    }
}

void AEnvironmentalAudioManager::TriggerWaterSound(FVector Location, float Volume)
{
    if (WaterSounds.Num() == 0)
    {
        return;
    }

    USoundBase* WaterSound = GetRandomSoundFromArray(WaterSounds);
    if (WaterSound)
    {
        // Find available random element layer
        for (UAudioComponent* RandomComp : RandomElementLayers)
        {
            if (RandomComp && !RandomComp->IsPlaying())
            {
                RandomComp->SetSound(WaterSound);
                RandomComp->SetWorldLocation(Location);
                
                float Distance = FVector::Dist(Location, LastPlayerLocation);
                float DistanceVolume = CalculateVolumeFromDistance(Distance);
                RandomComp->SetVolumeMultiplier(Volume * DistanceVolume * MasterAmbientVolume);
                
                RandomComp->Play();
                break;
            }
        }
    }
}

void AEnvironmentalAudioManager::SetMasterAmbientVolume(float Volume)
{
    MasterAmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CurrentAmbientVolume = MasterAmbientVolume;
    
    // Update all active audio components
    for (UAudioComponent* AmbientComp : AmbientLayers)
    {
        if (AmbientComp)
        {
            AmbientComp->SetVolumeMultiplier(MasterAmbientVolume);
        }
    }
    
    for (UAudioComponent* WeatherComp : WeatherLayers)
    {
        if (WeatherComp)
        {
            WeatherComp->SetVolumeMultiplier(MasterAmbientVolume);
        }
    }
}

void AEnvironmentalAudioManager::FadeAmbientAudio(float TargetVolume, float FadeTime)
{
    UE_LOG(LogTemp, Log, TEXT("EnvironmentalAudioManager: Fading ambient audio to %.2f over %.1f seconds"), TargetVolume, FadeTime);

    float ClampedTargetVolume = FMath::Clamp(TargetVolume, 0.0f, 1.0f);
    
    for (UAudioComponent* AmbientComp : AmbientLayers)
    {
        if (AmbientComp && AmbientComp->IsPlaying())
        {
            AmbientComp->FadeOut(FadeTime, ClampedTargetVolume);
        }
    }
    
    CurrentAmbientVolume = ClampedTargetVolume;
}

void AEnvironmentalAudioManager::SetProceduralSoundsEnabled(bool bEnabled)
{
    bEnableProceduralSounds = bEnabled;
    
    if (bEnabled && !GetWorld()->GetTimerManager().IsTimerActive(ProceduralSoundTimer))
    {
        GetWorld()->GetTimerManager().SetTimer(
            ProceduralSoundTimer,
            this,
            &AEnvironmentalAudioManager::ProcessProceduralSounds,
            1.0f,
            true
        );
    }
    else if (!bEnabled)
    {
        GetWorld()->GetTimerManager().ClearTimer(ProceduralSoundTimer);
    }
}

void AEnvironmentalAudioManager::UpdateAmbientLayers()
{
    if (bIsTransitioningZones)
    {
        return;
    }

    FEnvironmentalSoundscape* CurrentSoundscape = ZoneSoundscapes.Find(CurrentZone);
    if (!CurrentSoundscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvironmentalAudioManager: No soundscape configuration found for zone %d"), (int32)CurrentZone);
        return;
    }

    UE_LOG(LogTemp, VeryVerbose, TEXT("EnvironmentalAudioManager: Updating ambient layers for zone %d"), (int32)CurrentZone);

    // Start ambient layers for current zone
    for (int32 i = 0; i < CurrentSoundscape->AmbientLayers.Num() && i < AmbientLayers.Num(); i++)
    {
        UAudioComponent* AmbientComp = AmbientLayers[i];
        USoundBase* AmbientSound = CurrentSoundscape->AmbientLayers[i];
        
        if (AmbientComp && AmbientSound)
        {
            if (!AmbientComp->IsPlaying() || AmbientComp->GetSound() != AmbientSound)
            {
                AmbientComp->SetSound(AmbientSound);
                
                // Adjust volume based on time of day
                float TimeOfDayMultiplier = 1.0f;
                switch (CurrentTimeOfDay)
                {
                    case ETimeOfDay::Night:
                    case ETimeOfDay::DeepNight:
                        TimeOfDayMultiplier = 0.6f; // Quieter at night
                        break;
                    case ETimeOfDay::Dawn:
                    case ETimeOfDay::Dusk:
                        TimeOfDayMultiplier = 0.8f; // Transitional volume
                        break;
                    default:
                        TimeOfDayMultiplier = 1.0f;
                        break;
                }
                
                AmbientComp->SetVolumeMultiplier(MasterAmbientVolume * TimeOfDayMultiplier);
                AmbientComp->Play();
            }
        }
    }
}

void AEnvironmentalAudioManager::UpdateWeatherLayers()
{
    // Update rain sounds
    if (WeatherIntensity > 0.1f && RainSounds.Num() > 0)
    {
        UAudioComponent* RainComp = WeatherLayers.Num() > 0 ? WeatherLayers[0] : nullptr;
        if (RainComp)
        {
            if (!RainComp->IsPlaying())
            {
                USoundBase* RainSound = GetRandomSoundFromArray(RainSounds);
                if (RainSound)
                {
                    RainComp->SetSound(RainSound);
                    RainComp->Play();
                }
            }
            RainComp->SetVolumeMultiplier(WeatherIntensity * MasterAmbientVolume);
        }
    }
    
    // Update wind sounds
    if (WeatherIntensity > 0.05f && WindSounds.Num() > 0)
    {
        UAudioComponent* WindComp = WeatherLayers.Num() > 1 ? WeatherLayers[1] : nullptr;
        if (WindComp)
        {
            if (!WindComp->IsPlaying())
            {
                USoundBase* WindSound = GetRandomSoundFromArray(WindSounds);
                if (WindSound)
                {
                    WindComp->SetSound(WindSound);
                    WindComp->Play();
                }
            }
            WindComp->SetVolumeMultiplier(WeatherIntensity * 0.7f * MasterAmbientVolume);
        }
    }
}

void AEnvironmentalAudioManager::ProcessProceduralSounds()
{
    if (!bEnableProceduralSounds)
    {
        return;
    }

    // Random chance to trigger procedural sounds
    if (FMath::RandRange(0.0f, 1.0f) < RandomSoundFrequency)
    {
        // Choose random sound type
        int32 SoundType = FMath::RandRange(0, 2);
        FVector RandomLocation = GetRandomLocationAroundPlayer(
            RandomSoundDistanceRange.X, 
            RandomSoundDistanceRange.Y
        );
        
        switch (SoundType)
        {
            case 0:
                TriggerDistantCreatureCall(RandomLocation, FMath::RandRange(0.3f, 0.8f));
                break;
            case 1:
                TriggerVegetationSound(RandomLocation, FMath::RandRange(0.2f, 0.5f));
                break;
            case 2:
                if (CurrentZone == EEnvironmentalZone::Wetlands || CurrentZone == EEnvironmentalZone::Coastline)
                {
                    TriggerWaterSound(RandomLocation, FMath::RandRange(0.4f, 0.7f));
                }
                break;
        }
    }
}

void AEnvironmentalAudioManager::UpdateSpatialAudioSettings()
{
    // Update spatial audio settings for random elements based on player location
    for (UAudioComponent* RandomComp : RandomElementLayers)
    {
        if (RandomComp && RandomComp->IsPlaying())
        {
            float Distance = FVector::Dist(RandomComp->GetComponentLocation(), LastPlayerLocation);
            if (Distance > MaxAudibleDistance)
            {
                RandomComp->Stop(); // Stop sounds that are too far away
            }
        }
    }
}

void AEnvironmentalAudioManager::PlayRandomElementSound()
{
    FEnvironmentalSoundscape* CurrentSoundscape = ZoneSoundscapes.Find(CurrentZone);
    if (CurrentSoundscape && CurrentSoundscape->RandomElements.Num() > 0)
    {
        if (FMath::RandRange(0.0f, 1.0f) < CurrentSoundscape->RandomElementChance)
        {
            USoundBase* RandomElement = GetRandomSoundFromArray(CurrentSoundscape->RandomElements);
            if (RandomElement)
            {
                FVector RandomLocation = GetRandomLocationAroundPlayer(300.0f, 1500.0f);
                
                // Find available random element layer
                for (UAudioComponent* RandomComp : RandomElementLayers)
                {
                    if (RandomComp && !RandomComp->IsPlaying())
                    {
                        RandomComp->SetSound(RandomElement);
                        RandomComp->SetWorldLocation(RandomLocation);
                        
                        float Distance = FVector::Dist(RandomLocation, LastPlayerLocation);
                        float DistanceVolume = CalculateVolumeFromDistance(Distance);
                        RandomComp->SetVolumeMultiplier(DistanceVolume * MasterAmbientVolume * 0.6f);
                        
                        RandomComp->Play();
                        break;
                    }
                }
            }
        }
    }
    
    // Schedule next random element
    float NextInterval = FMath::RandRange(
        CurrentSoundscape ? CurrentSoundscape->RandomElementInterval.X : 30.0f,
        CurrentSoundscape ? CurrentSoundscape->RandomElementInterval.Y : 120.0f
    );
    
    GetWorld()->GetTimerManager().SetTimer(
        RandomElementTimer,
        this,
        &AEnvironmentalAudioManager::PlayRandomElementSound,
        NextInterval,
        false
    );
}

FVector AEnvironmentalAudioManager::GetRandomLocationAroundPlayer(float MinDistance, float MaxDistance) const
{
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    float RandomDistance = FMath::RandRange(MinDistance, MaxDistance);
    
    FVector RandomDirection = FVector(
        FMath::Cos(FMath::DegreesToRadians(RandomAngle)),
        FMath::Sin(FMath::DegreesToRadians(RandomAngle)),
        FMath::RandRange(-0.2f, 0.2f) // Slight vertical variation
    );
    
    return LastPlayerLocation + (RandomDirection * RandomDistance);
}

USoundBase* AEnvironmentalAudioManager::GetRandomSoundFromArray(const TArray<USoundBase*>& SoundArray) const
{
    if (SoundArray.Num() == 0)
    {
        return nullptr;
    }
    
    int32 RandomIndex = FMath::RandRange(0, SoundArray.Num() - 1);
    return SoundArray[RandomIndex];
}

float AEnvironmentalAudioManager::CalculateVolumeFromDistance(float Distance) const
{
    if (Distance <= 0.0f)
    {
        return 1.0f;
    }
    
    float NormalizedDistance = Distance / MaxAudibleDistance;
    return FMath::Pow(1.0f - FMath::Clamp(NormalizedDistance, 0.0f, 1.0f), SpatialAudioFalloffExponent);
}

void AEnvironmentalAudioManager::CleanupFinishedAudioComponents()
{
    // Stop and clean up any audio components that have finished playing
    for (UAudioComponent* RandomComp : RandomElementLayers)
    {
        if (RandomComp && !RandomComp->IsPlaying() && RandomComp->GetSound())
        {
            RandomComp->SetSound(nullptr); // Clear the sound reference
        }
    }
}