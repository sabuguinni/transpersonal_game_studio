#include "World_PrehistoricSoundscape.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

UWorld_PrehistoricSoundscape::UWorld_PrehistoricSoundscape()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update twice per second
    
    GlobalVolumeMultiplier = 1.0f;
    UpdateFrequency = 2.0f;
    MaxSimultaneousSounds = 8;
    bEnableTimeOfDayVariation = true;
    bEnableWeatherVariation = true;
    bEnablePlayerProximityFade = true;
    
    LastPlayerLocation = FVector::ZeroVector;
    TimeSinceLastUpdate = 0.0f;
}

void UWorld_PrehistoricSoundscape::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSoundscape();
    LoadDefaultSounds();
    
    UE_LOG(LogTemp, Log, TEXT("World_PrehistoricSoundscape: Initialized with %d zones"), SoundscapeZones.Num());
}

void UWorld_PrehistoricSoundscape::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopAllSounds();
    Super::EndPlay(EndPlayReason);
}

void UWorld_PrehistoricSoundscape::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= (1.0f / UpdateFrequency))
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            FVector CurrentPlayerLocation = PlayerPawn->GetActorLocation();
            UpdateSoundscape(CurrentPlayerLocation);
            LastPlayerLocation = CurrentPlayerLocation;
        }
        
        CleanupInactiveSounds();
        UpdateSoundParameters();
        TimeSinceLastUpdate = 0.0f;
    }
}

void UWorld_PrehistoricSoundscape::InitializeSoundscape()
{
    // Create default soundscape zones if none exist
    if (SoundscapeZones.Num() == 0)
    {
        // Forest zone
        FWorld_SoundscapeZone ForestZone;
        ForestZone.ZoneCenter = FVector(0, 0, 0);
        ForestZone.ZoneRadius = 15000.0f;
        ForestZone.PrimarySoundType = EWorld_PrehistoricSoundType::ForestAmbient;
        ForestZone.SecondarySounds.Add(EWorld_PrehistoricSoundType::RiverFlow);
        SoundscapeZones.Add(ForestZone);
        
        // Plains zone
        FWorld_SoundscapeZone PlainsZone;
        PlainsZone.ZoneCenter = FVector(20000, 0, 0);
        PlainsZone.ZoneRadius = 12000.0f;
        PlainsZone.PrimarySoundType = EWorld_PrehistoricSoundType::PlainWinds;
        PlainsZone.SecondarySounds.Add(EWorld_PrehistoricSoundType::DesertWind);
        SoundscapeZones.Add(PlainsZone);
        
        // Volcanic zone
        FWorld_SoundscapeZone VolcanicZone;
        VolcanicZone.ZoneCenter = FVector(-15000, -15000, 0);
        VolcanicZone.ZoneRadius = 8000.0f;
        VolcanicZone.PrimarySoundType = EWorld_PrehistoricSoundType::VolcanicRumble;
        VolcanicZone.IntensityMultiplier = 1.5f;
        SoundscapeZones.Add(VolcanicZone);
        
        // River zone
        FWorld_SoundscapeZone RiverZone;
        RiverZone.ZoneCenter = FVector(5000, 10000, 0);
        RiverZone.ZoneRadius = 3000.0f;
        RiverZone.PrimarySoundType = EWorld_PrehistoricSoundType::RiverFlow;
        RiverZone.SecondarySounds.Add(EWorld_PrehistoricSoundType::SwampBubbles);
        SoundscapeZones.Add(RiverZone);
    }
    
    UE_LOG(LogTemp, Log, TEXT("World_PrehistoricSoundscape: Initialized %d soundscape zones"), SoundscapeZones.Num());
}

