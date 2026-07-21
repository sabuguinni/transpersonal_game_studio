#include "World_BiomeClimateSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UWorld_BiomeClimateSystem::UWorld_BiomeClimateSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    ClimateUpdateInterval = 5.0f;
    TemperatureVariationSpeed = 0.1f;
    HumidityVariationSpeed = 0.05f;
    WindVariationSpeed = 0.2f;
    
    ClimateUpdateTimer = 0.0f;
    CurrentTimeOfDay = 12.0f;
    CurrentSeasonProgress = 0.25f; // Spring in Cretaceous
}

void UWorld_BiomeClimateSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeClimateProfiles();
    
    // Initialize current climate data for each biome
    for (const auto& Profile : BiomeClimateProfiles)
    {
        CurrentBiomeClimates.Add(Profile.Key, Profile.Value.BaseClimate);
        CreateBiomeAudioComponent(Profile.Key);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeClimateSystem: Initialized with %d biome climate profiles"), BiomeClimateProfiles.Num());
}

void UWorld_BiomeClimateSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ClimateUpdateTimer += DeltaTime;
    
    if (ClimateUpdateTimer >= ClimateUpdateInterval)
    {
        UpdateClimateVariations(DeltaTime);
        ClimateUpdateTimer = 0.0f;
    }
    
    // Update time of day (simplified 24-hour cycle)
    CurrentTimeOfDay += DeltaTime * 0.01f; // Slow time progression
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay = 0.0f;
    }
}

