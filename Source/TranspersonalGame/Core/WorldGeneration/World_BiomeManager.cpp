#include "World_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TriggerBox.h"

AWorld_BiomeManager::AWorld_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    EnvironmentUpdateInterval = 1.0f;
    LastEnvironmentUpdate = 0.0f;

    // Setup default biomes
    SetupDefaultBiomes();
}

void AWorld_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomes();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized with %d biomes"), BiomeDataArray.Num());
}

void AWorld_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastEnvironmentUpdate += DeltaTime;
    if (LastEnvironmentUpdate >= EnvironmentUpdateInterval)
    {
        UpdateWeatherEffects(DeltaTime);
        LastEnvironmentUpdate = 0.0f;
    }
}

void AWorld_BiomeManager::InitializeBiomes()
{
    if (BiomeDataArray.Num() == 0)
    {
        SetupDefaultBiomes();
    }

    // Log biome initialization
    for (const FWorld_BiomeData& BiomeData : BiomeDataArray)
    {
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized biome at location %s"), 
               *BiomeData.CenterLocation.ToString());
    }
}

EWorld_BiomeType AWorld_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float MinDistance = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Forest;

    for (const FWorld_BiomeData& BiomeData : BiomeDataArray)
    {
        float Distance = CalculateDistanceToBiome(Location, BiomeData);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomeData.BiomeType;
        }
    }

    return ClosestBiome;
}

FWorld_BiomeData AWorld_BiomeManager::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& BiomeData : BiomeDataArray)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            return BiomeData;
        }
    }

    // Return default if not found
    return FWorld_BiomeData();
}

FWorld_WeatherData AWorld_BiomeManager::GetCurrentWeather(const FVector& Location) const
{
    EWorld_BiomeType CurrentBiome = GetBiomeAtLocation(Location);
    FWorld_BiomeData BiomeData = GetBiomeData(CurrentBiome);

    FWorld_WeatherData Weather;
    Weather.Temperature = BiomeData.TemperatureBase;
    Weather.Humidity = BiomeData.HumidityBase;

    // Adjust weather based on biome type
    switch (CurrentBiome)
    {
        case EWorld_BiomeType::Desert:
            Weather.Temperature += 15.0f;
            Weather.Humidity -= 30.0f;
            Weather.WindSpeed = 5.0f;
            break;
        case EWorld_BiomeType::Swamp:
            Weather.Humidity += 25.0f;
            Weather.FogDensity = 0.3f;
            break;
        case EWorld_BiomeType::Mountain:
            Weather.Temperature -= 10.0f;
            Weather.WindSpeed = 8.0f;
            break;
        case EWorld_BiomeType::Forest:
            Weather.RainIntensity = 0.2f;
            Weather.Humidity += 10.0f;
            break;
        default:
            break;
    }

    return Weather;
}

void AWorld_BiomeManager::UpdateEnvironmentalAudio(const FVector& PlayerLocation)
{
    EWorld_BiomeType CurrentBiome = GetBiomeAtLocation(PlayerLocation);
    FWorld_BiomeData BiomeData = GetBiomeData(CurrentBiome);

    // Log current biome for debugging
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Player in biome type %d at location %s"), 
           (int32)CurrentBiome, *PlayerLocation.ToString());

    // Here we would trigger audio changes based on biome
    // This would integrate with the Audio Agent's systems
}

void AWorld_BiomeManager::UpdateWeatherEffects(float DeltaTime)
{
    // Update global weather effects
    // This would integrate with lighting and VFX systems
    
    // Simple weather progression
    CurrentWeather.WindSpeed = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.1f) * 3.0f + 5.0f;
    CurrentWeather.FogDensity = FMath::Max(0.0f, FMath::Sin(GetWorld()->GetTimeSeconds() * 0.05f) * 0.2f + 0.1f);
}

void AWorld_BiomeManager::SpawnBiomeActors(EWorld_BiomeType BiomeType, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Spawn biome-specific actors based on type
    switch (BiomeType)
    {
        case EWorld_BiomeType::Forest:
            // Spawn trees and forest props
            for (int32 i = 0; i < 3; i++)
            {
                FVector SpawnLocation = Location + FVector(
                    FMath::RandRange(-200.0f, 200.0f),
                    FMath::RandRange(-200.0f, 200.0f),
                    0.0f
                );
                
                AStaticMeshActor* TreeActor = World->SpawnActor<AStaticMeshActor>(
                    AStaticMeshActor::StaticClass(),
                    SpawnLocation,
                    FRotator::ZeroRotator
                );
                
                if (TreeActor)
                {
                    TreeActor->SetActorLabel(FString::Printf(TEXT("ProceduralTree_%d"), i));
                }
            }
            break;
            
        case EWorld_BiomeType::Desert:
            // Spawn cacti and desert props
            for (int32 i = 0; i < 2; i++)
            {
                FVector SpawnLocation = Location + FVector(
                    FMath::RandRange(-300.0f, 300.0f),
                    FMath::RandRange(-300.0f, 300.0f),
                    0.0f
                );
                
                AStaticMeshActor* CactusActor = World->SpawnActor<AStaticMeshActor>(
                    AStaticMeshActor::StaticClass(),
                    SpawnLocation,
                    FRotator::ZeroRotator
                );
                
                if (CactusActor)
                {
                    CactusActor->SetActorLabel(FString::Printf(TEXT("ProceduralCactus_%d"), i));
                }
            }
            break;
            
        default:
            break;
    }
}