void UWorld_PrehistoricSoundscape::UpdateSoundscape(const FVector& PlayerLocation)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get active zones based on player location
    TArray<FWorld_SoundscapeZone> ActiveZones = GetActiveSoundscapeZones(PlayerLocation, 20000.0f);
    
    // Process zone transitions
    ProcessZoneTransitions(PlayerLocation);
    
    // Update existing sounds based on proximity
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp))
        {
            FVector SoundLocation = AudioComp->GetComponentLocation();
            float Distance = FVector::Dist(PlayerLocation, SoundLocation);
            float VolumeMultiplier = GetDistanceAttenuation(SoundLocation, PlayerLocation, 10000.0f);
            
            // Apply time of day and weather variations
            if (bEnableTimeOfDayVariation)
            {
                VolumeMultiplier *= GetTimeOfDayMultiplier();
            }
            
            if (bEnableWeatherVariation)
            {
                VolumeMultiplier *= GetWeatherMultiplier();
            }
            
            AudioComp->SetVolumeMultiplier(VolumeMultiplier * GlobalVolumeMultiplier);
        }
    }
    
    // Spawn new sounds for active zones if needed
    for (const FWorld_SoundscapeZone& Zone : ActiveZones)
    {
        float ZoneInfluence = CalculateZoneInfluence(Zone, PlayerLocation);
        
        if (ZoneInfluence > 0.1f && ActiveAudioComponents.Num() < MaxSimultaneousSounds)
        {
            // Check if we already have a sound for this zone type
            bool bHasSoundForType = false;
            for (UAudioComponent* AudioComp : ActiveAudioComponents)
            {
                if (AudioComp && IsValid(AudioComp))
                {
                    // Simple check - in a real implementation you'd store the sound type with the component
                    FVector SoundLocation = AudioComp->GetComponentLocation();
                    if (FVector::Dist(SoundLocation, Zone.ZoneCenter) < Zone.ZoneRadius * 0.5f)
                    {
                        bHasSoundForType = true;
                        break;
                    }
                }
            }
            
            if (!bHasSoundForType)
            {
                FVector SoundSpawnLocation = Zone.ZoneCenter + FMath::VRand() * Zone.ZoneRadius * 0.3f;
                UAudioComponent* NewAudio = PlayPrehistoricSound(Zone.PrimarySoundType, SoundSpawnLocation);
                if (NewAudio)
                {
                    FadeInSound(NewAudio, 3.0f);
                }
            }
        }
    }
}

void UWorld_PrehistoricSoundscape::AddSoundscapeZone(const FWorld_SoundscapeZone& NewZone)
{
    SoundscapeZones.Add(NewZone);
    UE_LOG(LogTemp, Log, TEXT("World_PrehistoricSoundscape: Added new soundscape zone at %s"), *NewZone.ZoneCenter.ToString());
}

void UWorld_PrehistoricSoundscape::RemoveSoundscapeZone(int32 ZoneIndex)
{
    if (SoundscapeZones.IsValidIndex(ZoneIndex))
    {
        SoundscapeZones.RemoveAt(ZoneIndex);
        UE_LOG(LogTemp, Log, TEXT("World_PrehistoricSoundscape: Removed soundscape zone at index %d"), ZoneIndex);
    }
}

void UWorld_PrehistoricSoundscape::SetGlobalVolume(float NewVolume)
{
    GlobalVolumeMultiplier = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    
    // Update all active audio components
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp))
        {
            float CurrentVolume = AudioComp->GetVolumeMultiplier();
            AudioComp->SetVolumeMultiplier(CurrentVolume); // This will trigger recalculation
        }
    }
}