void UWorld_BiomeClimateSystem::InitializeBiomeClimateProfiles()
{
    // Forest Biome - Humid, moderate temperature
    FWorld_BiomeClimateProfile ForestProfile;
    ForestProfile.BiomeType = EBiomeType::Forest;
    ForestProfile.BaseClimate.Temperature = 26.0f; // Warm Cretaceous climate
    ForestProfile.BaseClimate.Humidity = 0.8f; // High humidity
    ForestProfile.BaseClimate.WindSpeed = 3.0f;
    ForestProfile.BaseClimate.WindDirection = FVector(1.0f, 0.5f, 0.0f);
    ForestProfile.BaseClimate.CloudCover = 0.6f;
    ForestProfile.TemperatureFluctuationRange = 4.0f;
    ForestProfile.HumidityFluctuationRange = 0.15f;
    ForestProfile.WindVariability = 1.5f;
    BiomeClimateProfiles.Add(EBiomeType::Forest, ForestProfile);
    
    // Plains Biome - Dry, windy
    FWorld_BiomeClimateProfile PlainsProfile;
    PlainsProfile.BiomeType = EBiomeType::Plains;
    PlainsProfile.BaseClimate.Temperature = 28.0f; // Warmer, open area
    PlainsProfile.BaseClimate.Humidity = 0.4f; // Lower humidity
    PlainsProfile.BaseClimate.WindSpeed = 8.0f; // Higher wind speed
    PlainsProfile.BaseClimate.WindDirection = FVector(1.0f, 0.0f, 0.0f);
    PlainsProfile.BaseClimate.CloudCover = 0.3f;
    PlainsProfile.TemperatureFluctuationRange = 8.0f; // Greater temperature variation
    PlainsProfile.HumidityFluctuationRange = 0.2f;
    PlainsProfile.WindVariability = 3.0f;
    BiomeClimateProfiles.Add(EBiomeType::Plains, PlainsProfile);
    
    // Desert Biome - Hot, dry, extreme variations
    FWorld_BiomeClimateProfile DesertProfile;
    DesertProfile.BiomeType = EBiomeType::Desert;
    DesertProfile.BaseClimate.Temperature = 35.0f; // Hot
    DesertProfile.BaseClimate.Humidity = 0.2f; // Very low humidity
    DesertProfile.BaseClimate.WindSpeed = 12.0f; // Strong winds
    DesertProfile.BaseClimate.WindDirection = FVector(0.8f, 0.6f, 0.0f);
    DesertProfile.BaseClimate.CloudCover = 0.1f; // Clear skies
    DesertProfile.TemperatureFluctuationRange = 15.0f; // Extreme day/night variation
    DesertProfile.HumidityFluctuationRange = 0.1f;
    DesertProfile.WindVariability = 5.0f;
    BiomeClimateProfiles.Add(EBiomeType::Desert, DesertProfile);
    
    // Swamp Biome - Very humid, warm, stagnant air
    FWorld_BiomeClimateProfile SwampProfile;
    SwampProfile.BiomeType = EBiomeType::Swamp;
    SwampProfile.BaseClimate.Temperature = 30.0f; // Warm and humid
    SwampProfile.BaseClimate.Humidity = 0.95f; // Very high humidity
    SwampProfile.BaseClimate.WindSpeed = 1.0f; // Low wind
    SwampProfile.BaseClimate.WindDirection = FVector(0.3f, 0.3f, 0.0f);
    SwampProfile.BaseClimate.CloudCover = 0.7f; // Often cloudy
    SwampProfile.BaseClimate.Precipitation = 2.0f; // Frequent rain
    SwampProfile.TemperatureFluctuationRange = 3.0f;
    SwampProfile.HumidityFluctuationRange = 0.05f; // Stable humidity
    SwampProfile.WindVariability = 0.5f;
    BiomeClimateProfiles.Add(EBiomeType::Swamp, SwampProfile);
    
    // Mountain Biome - Cool, variable weather
    FWorld_BiomeClimateProfile MountainProfile;
    MountainProfile.BiomeType = EBiomeType::Mountain;
    MountainProfile.BaseClimate.Temperature = 18.0f; // Cooler at altitude
    MountainProfile.BaseClimate.Humidity = 0.6f;
    MountainProfile.BaseClimate.WindSpeed = 15.0f; // Strong mountain winds
    MountainProfile.BaseClimate.WindDirection = FVector(0.0f, 1.0f, 0.2f);
    MountainProfile.BaseClimate.CloudCover = 0.5f;
    MountainProfile.BaseClimate.AtmosphericPressure = 85000.0f; // Lower pressure at altitude
    MountainProfile.TemperatureFluctuationRange = 10.0f;
    MountainProfile.HumidityFluctuationRange = 0.3f;
    MountainProfile.WindVariability = 8.0f;
    BiomeClimateProfiles.Add(EBiomeType::Mountain, MountainProfile);
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeClimateSystem: Initialized %d biome climate profiles"), BiomeClimateProfiles.Num());
}

FWorld_ClimateData UWorld_BiomeClimateSystem::GetCurrentClimate(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeTypeAtLocation(Location);
    
    if (const FWorld_ClimateData* ClimateData = CurrentBiomeClimates.Find(BiomeType))
    {
        FWorld_ClimateData ModifiedClimate = *ClimateData;
        
        // Apply time of day modifications
        float DailyTempModifier = GetDailyTemperatureModifier(CurrentTimeOfDay);
        ModifiedClimate.Temperature += DailyTempModifier;
        
        // Apply seasonal modifications
        ModifiedClimate = ApplySeasonalModifiers(ModifiedClimate, CurrentSeasonProgress);
        
        return ModifiedClimate;
    }
    
    // Default climate if biome not found
    FWorld_ClimateData DefaultClimate;
    DefaultClimate.Temperature = 25.0f;
    DefaultClimate.Humidity = 0.6f;
    return DefaultClimate;
}

void UWorld_BiomeClimateSystem::UpdateClimateForBiome(EBiomeType BiomeType, const FWorld_ClimateData& NewClimate)
{
    if (CurrentBiomeClimates.Contains(BiomeType))
    {
        CurrentBiomeClimates[BiomeType] = NewClimate;
        UE_LOG(LogTemp, Log, TEXT("Updated climate for biome %d: Temp=%.1f, Humidity=%.2f"), 
               (int32)BiomeType, NewClimate.Temperature, NewClimate.Humidity);
    }
}

