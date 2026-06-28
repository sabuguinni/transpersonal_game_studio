// PCGBiomeSystem.cpp
// Agent #5 — Procedural World Generator
// Biome zone management, weather transitions, day/night cycle

#include "PCGBiomeSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"

APCGBiomeSystem::APCGBiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5s for performance

    // Default weather change interval
    NextWeatherChange = WeatherCycleIntervalMin;
}

void APCGBiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    SetupDefaultBiomeZones();
    InitializeBiomeZones();
}

void APCGBiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        TickDayNightCycle(DeltaTime);
    }

    if (bEnableWeatherCycle)
    {
        TickWeatherCycle(DeltaTime);
    }

    // Update player biome every tick (0.5s interval)
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                UpdatePlayerBiome(PlayerPawn->GetActorLocation());
            }
        }
    }
}

void APCGBiomeSystem::SetupDefaultBiomeZones()
{
    BiomeZones.Empty();

    // River Delta — center of map, water-heavy
    FWorld_BiomeZone RiverDelta;
    RiverDelta.BiomeType = EWorld_BiomeType::RiverDelta;
    RiverDelta.CenterLocation = FVector(0.0f, -400.0f, -45.0f);
    RiverDelta.Radius = 1500.0f;
    RiverDelta.VegetationDensity = 1.8f;
    RiverDelta.WaterLevel = -45.0f;
    RiverDelta.FogColor = FLinearColor(0.4f, 0.55f, 0.6f, 1.0f);
    RiverDelta.FogDensity = 0.04f;
    RiverDelta.AmbientTemperature = 28.0f;
    RiverDelta.bHasWater = true;
    BiomeZones.Add(RiverDelta);

    // Rocky Highlands — northeast, elevated
    FWorld_BiomeZone RockyHighlands;
    RockyHighlands.BiomeType = EWorld_BiomeType::RockyHighlands;
    RockyHighlands.CenterLocation = FVector(1200.0f, 800.0f, 80.0f);
    RockyHighlands.Radius = 1200.0f;
    RockyHighlands.VegetationDensity = 0.4f;
    RockyHighlands.WaterLevel = -200.0f;
    RockyHighlands.FogColor = FLinearColor(0.6f, 0.6f, 0.65f, 1.0f);
    RockyHighlands.FogDensity = 0.015f;
    RockyHighlands.AmbientTemperature = 18.0f;
    RockyHighlands.bHasWater = false;
    BiomeZones.Add(RockyHighlands);

    // Dense Forest — northwest
    FWorld_BiomeZone DenseForest;
    DenseForest.BiomeType = EWorld_BiomeType::DenseForest;
    DenseForest.CenterLocation = FVector(-850.0f, 600.0f, 0.0f);
    DenseForest.Radius = 1400.0f;
    DenseForest.VegetationDensity = 2.5f;
    DenseForest.WaterLevel = -100.0f;
    DenseForest.FogColor = FLinearColor(0.3f, 0.5f, 0.35f, 1.0f);
    DenseForest.FogDensity = 0.05f;
    DenseForest.AmbientTemperature = 24.0f;
    DenseForest.bHasWater = false;
    BiomeZones.Add(DenseForest);

    // Open Savanna — south
    FWorld_BiomeZone OpenSavanna;
    OpenSavanna.BiomeType = EWorld_BiomeType::OpenSavanna;
    OpenSavanna.CenterLocation = FVector(400.0f, -900.0f, -20.0f);
    OpenSavanna.Radius = 1800.0f;
    OpenSavanna.VegetationDensity = 0.6f;
    OpenSavanna.WaterLevel = -150.0f;
    OpenSavanna.FogColor = FLinearColor(0.7f, 0.65f, 0.5f, 1.0f);
    OpenSavanna.FogDensity = 0.008f;
    OpenSavanna.AmbientTemperature = 32.0f;
    OpenSavanna.bHasWater = false;
    BiomeZones.Add(OpenSavanna);

    UE_LOG(LogTemp, Log, TEXT("PCGBiomeSystem: Initialized %d biome zones"), BiomeZones.Num());
}

