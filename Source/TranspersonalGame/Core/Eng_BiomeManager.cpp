#include "Eng_BiomeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UEng_BiomeManager::UEng_BiomeManager()
{
    BiomeTransitionDistance = 5000.0f;
    bDebugBiomes = false;
}

void UEng_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initializing biome system"));
    SetupDefaultBiomes();
    
    if (bDebugBiomes)
    {
        ValidateBiomeSetup();
    }
}

void UEng_BiomeManager::Deinitialize()
{
    BiomeDataMap.Empty();
    Super::Deinitialize();
}

void UEng_BiomeManager::SetupDefaultBiomes()
{
    // Clear existing data
    BiomeDataMap.Empty();
    
    // PANTANO (Southwest) - Swamp biome
    FEng_BiomeData SwampBiome;
    SwampBiome.BiomeType = EBiomeType::Pantano;
    SwampBiome.Center = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampBiome.BoundsMin = FVector2D(-77500.0f, -76500.0f);
    SwampBiome.BoundsMax = FVector2D(-25000.0f, -15000.0f);
    SwampBiome.Temperature = 28.0f;
    SwampBiome.Humidity = 0.9f;
    SwampBiome.WindStrength = 0.1f;
    BiomeDataMap.Add(EBiomeType::Pantano, SwampBiome);
    
    // FLORESTA (Northwest) - Forest biome
    FEng_BiomeData ForestBiome;
    ForestBiome.BiomeType = EBiomeType::Floresta;
    ForestBiome.Center = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestBiome.BoundsMin = FVector2D(-77500.0f, 15000.0f);
    ForestBiome.BoundsMax = FVector2D(-15000.0f, 76500.0f);
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 0.8f;
    ForestBiome.WindStrength = 0.2f;
    BiomeDataMap.Add(EBiomeType::Floresta, ForestBiome);
    
    // SAVANA (Center) - Savanna biome
    FEng_BiomeData SavannaBiome;
    SavannaBiome.BiomeType = EBiomeType::Savana;
    SavannaBiome.Center = FVector(0.0f, 0.0f, 0.0f);
    SavannaBiome.BoundsMin = FVector2D(-20000.0f, -20000.0f);
    SavannaBiome.BoundsMax = FVector2D(20000.0f, 20000.0f);
    SavannaBiome.Temperature = 30.0f;
    SavannaBiome.Humidity = 0.4f;
    SavannaBiome.WindStrength = 0.3f;
    BiomeDataMap.Add(EBiomeType::Savana, SavannaBiome);
    
    // DESERTO (East) - Desert biome
    FEng_BiomeData DesertBiome;
    DesertBiome.BiomeType = EBiomeType::Deserto;
    DesertBiome.Center = FVector(55000.0f, 0.0f, 0.0f);
    DesertBiome.BoundsMin = FVector2D(25000.0f, -30000.0f);
    DesertBiome.BoundsMax = FVector2D(79500.0f, 30000.0f);
    DesertBiome.Temperature = 40.0f;
    DesertBiome.Humidity = 0.1f;
    DesertBiome.WindStrength = 0.5f;
    BiomeDataMap.Add(EBiomeType::Deserto, DesertBiome);
    
    // MONTANHA_NEVADA (Northeast) - Snowy Mountain biome
    FEng_BiomeData MountainBiome;
    MountainBiome.BiomeType = EBiomeType::MontanhaNevada;
    MountainBiome.Center = FVector(40000.0f, 50000.0f, 500.0f);
    MountainBiome.BoundsMin = FVector2D(15000.0f, 20000.0f);
    MountainBiome.BoundsMax = FVector2D(79500.0f, 76500.0f);
    MountainBiome.Temperature = -5.0f;
    MountainBiome.Humidity = 0.6f;
    MountainBiome.WindStrength = 0.8f;
    BiomeDataMap.Add(EBiomeType::MontanhaNevada, MountainBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Setup complete - %d biomes configured"), BiomeDataMap.Num());
}

EBiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savana;
    
    for (const auto& BiomePair : BiomeDataMap)
    {
        const FEng_BiomeData& BiomeData = BiomePair.Value;
        float Distance = CalculateDistanceToBiome(WorldLocation, BiomeData);
        
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = BiomeData.BiomeType;
        }
    }
    
    return ClosestBiome;
}

