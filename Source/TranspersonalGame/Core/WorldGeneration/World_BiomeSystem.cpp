#include "World_BiomeSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"

UWorld_BiomeSystem::UWorld_BiomeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default weather
    CurrentWeather = FWorld_WeatherData();
    TimeOfDay = 12.0f;
    DayDuration = 1200.0f;
}

void UWorld_BiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default biomes if none exist
    if (BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Initialized with %d biomes"), BiomeZones.Num());
}

void UWorld_BiomeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateDayNightCycle(DeltaTime);
    UpdateWeatherSystem(DeltaTime);
}

EWorld_BiomeType UWorld_BiomeSystem::GetBiomeAtLocation(const FVector& Location) const
{
    float MaxInfluence = 0.0f;
    EWorld_BiomeType DominantBiome = EWorld_BiomeType::Savana;
    
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        float Influence = CalculateBiomeInfluence(Location, Biome);
        if (Influence > MaxInfluence)
        {
            MaxInfluence = Influence;
            DominantBiome = Biome.BiomeType;
        }
    }
    
    return DominantBiome;
}

FWorld_BiomeData UWorld_BiomeSystem::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    // Return default savana biome if not found
    FWorld_BiomeData DefaultBiome;
    DefaultBiome.BiomeType = EWorld_BiomeType::Savana;
    return DefaultBiome;
}

void UWorld_BiomeSystem::SetWeather(const FWorld_WeatherData& NewWeather)
{
    CurrentWeather = NewWeather;
    UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Weather updated - Rain: %.2f, Wind: %.2f"), 
           NewWeather.RainIntensity, NewWeather.WindStrength);
}

void UWorld_BiomeSystem::SetTimeOfDay(float NewTime)
{
    TimeOfDay = FMath::Fmod(NewTime, 24.0f);
    if (TimeOfDay < 0.0f)
    {
        TimeOfDay += 24.0f;
    }
}

void UWorld_BiomeSystem::InitializeDefaultBiomes()
{
    BiomeZones.Empty();
    
    // Savana biome (center)
    FWorld_BiomeData SavanaBiome;
    SavanaBiome.BiomeType = EWorld_BiomeType::Savana;
    SavanaBiome.CenterLocation = FVector(0, 0, 100);
    SavanaBiome.Radius = 15000.0f;
    SavanaBiome.Temperature = 28.0f;
    SavanaBiome.Humidity = 0.4f;
    SavanaBiome.VegetationDensity = 0.3f;
    BiomeZones.Add(SavanaBiome);
    
    // Forest biome (northwest)
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.CenterLocation = FVector(-45000, 40000, 150);
    ForestBiome.Radius = 20000.0f;
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 0.8f;
    ForestBiome.VegetationDensity = 0.9f;
    BiomeZones.Add(ForestBiome);
    
    // Desert biome (southeast)
    FWorld_BiomeData DesertBiome;
    DesertBiome.BiomeType = EWorld_BiomeType::Desert;
    DesertBiome.CenterLocation = FVector(50000, -40000, 50);
    DesertBiome.Radius = 18000.0f;
    DesertBiome.Temperature = 35.0f;
    DesertBiome.Humidity = 0.1f;
    DesertBiome.VegetationDensity = 0.1f;
    BiomeZones.Add(DesertBiome);
    
    // Mountain biome (north)
    FWorld_BiomeData MountainBiome;
    MountainBiome.BiomeType = EWorld_BiomeType::Mountain;
    MountainBiome.CenterLocation = FVector(0, 60000, 500);
    MountainBiome.Radius = 12000.0f;
    MountainBiome.Temperature = 15.0f;
    MountainBiome.Humidity = 0.6f;
    MountainBiome.VegetationDensity = 0.4f;
    BiomeZones.Add(MountainBiome);
    
    // Swamp biome (southwest)
    FWorld_BiomeData SwampBiome;
    SwampBiome.BiomeType = EWorld_BiomeType::Swamp;
    SwampBiome.CenterLocation = FVector(-30000, -30000, 80);
    SwampBiome.Radius = 14000.0f;
    SwampBiome.Temperature = 26.0f;
    SwampBiome.Humidity = 0.95f;
    SwampBiome.VegetationDensity = 0.7f;
    BiomeZones.Add(SwampBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Initialized %d default biomes"), BiomeZones.Num());
}

void UWorld_BiomeSystem::SpawnBiomeMarkers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        // This would be called from UE5 Python to actually spawn the markers
        UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Should spawn marker for %s at %s"), 
               *UEnum::GetValueAsString(Biome.BiomeType), 
               *Biome.CenterLocation.ToString());
    }
}

void UWorld_BiomeSystem::UpdateDayNightCycle(float DeltaTime)
{
    // Advance time
    TimeOfDay += (DeltaTime / DayDuration) * 24.0f;
    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay -= 24.0f;
    }
    
    // Calculate sun intensity based on time
    float SunIntensity = 1.0f;
    if (TimeOfDay < 6.0f || TimeOfDay > 18.0f)
    {
        // Night time - reduce intensity
        SunIntensity = 0.1f;
    }
    else if (TimeOfDay < 8.0f || TimeOfDay > 16.0f)
    {
        // Dawn/dusk - gradual transition
        float Factor = (TimeOfDay < 8.0f) ? (TimeOfDay - 6.0f) / 2.0f : (18.0f - TimeOfDay) / 2.0f;
        SunIntensity = FMath::Lerp(0.1f, 1.0f, Factor);
    }
    
    // Find and update directional light
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
        {
            ADirectionalLight* Light = *ActorItr;
            if (Light && Light->GetLightComponent())
            {
                Light->GetLightComponent()->SetIntensity(SunIntensity * 3.0f);
                break;
            }
        }
    }
}

void UWorld_BiomeSystem::UpdateWeatherSystem(float DeltaTime)
{
    // Simple weather progression - could be expanded
    static float WeatherTimer = 0.0f;
    WeatherTimer += DeltaTime;
    
    if (WeatherTimer > 300.0f) // Change weather every 5 minutes
    {
        WeatherTimer = 0.0f;
        
        // Random weather changes
        CurrentWeather.RainIntensity = FMath::RandRange(0.0f, 1.0f);
        CurrentWeather.WindStrength = FMath::RandRange(0.0f, 2.0f);
        CurrentWeather.CloudCoverage = FMath::RandRange(0.1f, 0.9f);
        
        UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Weather changed - Rain: %.2f"), CurrentWeather.RainIntensity);
    }
}

float UWorld_BiomeSystem::CalculateBiomeInfluence(const FVector& Location, const FWorld_BiomeData& Biome) const
{
    float Distance = FVector::Dist(Location, Biome.CenterLocation);
    if (Distance > Biome.Radius)
    {
        return 0.0f;
    }
    
    // Linear falloff from center to edge
    return 1.0f - (Distance / Biome.Radius);
}