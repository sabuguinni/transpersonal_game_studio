#include "BiomeSystemManager.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UBiomeSystemManager::UBiomeSystemManager()
{
    // Constructor implementation
}

void UBiomeSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Initializing biome system"));
    SetupDefaultBiomes();
}

void UBiomeSystemManager::Deinitialize()
{
    BiomeConfigurations.Empty();
    Super::Deinitialize();
}

void UBiomeSystemManager::InitializeBiomes()
{
    SetupDefaultBiomes();
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: %d biomes configured"), BiomeConfigurations.Num());
}

void UBiomeSystemManager::SetupDefaultBiomes()
{
    BiomeConfigurations.Empty();

    // Savana biome (center of map)
    FEng_BiomeConfiguration SavanaConfig;
    SavanaConfig.BiomeType = EBiomeType::Savana;
    SavanaConfig.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavanaConfig.Radius = 45000.0f;
    SavanaConfig.Temperature = 28.0f;
    SavanaConfig.Humidity = 0.3f;
    SavanaConfig.DangerLevel = 0.4f;
    SavanaConfig.AllowedDinosaurTypes = {TEXT("TRex"), TEXT("Velociraptor"), TEXT("Triceratops")};
    BiomeConfigurations.Add(EBiomeType::Savana, SavanaConfig);

    // Forest biome
    FEng_BiomeConfiguration ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Forest;
    ForestConfig.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestConfig.Radius = 40000.0f;
    ForestConfig.Temperature = 22.0f;
    ForestConfig.Humidity = 0.8f;
    ForestConfig.DangerLevel = 0.6f;
    ForestConfig.AllowedDinosaurTypes = {TEXT("Brachiosaurus"), TEXT("Parasaurolophus"), TEXT("Protoceratops")};
    BiomeConfigurations.Add(EBiomeType::Forest, ForestConfig);

    // Desert biome
    FEng_BiomeConfiguration DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertConfig.Radius = 35000.0f;
    DesertConfig.Temperature = 35.0f;
    DesertConfig.Humidity = 0.1f;
    DesertConfig.DangerLevel = 0.7f;
    DesertConfig.AllowedDinosaurTypes = {TEXT("Ankylosaurus"), TEXT("Pachycephalo")};
    BiomeConfigurations.Add(EBiomeType::Desert, DesertConfig);

    // Swamp biome
    FEng_BiomeConfiguration SwampConfig;
    SwampConfig.BiomeType = EBiomeType::Swamp;
    SwampConfig.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampConfig.Radius = 30000.0f;
    SwampConfig.Temperature = 26.0f;
    SwampConfig.Humidity = 0.9f;
    SwampConfig.DangerLevel = 0.8f;
    SwampConfig.AllowedDinosaurTypes = {TEXT("TRex"), TEXT("Tsintaosaurus")};
    BiomeConfigurations.Add(EBiomeType::Swamp, SwampConfig);

    // Mountain biome
    FEng_BiomeConfiguration MountainConfig;
    MountainConfig.BiomeType = EBiomeType::Mountain;
    MountainConfig.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    MountainConfig.Radius = 25000.0f;
    MountainConfig.Temperature = 15.0f;
    MountainConfig.Humidity = 0.4f;
    MountainConfig.DangerLevel = 0.5f;
    MountainConfig.AllowedDinosaurTypes = {TEXT("Triceratops"), TEXT("Ankylosaurus")};
    BiomeConfigurations.Add(EBiomeType::Mountain, MountainConfig);
}

EBiomeType UBiomeSystemManager::GetBiomeAtLocation(const FVector& Location) const
{
    float MinDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savana;

    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeConfiguration& Config = BiomePair.Value;
        float Distance = FVector::Dist2D(Location, Config.CenterLocation);
        
        if (Distance <= Config.Radius && Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Config.BiomeType;
        }
    }

    return ClosestBiome;
}

FEng_BiomeConfiguration UBiomeSystemManager::GetBiomeConfiguration(EBiomeType BiomeType) const
{
    if (const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType))
    {
        return *Config;
    }
    
    // Return default savana config if not found
    return BiomeConfigurations.FindRef(EBiomeType::Savana);
}

bool UBiomeSystemManager::IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const
{
    if (const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType))
    {
        float Distance = FVector::Dist2D(Location, Config->CenterLocation);
        return Distance <= Config->Radius;
    }
    return false;
}

TArray<FVector> UBiomeSystemManager::GetSpawnPointsInBiome(EBiomeType BiomeType, int32 Count) const
{
    TArray<FVector> SpawnPoints;
    
    if (const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType))
    {
        for (int32 i = 0; i < Count; i++)
        {
            FVector SpawnPoint = GetRandomPointInBiome(*Config);
            SpawnPoints.Add(SpawnPoint);
        }
    }
    
    return SpawnPoints;
}

float UBiomeSystemManager::GetBiomeTemperature(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    const FEng_BiomeConfiguration& Config = GetBiomeConfiguration(BiomeType);
    return Config.Temperature;
}

float UBiomeSystemManager::GetBiomeHumidity(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    const FEng_BiomeConfiguration& Config = GetBiomeConfiguration(BiomeType);
    return Config.Humidity;
}

FVector UBiomeSystemManager::GetRandomPointInBiome(const FEng_BiomeConfiguration& BiomeConfig) const
{
    float RandomAngle = UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f);
    float RandomDistance = UKismetMathLibrary::RandomFloatInRange(0.0f, BiomeConfig.Radius * 0.8f);
    
    float X = BiomeConfig.CenterLocation.X + RandomDistance * FMath::Cos(FMath::DegreesToRadians(RandomAngle));
    float Y = BiomeConfig.CenterLocation.Y + RandomDistance * FMath::Sin(FMath::DegreesToRadians(RandomAngle));
    float Z = BiomeConfig.CenterLocation.Z + 100.0f; // Spawn above ground
    
    return FVector(X, Y, Z);
}