void APCGBiomeSystem::InitializeBiomeZones()
{
    // Setup ambient sound cue names per biome
    BiomeAmbientSoundCues.Add(EWorld_BiomeType::RiverDelta, FName("SC_Ambient_RiverDelta"));
    BiomeAmbientSoundCues.Add(EWorld_BiomeType::RockyHighlands, FName("SC_Ambient_RockyHighlands"));
    BiomeAmbientSoundCues.Add(EWorld_BiomeType::DenseForest, FName("SC_Ambient_DenseForest"));
    BiomeAmbientSoundCues.Add(EWorld_BiomeType::OpenSavanna, FName("SC_Ambient_OpenSavanna"));
    BiomeAmbientSoundCues.Add(EWorld_BiomeType::VolcanicPlains, FName("SC_Ambient_VolcanicPlains"));
    BiomeAmbientSoundCues.Add(EWorld_BiomeType::CoastalMarsh, FName("SC_Ambient_CoastalMarsh"));

    UE_LOG(LogTemp, Log, TEXT("PCGBiomeSystem: Biome audio cues registered"));
}

EWorld_BiomeType APCGBiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDist = MAX_FLT;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::OpenSavanna;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
        if (Dist < Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.BiomeType;
        }
    }

    return ClosestBiome;
}

FWorld_BiomeZone APCGBiomeSystem::GetBiomeZoneData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            return Zone;
        }
    }
    return FWorld_BiomeZone(); // Default empty zone
}

void APCGBiomeSystem::UpdatePlayerBiome(const FVector& PlayerLocation)
{
    EWorld_BiomeType NewBiome = GetBiomeAtLocation(PlayerLocation);

    if (NewBiome != CurrentPlayerBiome)
    {
        CurrentPlayerBiome = NewBiome;

        // Find zone index
        for (int32 i = 0; i < BiomeZones.Num(); ++i)
        {
            if (BiomeZones[i].BiomeType == NewBiome)
            {
                CurrentBiomeIndex = i;
                ApplyBiomeAtmosphere(BiomeZones[i]);
                break;
            }
        }

        UE_LOG(LogTemp, Log, TEXT("PCGBiomeSystem: Player entered biome %d"), (int32)NewBiome);
    }
}

void APCGBiomeSystem::SetWeatherState(EWorld_WeatherState NewWeather)
{
    if (CurrentWeather == NewWeather) return;

    CurrentWeather = NewWeather;

    // Apply weather effects
    switch (NewWeather)
    {
        case EWorld_WeatherState::Clear:
            RainIntensity = 0.0f;
            break;
        case EWorld_WeatherState::LightRain:
            RainIntensity = 0.3f;
            break;
        case EWorld_WeatherState::HeavyRain:
            RainIntensity = 0.8f;
            break;
        case EWorld_WeatherState::Thunderstorm:
            RainIntensity = 1.0f;
            break;
        case EWorld_WeatherState::Overcast:
            RainIntensity = 0.0f;
            break;
        case EWorld_WeatherState::Fog:
            RainIntensity = 0.0f;
            break;
        default:
            RainIntensity = 0.0f;
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("PCGBiomeSystem: Weather changed to %d, rain=%.2f"), (int32)NewWeather, RainIntensity);
}

void APCGBiomeSystem::TriggerRandomWeatherTransition()
{
    // Weighted random weather selection
    int32 Roll = FMath::RandRange(0, 100);
    EWorld_WeatherState NewWeather;

    if (Roll < 40)       NewWeather = EWorld_WeatherState::Clear;
    else if (Roll < 60)  NewWeather = EWorld_WeatherState::Overcast;
    else if (Roll < 75)  NewWeather = EWorld_WeatherState::LightRain;
    else if (Roll < 88)  NewWeather = EWorld_WeatherState::HeavyRain;
    else if (Roll < 95)  NewWeather = EWorld_WeatherState::Thunderstorm;
    else                 NewWeather = EWorld_WeatherState::Fog;

    SetWeatherState(NewWeather);
}

