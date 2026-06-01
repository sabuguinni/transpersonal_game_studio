#include "Eng_BiomeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogBiomeManager, Log, All);

UEng_BiomeManager::UEng_BiomeManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize biome data
    InitializeBiomeData();
}

void UEng_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBiomeManager, Log, TEXT("BiomeManager initialized"));
    
    // Register with world subsystem
    if (UWorld* World = GetWorld())
    {
        World->GetSubsystem<UEng_BiomeManager>();
    }
}

void UEng_BiomeManager::InitializeBiomeData()
{
    // Clear existing data
    BiomeRegions.Empty();
    
    // Define the 5 core biomes with their world coordinates
    FEng_BiomeData SavanaBiome;
    SavanaBiome.BiomeType = EEng_BiomeType::Savana;
    SavanaBiome.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavanaBiome.Radius = 15000.0f;
    SavanaBiome.Temperature = 28.0f;
    SavanaBiome.Humidity = 0.3f;
    SavanaBiome.Vegetation = 0.4f;
    BiomeRegions.Add(SavanaBiome);
    
    FEng_BiomeData PantanoBiome;
    PantanoBiome.BiomeType = EEng_BiomeType::Pantano;
    PantanoBiome.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoBiome.Radius = 15000.0f;
    PantanoBiome.Temperature = 26.0f;
    PantanoBiome.Humidity = 0.9f;
    PantanoBiome.Vegetation = 0.8f;
    BiomeRegions.Add(PantanoBiome);
    
    FEng_BiomeData FlorestaBiome;
    FlorestaBiome.BiomeType = EEng_BiomeType::Floresta;
    FlorestaBiome.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaBiome.Radius = 15000.0f;
    FlorestaBiome.Temperature = 22.0f;
    FlorestaBiome.Humidity = 0.7f;
    FlorestaBiome.Vegetation = 0.9f;
    BiomeRegions.Add(FlorestaBiome);
    
    FEng_BiomeData DesertoBiome;
    DesertoBiome.BiomeType = EEng_BiomeType::Deserto;
    DesertoBiome.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertoBiome.Radius = 15000.0f;
    DesertoBiome.Temperature = 35.0f;
    DesertoBiome.Humidity = 0.1f;
    DesertoBiome.Vegetation = 0.1f;
    BiomeRegions.Add(DesertoBiome);
    
    FEng_BiomeData MontanhaBiome;
    MontanhaBiome.BiomeType = EEng_BiomeType::Montanha;
    MontanhaBiome.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    MontanhaBiome.Radius = 15000.0f;
    MontanhaBiome.Temperature = 15.0f;
    MontanhaBiome.Humidity = 0.5f;
    MontanhaBiome.Vegetation = 0.3f;
    BiomeRegions.Add(MontanhaBiome);
    
    UE_LOG(LogBiomeManager, Log, TEXT("Initialized %d biome regions"), BiomeRegions.Num());
}

EEng_BiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDistance = FLT_MAX;
    EEng_BiomeType ClosestBiome = EEng_BiomeType::Savana;
    
    for (const FEng_BiomeData& Biome : BiomeRegions)
    {
        float Distance = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }
    
    return ClosestBiome;
}

FEng_BiomeData UEng_BiomeManager::GetBiomeData(EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeData& Biome : BiomeRegions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    // Return default savana if not found
    return BiomeRegions.Num() > 0 ? BiomeRegions[0] : FEng_BiomeData();
}

TArray<FVector> UEng_BiomeManager::GetSpawnLocationsForBiome(EEng_BiomeType BiomeType, int32 NumLocations) const
{
    TArray<FVector> SpawnLocations;
    
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    for (int32 i = 0; i < NumLocations; i++)
    {
        // Generate random location within biome radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, BiomeData.Radius * 0.8f); // 80% of radius to avoid edge
        
        FVector SpawnLocation = BiomeData.CenterLocation;
        SpawnLocation.X += FMath::Cos(Angle) * Distance;
        SpawnLocation.Y += FMath::Sin(Angle) * Distance;
        SpawnLocation.Z = 100.0f; // Default height
        
        SpawnLocations.Add(SpawnLocation);
    }
    
    return SpawnLocations;
}

void UEng_BiomeManager::ValidateBiomeDistribution() const
{
    UE_LOG(LogBiomeManager, Log, TEXT("=== BIOME DISTRIBUTION VALIDATION ==="));
    
    for (const FEng_BiomeData& Biome : BiomeRegions)
    {
        FString BiomeName = UEnum::GetValueAsString(Biome.BiomeType);
        UE_LOG(LogBiomeManager, Log, TEXT("Biome: %s, Center: %s, Radius: %.0f"), 
               *BiomeName, 
               *Biome.CenterLocation.ToString(), 
               Biome.Radius);
    }
}

bool UEng_BiomeManager::IsLocationInBiome(const FVector& Location, EEng_BiomeType BiomeType) const
{
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    float Distance = FVector::Dist2D(Location, BiomeData.CenterLocation);
    return Distance <= BiomeData.Radius;
}

float UEng_BiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(Biome);
    return BiomeData.Temperature;
}

float UEng_BiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(Biome);
    return BiomeData.Humidity;
}

FString UEng_BiomeManager::GetBiomeName(EEng_BiomeType BiomeType) const
{
    return UEnum::GetValueAsString(BiomeType);
}