float UWorld_BiomeClimateSystem::GetTemperatureAtLocation(const FVector& Location) const
{
    FWorld_ClimateData Climate = GetCurrentClimate(Location);
    return Climate.Temperature;
}

float UWorld_BiomeClimateSystem::GetHumidityAtLocation(const FVector& Location) const
{
    FWorld_ClimateData Climate = GetCurrentClimate(Location);
    return Climate.Humidity;
}

FVector UWorld_BiomeClimateSystem::GetWindAtLocation(const FVector& Location) const
{
    FWorld_ClimateData Climate = GetCurrentClimate(Location);
    return Climate.WindDirection * Climate.WindSpeed;
}

FWorld_BiomeClimateProfile UWorld_BiomeClimateSystem::GetBiomeClimateProfile(EBiomeType BiomeType) const
{
    if (const FWorld_BiomeClimateProfile* Profile = BiomeClimateProfiles.Find(BiomeType))
    {
        return *Profile;
    }
    
    return FWorld_BiomeClimateProfile(); // Return default profile
}

void UWorld_BiomeClimateSystem::SetBiomeClimateProfile(EBiomeType BiomeType, const FWorld_BiomeClimateProfile& Profile)
{
    BiomeClimateProfiles.Add(BiomeType, Profile);
    CurrentBiomeClimates.Add(BiomeType, Profile.BaseClimate);
}

void UWorld_BiomeClimateSystem::TriggerRainfall(EBiomeType BiomeType, float Intensity, float Duration)
{
    if (FWorld_ClimateData* Climate = CurrentBiomeClimates.Find(BiomeType))
    {
        Climate->Precipitation = Intensity;
        Climate->CloudCover = FMath::Clamp(Climate->CloudCover + 0.3f, 0.0f, 1.0f);
        Climate->Humidity = FMath::Clamp(Climate->Humidity + 0.2f, 0.0f, 1.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("Triggered rainfall in biome %d: Intensity=%.2f, Duration=%.1f"), 
               (int32)BiomeType, Intensity, Duration);
    }
}

void UWorld_BiomeClimateSystem::TriggerWindStorm(EBiomeType BiomeType, float WindSpeed, const FVector& Direction)
{
    if (FWorld_ClimateData* Climate = CurrentBiomeClimates.Find(BiomeType))
    {
        Climate->WindSpeed = WindSpeed;
        Climate->WindDirection = Direction.GetSafeNormal();
        
        UE_LOG(LogTemp, Warning, TEXT("Triggered wind storm in biome %d: Speed=%.1f"), 
               (int32)BiomeType, WindSpeed);
    }
}

void UWorld_BiomeClimateSystem::ClearWeather(EBiomeType BiomeType)
{
    if (const FWorld_BiomeClimateProfile* Profile = BiomeClimateProfiles.Find(BiomeType))
    {
        CurrentBiomeClimates[BiomeType] = Profile->BaseClimate;
        UE_LOG(LogTemp, Log, TEXT("Cleared weather for biome %d"), (int32)BiomeType);
    }
}

void UWorld_BiomeClimateSystem::UpdateBiomeAudio(EBiomeType BiomeType, const FVector& PlayerLocation)
{
    if (UAudioComponent** AudioComp = BiomeAudioComponents.Find(BiomeType))
    {
        if (*AudioComp && IsValid(*AudioComp))
        {
            FWorld_ClimateData Climate = GetCurrentClimate(PlayerLocation);
            
            // Adjust volume based on wind speed and proximity
            float Volume = FMath::Clamp(Climate.WindSpeed / 20.0f, 0.1f, 1.0f);
            (*AudioComp)->SetVolumeMultiplier(Volume);
            
            // Adjust pitch based on temperature (subtle effect)
            float Pitch = FMath::Clamp(1.0f + (Climate.Temperature - 25.0f) * 0.01f, 0.8f, 1.2f);
            (*AudioComp)->SetPitchMultiplier(Pitch);
        }
    }
}

