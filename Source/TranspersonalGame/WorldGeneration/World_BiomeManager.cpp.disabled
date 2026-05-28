#include "World_BiomeManager.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

AWorld_BiomeManager::AWorld_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    WorldSizeX = 100000;
    WorldSizeY = 100000;
    BiomeBlendDistance = 5000.0f;
}

void AWorld_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (BiomeRegions.Num() == 0)
    {
        CreateDefaultBiomeLayout();
    }
    
    InitializeBiomes();
}

void AWorld_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorld_BiomeManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initializing %d biome regions"), BiomeRegions.Num());
    
    for (const FWorld_BiomeData& BiomeData : BiomeRegions)
    {
        UE_LOG(LogTemp, Log, TEXT("Biome %s initialized at location %s"), 
            *UEnum::GetValueAsString(BiomeData.BiomeType), 
            *BiomeData.Location.ToString());
    }
}

EWorld_BiomeType AWorld_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    if (BiomeRegions.Num() == 0)
    {
        return EWorld_BiomeType::Grasslands;
    }

    float MinDistance = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Grasslands;

    for (const FWorld_BiomeData& BiomeData : BiomeRegions)
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
    for (const FWorld_BiomeData& BiomeData : BiomeRegions)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            return BiomeData;
        }
    }

    // Return default if not found
    FWorld_BiomeData DefaultBiome;
    DefaultBiome.BiomeType = BiomeType;
    return DefaultBiome;
}

void AWorld_BiomeManager::AddBiomeRegion(const FWorld_BiomeData& BiomeData)
{
    BiomeRegions.Add(BiomeData);
    UE_LOG(LogTemp, Log, TEXT("Added biome region: %s at %s"), 
        *UEnum::GetValueAsString(BiomeData.BiomeType), 
        *BiomeData.Location.ToString());
}

TArray<FWorld_BiomeData> AWorld_BiomeManager::GetNearbyBiomes(const FVector& Location, float Radius) const
{
    TArray<FWorld_BiomeData> NearbyBiomes;

    for (const FWorld_BiomeData& BiomeData : BiomeRegions)
    {
        float Distance = CalculateDistanceToBiome(Location, BiomeData);
        if (Distance <= Radius)
        {
            NearbyBiomes.Add(BiomeData);
        }
    }

    return NearbyBiomes;
}

void AWorld_BiomeManager::GenerateDefaultBiomes()
{
    BiomeRegions.Empty();
    CreateDefaultBiomeLayout();
    InitializeBiomes();
    
    UE_LOG(LogTemp, Warning, TEXT("Generated default biome layout with %d regions"), BiomeRegions.Num());
}

float AWorld_BiomeManager::GetBiomeInfluenceAtLocation(const FVector& Location, EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& BiomeData : BiomeRegions)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            float Distance = CalculateDistanceToBiome(Location, BiomeData);
            float MaxInfluenceDistance = FMath::Max(BiomeData.Size.X, BiomeData.Size.Y) * 0.5f + BiomeBlendDistance;
            
            if (Distance <= MaxInfluenceDistance)
            {
                return FMath::Clamp(1.0f - (Distance / MaxInfluenceDistance), 0.0f, 1.0f);
            }
        }
    }

    return 0.0f;
}

void AWorld_BiomeManager::CreateDefaultBiomeLayout()
{
    // Central Grasslands
    FWorld_BiomeData Grasslands;
    Grasslands.BiomeType = EWorld_BiomeType::Grasslands;
    Grasslands.Location = FVector(0, 0, 0);
    Grasslands.Size = FVector(20000, 20000, 1000);
    Grasslands.Temperature = 22.0f;
    Grasslands.Humidity = 0.6f;
    Grasslands.Elevation = 100.0f;
    BiomeRegions.Add(Grasslands);

    // Eastern Forest
    FWorld_BiomeData Forest;
    Forest.BiomeType = EWorld_BiomeType::Forest;
    Forest.Location = FVector(25000, 0, 0);
    Forest.Size = FVector(15000, 15000, 1200);
    Forest.Temperature = 18.0f;
    Forest.Humidity = 0.8f;
    Forest.Elevation = 200.0f;
    BiomeRegions.Add(Forest);

    // Western Mountains
    FWorld_BiomeData Mountains;
    Mountains.BiomeType = EWorld_BiomeType::Mountains;
    Mountains.Location = FVector(-25000, 0, 0);
    Mountains.Size = FVector(18000, 18000, 3000);
    Mountains.Temperature = 10.0f;
    Mountains.Humidity = 0.4f;
    Mountains.Elevation = 1500.0f;
    BiomeRegions.Add(Mountains);

    // Northern Rivers
    FWorld_BiomeData Rivers;
    Rivers.BiomeType = EWorld_BiomeType::Rivers;
    Rivers.Location = FVector(0, 25000, 0);
    Rivers.Size = FVector(30000, 8000, 500);
    Rivers.Temperature = 20.0f;
    Rivers.Humidity = 0.9f;
    Rivers.Elevation = 50.0f;
    BiomeRegions.Add(Rivers);

    // Southern Wetlands
    FWorld_BiomeData Wetlands;
    Wetlands.BiomeType = EWorld_BiomeType::Wetlands;
    Wetlands.Location = FVector(0, -25000, 0);
    Wetlands.Size = FVector(12000, 12000, 300);
    Wetlands.Temperature = 25.0f;
    Wetlands.Humidity = 0.95f;
    Wetlands.Elevation = 20.0f;
    BiomeRegions.Add(Wetlands);

    // Volcanic region in the far west
    FWorld_BiomeData Volcanic;
    Volcanic.BiomeType = EWorld_BiomeType::Volcanic;
    Volcanic.Location = FVector(-40000, -15000, 0);
    Volcanic.Size = FVector(8000, 8000, 2000);
    Volcanic.Temperature = 35.0f;
    Volcanic.Humidity = 0.2f;
    Volcanic.Elevation = 800.0f;
    BiomeRegions.Add(Volcanic);
}

float AWorld_BiomeManager::CalculateDistanceToBiome(const FVector& Location, const FWorld_BiomeData& BiomeData) const
{
    // Calculate distance to biome boundary (not center)
    FVector BiomeMin = BiomeData.Location - (BiomeData.Size * 0.5f);
    FVector BiomeMax = BiomeData.Location + (BiomeData.Size * 0.5f);
    
    FVector ClosestPoint;
    ClosestPoint.X = FMath::Clamp(Location.X, BiomeMin.X, BiomeMax.X);
    ClosestPoint.Y = FMath::Clamp(Location.Y, BiomeMin.Y, BiomeMax.Y);
    ClosestPoint.Z = FMath::Clamp(Location.Z, BiomeMin.Z, BiomeMax.Z);
    
    return FVector::Dist(Location, ClosestPoint);
}