void UWorld_PrehistoricSoundscape::EnableTimeOfDayVariation(bool bEnable)
{
    bEnableTimeOfDayVariation = bEnable;
    UE_LOG(LogTemp, Log, TEXT("World_PrehistoricSoundscape: Time of day variation %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UWorld_PrehistoricSoundscape::EnableWeatherVariation(bool bEnable)
{
    bEnableWeatherVariation = bEnable;
    UE_LOG(LogTemp, Log, TEXT("World_PrehistoricSoundscape: Weather variation %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

UAudioComponent* UWorld_PrehistoricSoundscape::PlayPrehistoricSound(EWorld_PrehistoricSoundType SoundType, const FVector& Location, float VolumeOverride)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    const FWorld_PrehistoricSoundData* SoundData = SoundDatabase.Find(SoundType);
    if (!SoundData || !SoundData->SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("World_PrehistoricSoundscape: No sound data found for type %d"), (int32)SoundType);
        return nullptr;
    }
    
    UAudioComponent* AudioComp = CreateAudioComponent(SoundType, Location);
    if (AudioComp)
    {
        AudioComp->SetSound(SoundData->SoundCue);
        AudioComp->SetVolumeMultiplier((VolumeOverride >= 0.0f ? VolumeOverride : SoundData->Volume) * GlobalVolumeMultiplier);
        AudioComp->SetPitchMultiplier(SoundData->Pitch);
        AudioComp->bAutoDestroy = false;
        
        if (SoundData->bLooping)
        {
            AudioComp->Play();
        }
        else
        {
            AudioComp->Play();
        }
        
        ActiveAudioComponents.Add(AudioComp);
        UE_LOG(LogTemp, Log, TEXT("World_PrehistoricSoundscape: Playing sound type %d at location %s"), (int32)SoundType, *Location.ToString());
    }
    
    return AudioComp;
}

void UWorld_PrehistoricSoundscape::StopPrehistoricSound(EWorld_PrehistoricSoundType SoundType)
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (AudioComp && IsValid(AudioComp))
        {
            // In a real implementation, you'd store the sound type with the component
            // For now, we'll just fade out and remove
            FadeOutSound(AudioComp, 2.0f);
        }
    }
}

void UWorld_PrehistoricSoundscape::StopAllSounds()
{
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
    }
    
    ActiveAudioComponents.Empty();
    UE_LOG(LogTemp, Log, TEXT("World_PrehistoricSoundscape: Stopped all sounds"));
}

void UWorld_PrehistoricSoundscape::FadeInSound(UAudioComponent* AudioComp, float FadeTime)
{
    if (AudioComp && IsValid(AudioComp))
    {
        AudioComp->FadeIn(FadeTime, 1.0f);
    }
}

void UWorld_PrehistoricSoundscape::FadeOutSound(UAudioComponent* AudioComp, float FadeTime)
{
    if (AudioComp && IsValid(AudioComp))
    {
        AudioComp->FadeOut(FadeTime, 0.0f);
    }
}

FWorld_SoundscapeZone UWorld_PrehistoricSoundscape::GetNearestSoundscapeZone(const FVector& Location) const
{
    FWorld_SoundscapeZone NearestZone;
    float NearestDistance = FLT_MAX;
    
    for (const FWorld_SoundscapeZone& Zone : SoundscapeZones)
    {
        float Distance = FVector::Dist(Location, Zone.ZoneCenter);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestZone = Zone;
        }
    }
    
    return NearestZone;
}

TArray<FWorld_SoundscapeZone> UWorld_PrehistoricSoundscape::GetActiveSoundscapeZones(const FVector& Location, float Radius) const
{
    TArray<FWorld_SoundscapeZone> ActiveZones;
    
    for (const FWorld_SoundscapeZone& Zone : SoundscapeZones)
    {
        float Distance = FVector::Dist(Location, Zone.ZoneCenter);
        if (Distance <= (Zone.ZoneRadius + Radius))
        {
            ActiveZones.Add(Zone);
        }
    }
    
    return ActiveZones;
}

float UWorld_PrehistoricSoundscape::CalculateZoneInfluence(const FWorld_SoundscapeZone& Zone, const FVector& Location) const
{
    float Distance = FVector::Dist(Location, Zone.ZoneCenter);
    if (Distance >= Zone.ZoneRadius)
    {
        return 0.0f;
    }
    
    float Influence = 1.0f - (Distance / Zone.ZoneRadius);
    return FMath::Clamp(Influence * Zone.IntensityMultiplier, 0.0f, 1.0f);
}

float UWorld_PrehistoricSoundscape::GetTimeOfDayMultiplier() const
{
    if (!GetWorld())
    {
        return 1.0f;
    }
    
    // Simple time of day calculation based on world time
    float WorldTime = GetWorld()->GetTimeSeconds();
    float DayLength = 1200.0f; // 20 minutes per day
    float TimeOfDay = FMath::Fmod(WorldTime, DayLength) / DayLength;
    
    // Night sounds are quieter (0.5x), day sounds are normal (1.0x)
    if (TimeOfDay > 0.75f || TimeOfDay < 0.25f) // Night
    {
        return 0.7f;
    }
    else if (TimeOfDay > 0.45f && TimeOfDay < 0.55f) // Noon
    {
        return 1.2f;
    }
    
    return 1.0f; // Day
}

float UWorld_PrehistoricSoundscape::GetWeatherMultiplier() const
{
    // Placeholder for weather system integration
    // In a real implementation, this would query the weather system
    return FMath::RandRange(0.8f, 1.2f); // Random weather variation
}

float UWorld_PrehistoricSoundscape::GetDistanceAttenuation(const FVector& SoundLocation, const FVector& ListenerLocation, float MaxDistance) const
{
    float Distance = FVector::Dist(SoundLocation, ListenerLocation);
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    return 1.0f - (Distance / MaxDistance);
}