void AWorld_BiomeManager::SetupDefaultBiomes()
{
    BiomeDataArray.Empty();

    // Forest biome
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.CenterLocation = FVector(-2000.0f, -2000.0f, 100.0f);
    ForestBiome.Radius = 1500.0f;
    ForestBiome.TemperatureBase = 18.0f;
    ForestBiome.HumidityBase = 70.0f;
    ForestBiome.AmbientSounds.Add(TEXT("Forest_Birds"));
    ForestBiome.AmbientSounds.Add(TEXT("Forest_Wind"));
    ForestBiome.WeatherEffects.Add(TEXT("Rain"));
    BiomeDataArray.Add(ForestBiome);

    // Savanna biome
    FWorld_BiomeData SavannaBiome;
    SavannaBiome.BiomeType = EWorld_BiomeType::Savanna;
    SavannaBiome.CenterLocation = FVector(2000.0f, -2000.0f, 100.0f);
    SavannaBiome.Radius = 1500.0f;
    SavannaBiome.TemperatureBase = 25.0f;
    SavannaBiome.HumidityBase = 40.0f;
    SavannaBiome.AmbientSounds.Add(TEXT("Savanna_Wind"));
    SavannaBiome.AmbientSounds.Add(TEXT("Savanna_Insects"));
    BiomeDataArray.Add(SavannaBiome);

    // Mountain biome
    FWorld_BiomeData MountainBiome;
    MountainBiome.BiomeType = EWorld_BiomeType::Mountain;
    MountainBiome.CenterLocation = FVector(0.0f, 3500.0f, 200.0f);
    MountainBiome.Radius = 1200.0f;
    MountainBiome.TemperatureBase = 10.0f;
    MountainBiome.HumidityBase = 30.0f;
    MountainBiome.AmbientSounds.Add(TEXT("Mountain_Wind"));
    MountainBiome.AmbientSounds.Add(TEXT("Mountain_Echo"));
    MountainBiome.WeatherEffects.Add(TEXT("Snow"));
    BiomeDataArray.Add(MountainBiome);

    // Desert biome
    FWorld_BiomeData DesertBiome;
    DesertBiome.BiomeType = EWorld_BiomeType::Desert;
    DesertBiome.CenterLocation = FVector(2000.0f, 2000.0f, 80.0f);
    DesertBiome.Radius = 1400.0f;
    DesertBiome.TemperatureBase = 35.0f;
    DesertBiome.HumidityBase = 15.0f;
    DesertBiome.AmbientSounds.Add(TEXT("Desert_Wind"));
    DesertBiome.WeatherEffects.Add(TEXT("Sandstorm"));
    BiomeDataArray.Add(DesertBiome);

    // Swamp biome
    FWorld_BiomeData SwampBiome;
    SwampBiome.BiomeType = EWorld_BiomeType::Swamp;
    SwampBiome.CenterLocation = FVector(-2000.0f, 2000.0f, 10.0f);
    SwampBiome.Radius = 1300.0f;
    SwampBiome.TemperatureBase = 22.0f;
    SwampBiome.HumidityBase = 85.0f;
    SwampBiome.AmbientSounds.Add(TEXT("Swamp_Frogs"));
    SwampBiome.AmbientSounds.Add(TEXT("Swamp_Water"));
    SwampBiome.WeatherEffects.Add(TEXT("Fog"));
    BiomeDataArray.Add(SwampBiome);
}

float AWorld_BiomeManager::CalculateDistanceToBiome(const FVector& Location, const FWorld_BiomeData& BiomeData) const
{
    float Distance = FVector::Dist(Location, BiomeData.CenterLocation);
    
    // If within radius, return 0 (inside biome)
    if (Distance <= BiomeData.Radius)
    {
        return 0.0f;
    }
    
    // Return distance beyond radius
    return Distance - BiomeData.Radius;
}

FWorld_WeatherData AWorld_BiomeManager::InterpolateWeather(const FWorld_WeatherData& WeatherA, const FWorld_WeatherData& WeatherB, float Alpha) const
{
    FWorld_WeatherData Result;
    
    Result.Temperature = FMath::Lerp(WeatherA.Temperature, WeatherB.Temperature, Alpha);
    Result.Humidity = FMath::Lerp(WeatherA.Humidity, WeatherB.Humidity, Alpha);
    Result.WindSpeed = FMath::Lerp(WeatherA.WindSpeed, WeatherB.WindSpeed, Alpha);
    Result.RainIntensity = FMath::Lerp(WeatherA.RainIntensity, WeatherB.RainIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(WeatherA.FogDensity, WeatherB.FogDensity, Alpha);
    
    return Result;
}