bool APCGBiomeSystem::IsNightTime() const
{
    // Night is between 0.75 and 0.25 (wrapping around midnight)
    return CurrentTimeOfDay > 0.75f || CurrentTimeOfDay < 0.25f;
}

FLinearColor APCGBiomeSystem::GetSkyColorForTime(float TimeOfDay) const
{
    // Dawn: 0.25, Noon: 0.5, Dusk: 0.75, Midnight: 0.0/1.0
    if (TimeOfDay < 0.25f) // Night to dawn
    {
        float T = TimeOfDay / 0.25f;
        return FLinearColor::LerpUsingHSV(FLinearColor(0.05f, 0.05f, 0.15f), FLinearColor(0.8f, 0.4f, 0.2f), T);
    }
    else if (TimeOfDay < 0.5f) // Dawn to noon
    {
        float T = (TimeOfDay - 0.25f) / 0.25f;
        return FLinearColor::LerpUsingHSV(FLinearColor(0.8f, 0.4f, 0.2f), FLinearColor(0.5f, 0.7f, 1.0f), T);
    }
    else if (TimeOfDay < 0.75f) // Noon to dusk
    {
        float T = (TimeOfDay - 0.5f) / 0.25f;
        return FLinearColor::LerpUsingHSV(FLinearColor(0.5f, 0.7f, 1.0f), FLinearColor(0.9f, 0.3f, 0.1f), T);
    }
    else // Dusk to night
    {
        float T = (TimeOfDay - 0.75f) / 0.25f;
        return FLinearColor::LerpUsingHSV(FLinearColor(0.9f, 0.3f, 0.1f), FLinearColor(0.05f, 0.05f, 0.15f), T);
    }
}

void APCGBiomeSystem::DebugDrawBiomeZones()
{
    if (UWorld* World = GetWorld())
    {
        for (const FWorld_BiomeZone& Zone : BiomeZones)
        {
            FColor ZoneColor;
            switch (Zone.BiomeType)
            {
                case EWorld_BiomeType::RiverDelta:     ZoneColor = FColor::Blue;   break;
                case EWorld_BiomeType::RockyHighlands: ZoneColor = FColor::Gray;   break;
                case EWorld_BiomeType::DenseForest:    ZoneColor = FColor::Green;  break;
                case EWorld_BiomeType::OpenSavanna:    ZoneColor = FColor::Yellow; break;
                case EWorld_BiomeType::VolcanicPlains: ZoneColor = FColor::Red;    break;
                case EWorld_BiomeType::CoastalMarsh:   ZoneColor = FColor::Cyan;   break;
                default:                               ZoneColor = FColor::White;  break;
            }
            DrawDebugCircle(World, Zone.CenterLocation, Zone.Radius, 32, ZoneColor, false, 10.0f, 0, 5.0f);
        }
    }
}

void APCGBiomeSystem::TickDayNightCycle(float DeltaTime)
{
    if (DayDurationSeconds <= 0.0f) return;

    CurrentTimeOfDay += DeltaTime / DayDurationSeconds;
    if (CurrentTimeOfDay >= 1.0f)
    {
        CurrentTimeOfDay -= 1.0f;
    }
}

void APCGBiomeSystem::TickWeatherCycle(float DeltaTime)
{
    WeatherTimer += DeltaTime;
    if (WeatherTimer >= NextWeatherChange)
    {
        WeatherTimer = 0.0f;
        NextWeatherChange = FMath::RandRange(WeatherCycleIntervalMin, WeatherCycleIntervalMax);
        TriggerRandomWeatherTransition();
    }
}

void APCGBiomeSystem::ApplyBiomeAtmosphere(const FWorld_BiomeZone& Zone)
{
    // Atmosphere application is handled via Blueprint or VFX agent
    // Log for integration
    UE_LOG(LogTemp, Log, TEXT("PCGBiomeSystem: Applying atmosphere for biome %d — fog=%.3f, temp=%.1f"),
        (int32)Zone.BiomeType, Zone.FogDensity, Zone.AmbientTemperature);
}