void UWorld_PrehistoricSoundscape::DebugDrawSoundscapeZones() const
{
    if (!GetWorld())
    {
        return;
    }
    
    for (const FWorld_SoundscapeZone& Zone : SoundscapeZones)
    {
        FColor ZoneColor = FColor::Green;
        switch (Zone.PrimarySoundType)
        {
            case EWorld_PrehistoricSoundType::ForestAmbient:
                ZoneColor = FColor::Green;
                break;
            case EWorld_PrehistoricSoundType::PlainWinds:
                ZoneColor = FColor::Yellow;
                break;
            case EWorld_PrehistoricSoundType::VolcanicRumble:
                ZoneColor = FColor::Red;
                break;
            case EWorld_PrehistoricSoundType::RiverFlow:
                ZoneColor = FColor::Blue;
                break;
            default:
                ZoneColor = FColor::White;
                break;
        }
        
        DrawDebugSphere(GetWorld(), Zone.ZoneCenter, Zone.ZoneRadius, 32, ZoneColor, false, 1.0f);
        DrawDebugString(GetWorld(), Zone.ZoneCenter + FVector(0, 0, 500), 
                       FString::Printf(TEXT("Zone: %d"), (int32)Zone.PrimarySoundType), 
                       nullptr, ZoneColor, 1.0f);
    }
}

void UWorld_PrehistoricSoundscape::LogActiveSounds() const
{
    UE_LOG(LogTemp, Log, TEXT("World_PrehistoricSoundscape: %d active sounds"), ActiveAudioComponents.Num());
    
    for (int32 i = 0; i < ActiveAudioComponents.Num(); i++)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (AudioComp && IsValid(AudioComp))
        {
            FVector Location = AudioComp->GetComponentLocation();
            float Volume = AudioComp->GetVolumeMultiplier();
            UE_LOG(LogTemp, Log, TEXT("  Sound %d: Location %s, Volume %.2f"), i, *Location.ToString(), Volume);
        }
    }
}

void UWorld_PrehistoricSoundscape::LogSoundscapeStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("World_PrehistoricSoundscape Status:"));
    UE_LOG(LogTemp, Log, TEXT("  Zones: %d"), SoundscapeZones.Num());
    UE_LOG(LogTemp, Log, TEXT("  Active Sounds: %d / %d"), ActiveAudioComponents.Num(), (int32)MaxSimultaneousSounds);
    UE_LOG(LogTemp, Log, TEXT("  Global Volume: %.2f"), GlobalVolumeMultiplier);
    UE_LOG(LogTemp, Log, TEXT("  Time of Day Variation: %s"), bEnableTimeOfDayVariation ? TEXT("ON") : TEXT("OFF"));
    UE_LOG(LogTemp, Log, TEXT("  Weather Variation: %s"), bEnableWeatherVariation ? TEXT("ON") : TEXT("OFF"));
}

void UWorld_PrehistoricSoundscape::CleanupInactiveSounds()
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (!AudioComp || !IsValid(AudioComp) || !AudioComp->IsPlaying())
        {
            if (AudioComp && IsValid(AudioComp))
            {
                AudioComp->DestroyComponent();
            }
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

void UWorld_PrehistoricSoundscape::UpdateSoundParameters()
{
    float TimeMultiplier = bEnableTimeOfDayVariation ? GetTimeOfDayMultiplier() : 1.0f;
    float WeatherMultiplier = bEnableWeatherVariation ? GetWeatherMultiplier() : 1.0f;
    
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp))
        {
            float BaseVolume = AudioComp->GetVolumeMultiplier();
            float NewVolume = BaseVolume * TimeMultiplier * WeatherMultiplier * GlobalVolumeMultiplier;
            AudioComp->SetVolumeMultiplier(NewVolume);
        }
    }
}

void UWorld_PrehistoricSoundscape::ProcessZoneTransitions(const FVector& PlayerLocation)
{
    // Get the current dominant zone
    FWorld_SoundscapeZone CurrentZone = GetNearestSoundscapeZone(PlayerLocation);
    float CurrentInfluence = CalculateZoneInfluence(CurrentZone, PlayerLocation);
    
    // Fade out sounds that are too far from active zones
    TArray<FWorld_SoundscapeZone> ActiveZones = GetActiveSoundscapeZones(PlayerLocation, 15000.0f);
    
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (AudioComp && IsValid(AudioComp))
        {
            FVector SoundLocation = AudioComp->GetComponentLocation();
            bool bInActiveZone = false;
            
            for (const FWorld_SoundscapeZone& Zone : ActiveZones)
            {
                if (FVector::Dist(SoundLocation, Zone.ZoneCenter) < Zone.ZoneRadius * 1.2f)
                {
                    bInActiveZone = true;
                    break;
                }
            }
            
            if (!bInActiveZone)
            {
                FadeOutSound(AudioComp, 4.0f);
            }
        }
    }
}