void UWorld_BiomeClimateSystem::PlayClimateAudio(const FWorld_ClimateData& Climate, const FVector& Location)
{
    // This would play specific audio based on climate conditions
    // For now, just log the climate state
    UE_LOG(LogTemp, Log, TEXT("Climate Audio at location: Temp=%.1f, Wind=%.1f, Rain=%.2f"), 
           Climate.Temperature, Climate.WindSpeed, Climate.Precipitation);
}

void UWorld_BiomeClimateSystem::UpdateDailyClimateVariation(float TimeOfDay)
{
    CurrentTimeOfDay = TimeOfDay;
    
    // Update all biome climates with daily variations
    for (auto& ClimatePair : CurrentBiomeClimates)
    {
        EBiomeType BiomeType = ClimatePair.Key;
        FWorld_ClimateData& Climate = ClimatePair.Value;
        
        if (const FWorld_BiomeClimateProfile* Profile = BiomeClimateProfiles.Find(BiomeType))
        {
            // Apply daily temperature variation
            float DailyModifier = GetDailyTemperatureModifier(TimeOfDay);
            Climate.Temperature = Profile->BaseClimate.Temperature + DailyModifier;
            
            // Humidity tends to be higher at night
            float HumidityModifier = FMath::Sin((TimeOfDay + 6.0f) * PI / 12.0f) * 0.1f;
            Climate.Humidity = FMath::Clamp(Profile->BaseClimate.Humidity + HumidityModifier, 0.0f, 1.0f);
        }
    }
}

float UWorld_BiomeClimateSystem::GetDailyTemperatureModifier(float TimeOfDay) const
{
    // Temperature peaks at 14:00 (2 PM) and is lowest at 6:00 AM
    float CyclePosition = (TimeOfDay - 6.0f) / 24.0f * 2.0f * PI;
    return FMath::Sin(CyclePosition) * 8.0f; // ±8°C daily variation
}

void UWorld_BiomeClimateSystem::UpdateSeasonalClimate(float SeasonProgress)
{
    CurrentSeasonProgress = SeasonProgress;
    
    // Update base climates with seasonal variations
    for (auto& ClimatePair : CurrentBiomeClimates)
    {
        EBiomeType BiomeType = ClimatePair.Key;
        FWorld_ClimateData& Climate = ClimatePair.Value;
        
        if (const FWorld_BiomeClimateProfile* Profile = BiomeClimateProfiles.Find(BiomeType))
        {
            Climate = ApplySeasonalModifiers(Profile->BaseClimate, SeasonProgress);
        }
    }
}

FWorld_ClimateData UWorld_BiomeClimateSystem::ApplySeasonalModifiers(const FWorld_ClimateData& BaseClimate, float SeasonProgress) const
{
    FWorld_ClimateData ModifiedClimate = BaseClimate;
    
    // Seasonal temperature variation (warmer in summer, cooler in winter)
    float SeasonalTempModifier = FMath::Sin(SeasonProgress * 2.0f * PI) * 5.0f;
    ModifiedClimate.Temperature += SeasonalTempModifier;
    
    // Seasonal precipitation (more rain in certain seasons)
    float SeasonalRainModifier = FMath::Sin((SeasonProgress + 0.25f) * 2.0f * PI) * 1.0f;
    ModifiedClimate.Precipitation = FMath::Max(0.0f, BaseClimate.Precipitation + SeasonalRainModifier);
    
    return ModifiedClimate;
}

