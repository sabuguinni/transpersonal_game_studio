// BiomeSystem.cpp — Agent #05 Procedural World Generator
#include "BiomeSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// UBiomeDetectorComponent
// ============================================================

UBiomeDetectorComponent::UBiomeDetectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
}

void UBiomeDetectorComponent::BeginPlay()
{
    Super::BeginPlay();
    UpdateBiomeDetection();
}

void UBiomeDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        TimeSinceLastUpdate = 0.0f;
        UpdateBiomeDetection();
    }
}

EWorld_BiomeType UBiomeDetectorComponent::GetCurrentBiome() const
{
    return CurrentBiome;
}

FWorld_BiomeParameters UBiomeDetectorComponent::GetCurrentBiomeParameters() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return FWorld_BiomeParameters();

    UWorld* World = GetWorld();
    if (!World) return FWorld_BiomeParameters();

    // Find BiomeSystemManager in world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ABiomeSystemManager::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        ABiomeSystemManager* Manager = Cast<ABiomeSystemManager>(FoundActors[0]);
        if (Manager)
        {
            return Manager->GetBiomeParameters(CurrentBiome);
        }
    }
    return FWorld_BiomeParameters();
}

void UBiomeDetectorComponent::UpdateBiomeDetection()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FVector Location = Owner->GetActorLocation();

    // Find BiomeSystemManager and query
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ABiomeSystemManager::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        ABiomeSystemManager* Manager = Cast<ABiomeSystemManager>(FoundActors[0]);
        if (Manager)
        {
            CurrentBiome = Manager->ClassifyLocationBiome(Location);
        }
    }
}

// ============================================================
// ABiomeSystemManager
// ============================================================

ABiomeSystemManager::ABiomeSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ABiomeSystemManager::BeginPlay()
{
    Super::BeginPlay();
    if (BiomeTable.Num() == 0)
    {
        InitializeBiomeTable();
    }
}

void ABiomeSystemManager::InitializeBiomeTable()
{
    // Jungle Forest
    FWorld_BiomeParameters Jungle;
    Jungle.BiomeType = EWorld_BiomeType::JungleForest;
    Jungle.Temperature = 32.0f;
    Jungle.Humidity = 0.85f;
    Jungle.VegetationDensity = 0.9f;
    Jungle.DangerLevel = 0.5f;
    Jungle.FogColor = FLinearColor(0.4f, 0.6f, 0.3f, 1.0f);
    Jungle.FogDensity = 0.04f;
    Jungle.AmbientSoundCue = TEXT("JungleAmbient");
    BiomeTable.Add(Jungle);

    // Volcanic Plains
    FWorld_BiomeParameters Volcanic;
    Volcanic.BiomeType = EWorld_BiomeType::VolcanicPlains;
    Volcanic.Temperature = 55.0f;
    Volcanic.Humidity = 0.1f;
    Volcanic.VegetationDensity = 0.1f;
    Volcanic.DangerLevel = 0.8f;
    Volcanic.FogColor = FLinearColor(0.7f, 0.3f, 0.1f, 1.0f);
    Volcanic.FogDensity = 0.06f;
    Volcanic.AmbientSoundCue = TEXT("VolcanicAmbient");
    BiomeTable.Add(Volcanic);

    // River Wetlands
    FWorld_BiomeParameters Wetlands;
    Wetlands.BiomeType = EWorld_BiomeType::RiverWetlands;
    Wetlands.Temperature = 28.0f;
    Wetlands.Humidity = 0.95f;
    Wetlands.VegetationDensity = 0.7f;
    Wetlands.DangerLevel = 0.4f;
    Wetlands.FogColor = FLinearColor(0.3f, 0.5f, 0.6f, 1.0f);
    Wetlands.FogDensity = 0.05f;
    Wetlands.AmbientSoundCue = TEXT("WetlandsAmbient");
    BiomeTable.Add(Wetlands);

    // Rocky Highlands
    FWorld_BiomeParameters Highlands;
    Highlands.BiomeType = EWorld_BiomeType::RockyHighlands;
    Highlands.Temperature = 18.0f;
    Highlands.Humidity = 0.2f;
    Highlands.VegetationDensity = 0.2f;
    Highlands.DangerLevel = 0.6f;
    Highlands.FogColor = FLinearColor(0.6f, 0.55f, 0.5f, 1.0f);
    Highlands.FogDensity = 0.02f;
    Highlands.AmbientSoundCue = TEXT("HighlandsAmbient");
    BiomeTable.Add(Highlands);

    // Open Savanna
    FWorld_BiomeParameters Savanna;
    Savanna.BiomeType = EWorld_BiomeType::OpenSavanna;
    Savanna.Temperature = 38.0f;
    Savanna.Humidity = 0.3f;
    Savanna.VegetationDensity = 0.35f;
    Savanna.DangerLevel = 0.7f;
    Savanna.FogColor = FLinearColor(0.8f, 0.7f, 0.4f, 1.0f);
    Savanna.FogDensity = 0.015f;
    Savanna.AmbientSoundCue = TEXT("SavannaAmbient");
    BiomeTable.Add(Savanna);
}