UAudioComponent* UWorld_PrehistoricSoundscape::CreateAudioComponent(EWorld_PrehistoricSoundType SoundType, const FVector& Location)
{
    if (!GetOwner())
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(GetOwner());
    if (AudioComp)
    {
        AudioComp->SetWorldLocation(Location);
        AudioComp->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        AudioComp->RegisterComponent();
    }
    
    return AudioComp;
}

void UWorld_PrehistoricSoundscape::LoadDefaultSounds()
{
    // Initialize default sound database
    // In a real implementation, these would be loaded from assets
    
    FWorld_PrehistoricSoundData ForestData;
    ForestData.Volume = 0.8f;
    ForestData.AttenuationRadius = 8000.0f;
    ForestData.bLooping = true;
    SoundDatabase.Add(EWorld_PrehistoricSoundType::ForestAmbient, ForestData);
    
    FWorld_PrehistoricSoundData WindData;
    WindData.Volume = 0.6f;
    WindData.AttenuationRadius = 12000.0f;
    WindData.bLooping = true;
    SoundDatabase.Add(EWorld_PrehistoricSoundType::PlainWinds, WindData);
    
    FWorld_PrehistoricSoundData RiverData;
    RiverData.Volume = 0.9f;
    RiverData.AttenuationRadius = 5000.0f;
    RiverData.bLooping = true;
    SoundDatabase.Add(EWorld_PrehistoricSoundType::RiverFlow, RiverData);
    
    FWorld_PrehistoricSoundData VolcanicData;
    VolcanicData.Volume = 1.2f;
    VolcanicData.AttenuationRadius = 15000.0f;
    VolcanicData.bLooping = true;
    SoundDatabase.Add(EWorld_PrehistoricSoundType::VolcanicRumble, VolcanicData);
    
    FWorld_PrehistoricSoundData CaveData;
    CaveData.Volume = 0.5f;
    CaveData.AttenuationRadius = 3000.0f;
    CaveData.bLooping = true;
    SoundDatabase.Add(EWorld_PrehistoricSoundType::CaveEcho, CaveData);
    
    FWorld_PrehistoricSoundData SwampData;
    SwampData.Volume = 0.7f;
    SwampData.AttenuationRadius = 4000.0f;
    SwampData.bLooping = true;
    SoundDatabase.Add(EWorld_PrehistoricSoundType::SwampBubbles, SwampData);
    
    FWorld_PrehistoricSoundData DesertData;
    DesertData.Volume = 0.4f;
    DesertData.AttenuationRadius = 10000.0f;
    DesertData.bLooping = true;
    SoundDatabase.Add(EWorld_PrehistoricSoundType::DesertWind, DesertData);
    
    FWorld_PrehistoricSoundData OceanData;
    OceanData.Volume = 1.0f;
    OceanData.AttenuationRadius = 20000.0f;
    OceanData.bLooping = true;
    SoundDatabase.Add(EWorld_PrehistoricSoundType::OceanWaves, OceanData);
    
    UE_LOG(LogTemp, Log, TEXT("World_PrehistoricSoundscape: Loaded %d default sound types"), SoundDatabase.Num());
}

float UWorld_PrehistoricSoundscape::CalculateVolumeForLocation(const FVector& SoundLocation, const FVector& PlayerLocation, float BaseVolume) const
{
    float Distance = FVector::Dist(SoundLocation, PlayerLocation);
    float Attenuation = GetDistanceAttenuation(SoundLocation, PlayerLocation, 10000.0f);
    
    float FinalVolume = BaseVolume * Attenuation;
    
    if (bEnableTimeOfDayVariation)
    {
        FinalVolume *= GetTimeOfDayMultiplier();
    }
    
    if (bEnableWeatherVariation)
    {
        FinalVolume *= GetWeatherMultiplier();
    }
    
    return FMath::Clamp(FinalVolume * GlobalVolumeMultiplier, 0.0f, 2.0f);
}