EBiomeType UWorld_BiomeClimateSystem::GetBiomeTypeAtLocation(const FVector& Location) const
{
    // Simple biome determination based on location
    // In a real implementation, this would query the biome system
    float X = Location.X;
    float Y = Location.Y;
    float Z = Location.Z;
    
    // High altitude = Mountain
    if (Z > 500.0f)
    {
        return EBiomeType::Mountain;
    }
    
    // Low areas near water = Swamp
    if (Z < 50.0f && FMath::Abs(X) < 1000.0f)
    {
        return EBiomeType::Swamp;
    }
    
    // Far from center = Desert
    float DistanceFromCenter = FVector2D(X, Y).Size();
    if (DistanceFromCenter > 5000.0f)
    {
        return EBiomeType::Desert;
    }
    
    // Open areas = Plains
    if (DistanceFromCenter > 2000.0f && Z < 200.0f)
    {
        return EBiomeType::Plains;
    }
    
    // Default = Forest
    return EBiomeType::Forest;
}

FWorld_ClimateData UWorld_BiomeClimateSystem::InterpolateClimate(const FWorld_ClimateData& ClimateA, const FWorld_ClimateData& ClimateB, float Alpha) const
{
    FWorld_ClimateData Result;
    
    Result.Temperature = FMath::Lerp(ClimateA.Temperature, ClimateB.Temperature, Alpha);
    Result.Humidity = FMath::Lerp(ClimateA.Humidity, ClimateB.Humidity, Alpha);
    Result.WindSpeed = FMath::Lerp(ClimateA.WindSpeed, ClimateB.WindSpeed, Alpha);
    Result.WindDirection = FMath::Lerp(ClimateA.WindDirection, ClimateB.WindDirection, Alpha);
    Result.AtmosphericPressure = FMath::Lerp(ClimateA.AtmosphericPressure, ClimateB.AtmosphericPressure, Alpha);
    Result.Precipitation = FMath::Lerp(ClimateA.Precipitation, ClimateB.Precipitation, Alpha);
    Result.CloudCover = FMath::Lerp(ClimateA.CloudCover, ClimateB.CloudCover, Alpha);
    
    return Result;
}

void UWorld_BiomeClimateSystem::UpdateClimateVariations(float DeltaTime)
{
    for (auto& ClimatePair : CurrentBiomeClimates)
    {
        EBiomeType BiomeType = ClimatePair.Key;
        FWorld_ClimateData& Climate = ClimatePair.Value;
        
        if (const FWorld_BiomeClimateProfile* Profile = BiomeClimateProfiles.Find(BiomeType))
        {
            // Add random variations within the fluctuation ranges
            float TempVariation = FMath::RandRange(-Profile->TemperatureFluctuationRange, Profile->TemperatureFluctuationRange) * TemperatureVariationSpeed * DeltaTime;
            Climate.Temperature = FMath::Clamp(Climate.Temperature + TempVariation, 
                                             Profile->BaseClimate.Temperature - Profile->TemperatureFluctuationRange,
                                             Profile->BaseClimate.Temperature + Profile->TemperatureFluctuationRange);
            
            float HumidityVariation = FMath::RandRange(-Profile->HumidityFluctuationRange, Profile->HumidityFluctuationRange) * HumidityVariationSpeed * DeltaTime;
            Climate.Humidity = FMath::Clamp(Climate.Humidity + HumidityVariation, 0.0f, 1.0f);
            
            float WindVariation = FMath::RandRange(-Profile->WindVariability, Profile->WindVariability) * WindVariationSpeed * DeltaTime;
            Climate.WindSpeed = FMath::Max(0.0f, Climate.WindSpeed + WindVariation);
        }
    }
}

void UWorld_BiomeClimateSystem::CreateBiomeAudioComponent(EBiomeType BiomeType)
{
    if (AActor* Owner = GetOwner())
    {
        UAudioComponent* AudioComp = NewObject<UAudioComponent>(Owner);
        if (AudioComp)
        {
            AudioComp->SetupAttachment(Owner->GetRootComponent());
            AudioComp->bAutoActivate = true;
            AudioComp->SetVolumeMultiplier(0.5f);
            
            BiomeAudioComponents.Add(BiomeType, AudioComp);
            
            UE_LOG(LogTemp, Log, TEXT("Created audio component for biome %d"), (int32)BiomeType);
        }
    }
}