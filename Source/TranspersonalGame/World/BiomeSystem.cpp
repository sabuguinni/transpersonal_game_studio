// BiomeSystem.cpp — Agent #5 Procedural World Generator
// Implements biome zone management, weather simulation, player biome tracking

#include "BiomeSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ABiomeSystem::ABiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Check biome every second

    // Default weather
    CurrentWeather.RainIntensity = 0.0f;
    CurrentWeather.FogDensity = 0.02f;
    CurrentWeather.WindSpeed = 5.0f;
    CurrentWeather.CloudCoverage = 0.3f;
    CurrentWeather.LightningProbability = 0.0f;
}

void ABiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultBiomes();
}

void ABiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdatePlayerBiome();
    UpdateWeatherSimulation(DeltaTime);
}

void ABiomeSystem::InitializeDefaultBiomes()
{
    // Forest biome — NW quadrant
    FWorld_BiomeProperties Forest;
    Forest.BiomeType = EWorld_BiomeType::Forest;
    Forest.BiomeName = TEXT("Dense Cretaceous Forest");
    Forest.GroundColor = FLinearColor(0.05f, 0.35f, 0.05f, 1.0f);
    Forest.Temperature = 22.0f;
    Forest.Humidity = 0.85f;
    Forest.VegetationDensity = 0.95f;
    Forest.DinosaurSpawnWeight = 1.2f;
    Forest.Radius = 3500.0f;
    RegisterBiomeZone(Forest, FVector(-3000.0f, -3000.0f, 0.0f));

    // Plains biome — NE quadrant
    FWorld_BiomeProperties Plains;
    Plains.BiomeType = EWorld_BiomeType::Plains;
    Plains.BiomeName = TEXT("Open Prehistoric Plains");
    Plains.GroundColor = FLinearColor(0.55f, 0.50f, 0.10f, 1.0f);
    Plains.Temperature = 28.0f;
    Plains.Humidity = 0.35f;
    Plains.VegetationDensity = 0.30f;
    Plains.DinosaurSpawnWeight = 1.5f;
    Plains.Radius = 3500.0f;
    RegisterBiomeZone(Plains, FVector(3000.0f, -3000.0f, 0.0f));

    // Rocky highlands — SE quadrant
    FWorld_BiomeProperties Rocky;
    Rocky.BiomeType = EWorld_BiomeType::RockyHighlands;
    Rocky.BiomeName = TEXT("Rocky Highlands");
    Rocky.GroundColor = FLinearColor(0.40f, 0.30f, 0.20f, 1.0f);
    Rocky.Temperature = 18.0f;
    Rocky.Humidity = 0.20f;
    Rocky.VegetationDensity = 0.10f;
    Rocky.DinosaurSpawnWeight = 0.7f;
    Rocky.Radius = 3500.0f;
    RegisterBiomeZone(Rocky, FVector(3000.0f, 3000.0f, 0.0f));

    // River delta — SW quadrant
    FWorld_BiomeProperties Delta;
    Delta.BiomeType = EWorld_BiomeType::RiverDelta;
    Delta.BiomeName = TEXT("River Delta Wetlands");
    Delta.GroundColor = FLinearColor(0.05f, 0.20f, 0.45f, 1.0f);
    Delta.Temperature = 24.0f;
    Delta.Humidity = 0.95f;
    Delta.VegetationDensity = 0.70f;
    Delta.DinosaurSpawnWeight = 1.8f;
    Delta.Radius = 3500.0f;
    RegisterBiomeZone(Delta, FVector(-3000.0f, 3000.0f, 0.0f));

    UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Initialized %d default biomes"), RegisteredBiomes.Num());
}

void ABiomeSystem::RegisterBiomeZone(const FWorld_BiomeProperties& BiomeProps, const FVector& Center)
{
    RegisteredBiomes.Add(BiomeProps);
    BiomeCenters.Add(Center);
    UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Registered biome '%s' at (%.0f, %.0f, %.0f)"),
           *BiomeProps.BiomeName, Center.X, Center.Y, Center.Z);
}