FEng_BiomeData UEng_BiomeManager::GetBiomeData(EBiomeType BiomeType) const
{
    if (const FEng_BiomeData* FoundData = BiomeDataMap.Find(BiomeType))
    {
        return *FoundData;
    }
    
    // Return default savanna if not found
    FEng_BiomeData DefaultData;
    DefaultData.BiomeType = EBiomeType::Savana;
    return DefaultData;
}

FVector UEng_BiomeManager::GetRandomLocationInBiome(EBiomeType BiomeType) const
{
    const FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    float RandomX = FMath::RandRange(BiomeData.BoundsMin.X, BiomeData.BoundsMax.X);
    float RandomY = FMath::RandRange(BiomeData.BoundsMin.Y, BiomeData.BoundsMax.Y);
    float RandomZ = 0.0f; // Ground level, can be adjusted based on terrain
    
    return FVector(RandomX, RandomY, RandomZ);
}

bool UEng_BiomeManager::IsLocationInBiome(const FVector& WorldLocation, EBiomeType BiomeType) const
{
    const FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    return (WorldLocation.X >= BiomeData.BoundsMin.X && WorldLocation.X <= BiomeData.BoundsMax.X &&
            WorldLocation.Y >= BiomeData.BoundsMin.Y && WorldLocation.Y <= BiomeData.BoundsMax.Y);
}

TArray<EBiomeType> UEng_BiomeManager::GetAllBiomeTypes() const
{
    TArray<EBiomeType> BiomeTypes;
    BiomeDataMap.GetKeys(BiomeTypes);
    return BiomeTypes;
}

float UEng_BiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    return BiomeData.Temperature;
}

float UEng_BiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    return BiomeData.Humidity;
}

FVector UEng_BiomeManager::GetBiomeCenter(EBiomeType BiomeType) const
{
    const FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    return BiomeData.Center;
}

void UEng_BiomeManager::InitializeBiomes()
{
    SetupDefaultBiomes();
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Manual biome initialization complete"));
}

void UEng_BiomeManager::ValidateBiomeSetup()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Validating biome setup..."));
    
    for (const auto& BiomePair : BiomeDataMap)
    {
        const FEng_BiomeData& BiomeData = BiomePair.Value;
        
        // Check bounds validity
        if (BiomeData.BoundsMin.X >= BiomeData.BoundsMax.X || BiomeData.BoundsMin.Y >= BiomeData.BoundsMax.Y)
        {
            UE_LOG(LogTemp, Error, TEXT("BiomeManager: Invalid bounds for biome %d"), (int32)BiomeData.BiomeType);
        }
        
        // Check if center is within bounds
        if (BiomeData.Center.X < BiomeData.BoundsMin.X || BiomeData.Center.X > BiomeData.BoundsMax.X ||
            BiomeData.Center.Y < BiomeData.BoundsMin.Y || BiomeData.Center.Y > BiomeData.BoundsMax.Y)
        {
            UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Center outside bounds for biome %d"), (int32)BiomeData.BiomeType);
        }
        
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Biome %d - Center: %s, Temp: %.1f, Humidity: %.2f"), 
               (int32)BiomeData.BiomeType, *BiomeData.Center.ToString(), BiomeData.Temperature, BiomeData.Humidity);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Validation complete"));
}

float UEng_BiomeManager::CalculateDistanceToBiome(const FVector& WorldLocation, const FEng_BiomeData& BiomeData) const
{
    // Check if location is within biome bounds
    if (WorldLocation.X >= BiomeData.BoundsMin.X && WorldLocation.X <= BiomeData.BoundsMax.X &&
        WorldLocation.Y >= BiomeData.BoundsMin.Y && WorldLocation.Y <= BiomeData.BoundsMax.Y)
    {
        return 0.0f; // Inside biome
    }
    
    // Calculate distance to nearest edge of biome bounds
    float DistanceX = 0.0f;
    if (WorldLocation.X < BiomeData.BoundsMin.X)
        DistanceX = BiomeData.BoundsMin.X - WorldLocation.X;
    else if (WorldLocation.X > BiomeData.BoundsMax.X)
        DistanceX = WorldLocation.X - BiomeData.BoundsMax.X;
    
    float DistanceY = 0.0f;
    if (WorldLocation.Y < BiomeData.BoundsMin.Y)
        DistanceY = BiomeData.BoundsMin.Y - WorldLocation.Y;
    else if (WorldLocation.Y > BiomeData.BoundsMax.Y)
        DistanceY = WorldLocation.Y - BiomeData.BoundsMax.Y;
    
    return FMath::Sqrt(DistanceX * DistanceX + DistanceY * DistanceY);
}