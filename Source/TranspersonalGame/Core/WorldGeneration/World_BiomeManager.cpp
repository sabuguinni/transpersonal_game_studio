#include "World_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

AWorld_BiomeManager::AWorld_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bAutoGenerateBiomes = true;
    MaxBiomeCount = 8;
    BiomeTransitionDistance = 1000.0f;
}

void AWorld_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoGenerateBiomes && BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager initialized with %d biome zones"), BiomeZones.Num());
}

EWorld_BiomeType AWorld_BiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDistance = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Forest;
    
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        float Distance = CalculateDistanceToBiome(WorldLocation, Biome);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }
    
    return ClosestBiome;
}

FWorld_BiomeData AWorld_BiomeManager::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    // Return default if not found
    return FWorld_BiomeData();
}

float AWorld_BiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    
    // Add some variation based on elevation
    float ElevationModifier = -(WorldLocation.Z - BiomeData.Elevation) * 0.01f;
    
    return BiomeData.Temperature + ElevationModifier;
}

float AWorld_BiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    
    return BiomeData.Humidity;
}

TArray<FString> AWorld_BiomeManager::GetVegetationForBiome(EWorld_BiomeType BiomeType) const
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    return BiomeData.VegetationTypes;
}

TArray<FString> AWorld_BiomeManager::GetDinosaursForBiome(EWorld_BiomeType BiomeType) const
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    return BiomeData.DinosaurSpecies;
}

void AWorld_BiomeManager::GenerateDefaultBiomes()
{
    ClearAllBiomes();
    InitializeDefaultBiomes();
    
    UE_LOG(LogTemp, Warning, TEXT("Generated %d default biomes"), BiomeZones.Num());
}

void AWorld_BiomeManager::ClearAllBiomes()
{
    BiomeZones.Empty();
}

void AWorld_BiomeManager::AddBiomeZone(const FWorld_BiomeData& NewBiome)
{
    BiomeZones.Add(NewBiome);
}

bool AWorld_BiomeManager::IsLocationInBiome(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            float Distance = FVector::Dist(WorldLocation, Biome.CenterLocation);
            return Distance <= Biome.Radius;
        }
    }
    
    return false;
}

float AWorld_BiomeManager::GetBiomeInfluenceAtLocation(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            float Distance = CalculateDistanceToBiome(WorldLocation, Biome);
            if (Distance <= Biome.Radius)
            {
                return FMath::Clamp(1.0f - (Distance / Biome.Radius), 0.0f, 1.0f);
            }
        }
    }
    
    return 0.0f;
}

void AWorld_BiomeManager::InitializeDefaultBiomes()
{
    // Forest biome - center of map
    BiomeZones.Add(CreateBiomeData(EWorld_BiomeType::Forest, FVector(0, 0, 150), 4000.0f));
    
    // Desert biome - southeast
    BiomeZones.Add(CreateBiomeData(EWorld_BiomeType::Desert, FVector(6000, -6000, 50), 3500.0f));
    
    // Mountain biome - north
    BiomeZones.Add(CreateBiomeData(EWorld_BiomeType::Mountain, FVector(0, 8000, 800), 3000.0f));
    
    // Swamp biome - southwest
    BiomeZones.Add(CreateBiomeData(EWorld_BiomeType::Swamp, FVector(-6000, -4000, 0), 2500.0f));
    
    // Volcanic biome - northeast
    BiomeZones.Add(CreateBiomeData(EWorld_BiomeType::Volcanic, FVector(8000, 6000, 400), 2000.0f));
    
    // Tundra biome - northwest
    BiomeZones.Add(CreateBiomeData(EWorld_BiomeType::Tundra, FVector(-8000, 8000, 200), 4000.0f));
    
    // Coastal biome - west
    BiomeZones.Add(CreateBiomeData(EWorld_BiomeType::Coastal, FVector(-10000, 0, 50), 3000.0f));
    
    // Plains biome - east
    BiomeZones.Add(CreateBiomeData(EWorld_BiomeType::Plains, FVector(10000, 0, 100), 3500.0f));
}