EWorld_BiomeType ABiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    if (RegisteredBiomes.Num() == 0 || BiomeCenters.Num() == 0)
    {
        return EWorld_BiomeType::Plains;
    }

    float ClosestDist = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Plains;

    for (int32 i = 0; i < BiomeCenters.Num() && i < RegisteredBiomes.Num(); ++i)
    {
        float Dist = FVector::Dist2D(WorldLocation, BiomeCenters[i]);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = RegisteredBiomes[i].BiomeType;
        }
    }

    return ClosestBiome;
}

FWorld_BiomeProperties ABiomeSystem::GetBiomeProperties(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeProperties& Props : RegisteredBiomes)
    {
        if (Props.BiomeType == BiomeType)
        {
            return Props;
        }
    }

    // Return default plains properties if not found
    FWorld_BiomeProperties Default;
    Default.BiomeType = EWorld_BiomeType::Plains;
    Default.BiomeName = TEXT("Default Plains");
    return Default;
}

EWorld_BiomeType ABiomeSystem::GetPlayerCurrentBiome() const
{
    return CurrentPlayerBiome;
}

FWorld_BiomeWeatherState ABiomeSystem::GetCurrentWeather() const
{
    return CurrentWeather;
}

void ABiomeSystem::SetWeatherState(const FWorld_BiomeWeatherState& NewWeather)
{
    CurrentWeather = NewWeather;
    OnWeatherChanged(CurrentWeather);
    UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Weather changed — Rain:%.2f Fog:%.3f Wind:%.1f"),
           CurrentWeather.RainIntensity, CurrentWeather.FogDensity, CurrentWeather.WindSpeed);
}

void ABiomeSystem::TriggerStorm()
{
    FWorld_BiomeWeatherState StormWeather;
    StormWeather.RainIntensity = 0.9f;
    StormWeather.FogDensity = 0.08f;
    StormWeather.WindSpeed = 25.0f;
    StormWeather.CloudCoverage = 0.95f;
    StormWeather.LightningProbability = 0.4f;
    SetWeatherState(StormWeather);
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: STORM TRIGGERED"));
}

void ABiomeSystem::ClearWeather()
{
    FWorld_BiomeWeatherState ClearState;
    ClearState.RainIntensity = 0.0f;
    ClearState.FogDensity = 0.01f;
    ClearState.WindSpeed = 3.0f;
    ClearState.CloudCoverage = 0.1f;
    ClearState.LightningProbability = 0.0f;
    SetWeatherState(ClearState);
    UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Weather cleared"));
}

int32 ABiomeSystem::GetBiomeZoneCount() const
{
    return RegisteredBiomes.Num();
}

void ABiomeSystem::UpdatePlayerBiome()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    EWorld_BiomeType NewBiome = GetBiomeAtLocation(PlayerLocation);

    if (NewBiome != CurrentPlayerBiome)
    {
        PreviousPlayerBiome = CurrentPlayerBiome;
        CurrentPlayerBiome = NewBiome;
        OnPlayerEnteredBiome(CurrentPlayerBiome, PreviousPlayerBiome);

        FWorld_BiomeProperties Props = GetBiomeProperties(CurrentPlayerBiome);
        UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Player entered biome '%s'"), *Props.BiomeName);
    }
}

void ABiomeSystem::UpdateWeatherSimulation(float DeltaTime)
{
    WeatherTimer += DeltaTime;
    if (WeatherTimer < WeatherUpdateInterval) return;
    WeatherTimer = 0.0f;

    // Gradually normalize weather toward biome defaults
    FWorld_BiomeProperties CurrentBiomeProps = GetBiomeProperties(CurrentPlayerBiome);

    // Humidity drives fog and rain tendency
    float TargetFog = CurrentBiomeProps.Humidity * 0.06f;
    CurrentWeather.FogDensity = FMath::Lerp(CurrentWeather.FogDensity, TargetFog, 0.1f);

    // Temperature drives wind speed
    float TargetWind = FMath::Clamp((40.0f - CurrentBiomeProps.Temperature) * 0.5f, 2.0f, 20.0f);
    CurrentWeather.WindSpeed = FMath::Lerp(CurrentWeather.WindSpeed, TargetWind, 0.15f);

    // Rain fades naturally
    if (CurrentWeather.RainIntensity > 0.01f)
    {
        CurrentWeather.RainIntensity = FMath::Lerp(CurrentWeather.RainIntensity, 0.0f, 0.05f);
    }
}