EWorld_BiomeType ABiomeSystemManager::ClassifyLocationBiome(FVector WorldLocation) const
{
    // Use normalized coordinates to classify biome via noise-based quadrant logic
    float NX = WorldLocation.X / WorldScale;  // -1 to 1
    float NY = WorldLocation.Y / WorldScale;  // -1 to 1
    float NZ = WorldLocation.Z / 1000.0f;    // elevation factor

    // Elevation-based: highlands if high enough
    if (NZ > 0.15f)
    {
        return EWorld_BiomeType::RockyHighlands;
    }

    // Quadrant-based biome assignment (matches world layout)
    // NW: Jungle, NE: Volcanic, SW: Wetlands, SE: Highlands, Center: Savanna
    float DistFromCenter = FMath::Sqrt(NX * NX + NY * NY);
    if (DistFromCenter < 0.2f)
    {
        return EWorld_BiomeType::OpenSavanna;
    }

    if (NX < 0 && NY < 0) return EWorld_BiomeType::JungleForest;
    if (NX > 0 && NY < 0) return EWorld_BiomeType::VolcanicPlains;
    if (NX < 0 && NY > 0) return EWorld_BiomeType::RiverWetlands;
    if (NX > 0 && NY > 0) return EWorld_BiomeType::RockyHighlands;

    return EWorld_BiomeType::OpenSavanna;
}

FWorld_BiomeParameters ABiomeSystemManager::GetBiomeParameters(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeParameters& Params : BiomeTable)
    {
        if (Params.BiomeType == BiomeType)
        {
            return Params;
        }
    }
    return FWorld_BiomeParameters();
}

float ABiomeSystemManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    EWorld_BiomeType Biome = ClassifyLocationBiome(WorldLocation);
    FWorld_BiomeParameters Params = GetBiomeParameters(Biome);
    // Add elevation cooling: -6.5°C per 1000m
    float ElevationCooling = (WorldLocation.Z / 100.0f) * 0.65f;
    return FMath::Max(0.0f, Params.Temperature - ElevationCooling);
}

float ABiomeSystemManager::GetHumidityAtLocation(FVector WorldLocation) const
{
    EWorld_BiomeType Biome = ClassifyLocationBiome(WorldLocation);
    FWorld_BiomeParameters Params = GetBiomeParameters(Biome);
    // Humidity decreases with elevation
    float ElevationFactor = FMath::Clamp(1.0f - (WorldLocation.Z / 5000.0f), 0.0f, 1.0f);
    return Params.Humidity * ElevationFactor;
}

bool ABiomeSystemManager::IsLocationNearWater(FVector WorldLocation, float Radius) const
{
    EWorld_BiomeType Biome = ClassifyLocationBiome(WorldLocation);
    // Wetlands always near water
    if (Biome == EWorld_BiomeType::RiverWetlands) return true;
    // Check if near river channel (X near -500, any Y)
    if (FMath::Abs(WorldLocation.X - (-500.0f)) < Radius) return true;
    return false;
}

float ABiomeSystemManager::SampleNoise(float X, float Y, float Frequency) const
{
    // Simple value noise approximation
    float SX = X * Frequency;
    float SY = Y * Frequency;
    return FMath::PerlinNoise2D(FVector2D(SX, SY));
}