FWorld_BiomeData AWorld_BiomeManager::CreateBiomeData(EWorld_BiomeType Type, const FVector& Center, float Radius)
{
    FWorld_BiomeData BiomeData;
    BiomeData.BiomeType = Type;
    BiomeData.CenterLocation = Center;
    BiomeData.Radius = Radius;
    BiomeData.Elevation = Center.Z;
    
    switch (Type)
    {
        case EWorld_BiomeType::Forest:
            BiomeData.Temperature = 18.0f;
            BiomeData.Humidity = 0.7f;
            BiomeData.VegetationTypes = {"OakTree", "PineTree", "Fern", "Moss"};
            BiomeData.DinosaurSpecies = {"Triceratops", "Parasaurolophus", "Compsognathus"};
            BiomeData.ResourceDensity = 1.2f;
            break;
            
        case EWorld_BiomeType::Desert:
            BiomeData.Temperature = 35.0f;
            BiomeData.Humidity = 0.1f;
            BiomeData.VegetationTypes = {"Cactus", "SagebrushBush", "DeadTree"};
            BiomeData.DinosaurSpecies = {"Dilophosaurus", "Protoceratops", "Oviraptor"};
            BiomeData.ResourceDensity = 0.3f;
            break;
            
        case EWorld_BiomeType::Mountain:
            BiomeData.Temperature = 5.0f;
            BiomeData.Humidity = 0.4f;
            BiomeData.VegetationTypes = {"AlpineTree", "MountainGrass", "Lichen"};
            BiomeData.DinosaurSpecies = {"Ankylosaurus", "Stegosaurus", "Pteranodon"};
            BiomeData.ResourceDensity = 0.8f;
            break;
            
        case EWorld_BiomeType::Swamp:
            BiomeData.Temperature = 25.0f;
            BiomeData.Humidity = 0.9f;
            BiomeData.VegetationTypes = {"CypressTree", "Cattails", "SwampMoss", "LilyPads"};
            BiomeData.DinosaurSpecies = {"Spinosaurus", "Baryonyx", "Deinonychus"};
            BiomeData.ResourceDensity = 1.1f;
            break;
            
        case EWorld_BiomeType::Volcanic:
            BiomeData.Temperature = 45.0f;
            BiomeData.Humidity = 0.2f;
            BiomeData.VegetationTypes = {"VolcanicRock", "LavaFlow", "SulfurDeposit"};
            BiomeData.DinosaurSpecies = {"Carnotaurus", "Giganotosaurus", "Therizinosaurus"};
            BiomeData.ResourceDensity = 0.5f;
            break;
            
        case EWorld_BiomeType::Tundra:
            BiomeData.Temperature = -10.0f;
            BiomeData.Humidity = 0.3f;
            BiomeData.VegetationTypes = {"IceFormation", "TundraGrass", "FrozenBush"};
            BiomeData.DinosaurSpecies = {"Mammoth", "WoollyRhino", "SabertoothTiger"};
            BiomeData.ResourceDensity = 0.4f;
            break;
            
        case EWorld_BiomeType::Coastal:
            BiomeData.Temperature = 15.0f;
            BiomeData.Humidity = 0.8f;
            BiomeData.VegetationTypes = {"Seaweed", "CoastalGrass", "Driftwood", "Coral"};
            BiomeData.DinosaurSpecies = {"Plesiosaur", "Mosasaurus", "Pteranodon"};
            BiomeData.ResourceDensity = 1.0f;
            break;
            
        case EWorld_BiomeType::Plains:
            BiomeData.Temperature = 22.0f;
            BiomeData.Humidity = 0.5f;
            BiomeData.VegetationTypes = {"GrassPlains", "WildFlowers", "ScatteredTrees"};
            BiomeData.DinosaurSpecies = {"Brachiosaurus", "Gallimimus", "TyrannosaurusRex"};
            BiomeData.ResourceDensity = 0.9f;
            break;
    }
    
    return BiomeData;
}

float AWorld_BiomeManager::CalculateDistanceToBiome(const FVector& Location, const FWorld_BiomeData& Biome) const
{
    return FVector::Dist(Location, Biome.CenterLocation);
}