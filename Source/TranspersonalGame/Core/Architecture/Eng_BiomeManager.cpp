#include "Eng_BiomeManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UEng_BiomeManager::UEng_BiomeManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    BiomeTransitionDistance = 1000.0f;
    BiomeResolution = 512;
}

void UEng_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeBiomeSystem();
}

void UEng_BiomeManager::InitializeBiomeSystem()
{
    SetupDefaultBiomes();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("BiomeManager: System initialized with biome database"));
    }
}

void UEng_BiomeManager::SetupDefaultBiomes()
{
    // Grassland biome
    FEng_BiomeData GrasslandData;
    GrasslandData.BiomeType = EBiomeType::Grassland;
    GrasslandData.Temperature = 22.0f;
    GrasslandData.Humidity = 0.6f;
    GrasslandData.Elevation = 100.0f;
    GrasslandData.AllowedVegetation = {"Grass", "Wildflowers", "SmallTrees"};
    GrasslandData.AllowedDinosaurs = {"Triceratops", "Parasaurolophus", "Edmontosaurus"};
    BiomeDatabase.Add(EBiomeType::Grassland, GrasslandData);

    // Forest biome
    FEng_BiomeData ForestData;
    ForestData.BiomeType = EBiomeType::Forest;
    ForestData.Temperature = 18.0f;
    ForestData.Humidity = 0.8f;
    ForestData.Elevation = 200.0f;
    ForestData.AllowedVegetation = {"LargeTrees", "Ferns", "Moss", "Undergrowth"};
    ForestData.AllowedDinosaurs = {"Velociraptor", "Compsognathus", "Therizinosaurus"};
    BiomeDatabase.Add(EBiomeType::Forest, ForestData);

    // Desert biome
    FEng_BiomeData DesertData;
    DesertData.BiomeType = EBiomeType::Desert;
    DesertData.Temperature = 35.0f;
    DesertData.Humidity = 0.2f;
    DesertData.Elevation = 50.0f;
    DesertData.AllowedVegetation = {"Cacti", "SparseGrass", "DesertShrubs"};
    DesertData.AllowedDinosaurs = {"Carnotaurus", "Dilophosaurus", "Ouranosaurus"};
    BiomeDatabase.Add(EBiomeType::Desert, DesertData);

    // Swamp biome
    FEng_BiomeData SwampData;
    SwampData.BiomeType = EBiomeType::Swamp;
    SwampData.Temperature = 26.0f;
    SwampData.Humidity = 0.9f;
    SwampData.Elevation = -10.0f;
    SwampData.AllowedVegetation = {"SwampTrees", "Reeds", "WaterLilies", "Moss"};
    SwampData.AllowedDinosaurs = {"Spinosaurus", "Baryonyx", "Deinosuchus"};
    BiomeDatabase.Add(EBiomeType::Swamp, SwampData);
}

EBiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& WorldLocation)
{
    // Simple noise-based biome determination
    float NoiseX = WorldLocation.X / 10000.0f;
    float NoiseY = WorldLocation.Y / 10000.0f;
    
    float Temperature = 20.0f + FMath::PerlinNoise2D(FVector2D(NoiseX, NoiseY)) * 15.0f;
    float Humidity = 0.5f + FMath::PerlinNoise2D(FVector2D(NoiseX * 1.5f, NoiseY * 1.5f)) * 0.4f;
    float Elevation = WorldLocation.Z;
    
    return CalculateBiomeFromEnvironment(Temperature, Humidity, Elevation);
}

FEng_BiomeData UEng_BiomeManager::GetBiomeData(EBiomeType BiomeType)
{
    if (BiomeDatabase.Contains(BiomeType))
    {
        return BiomeDatabase[BiomeType];
    }
    
    // Return default grassland if biome not found
    FEng_BiomeData DefaultData;
    DefaultData.BiomeType = EBiomeType::Grassland;
    return DefaultData;
}

TArray<EBiomeType> UEng_BiomeManager::GetAdjacentBiomes(const FVector& WorldLocation, float Radius)
{
    TArray<EBiomeType> AdjacentBiomes;
    
    // Sample biomes in a grid around the location
    int32 SampleCount = 8;
    for (int32 i = 0; i < SampleCount; i++)
    {
        float Angle = (2.0f * PI * i) / SampleCount;
        FVector SampleLocation = WorldLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        
        EBiomeType BiomeAtSample = GetBiomeAtLocation(SampleLocation);
        AdjacentBiomes.AddUnique(BiomeAtSample);
    }
    
    return AdjacentBiomes;
}

float UEng_BiomeManager::GetBiomeInfluence(const FVector& WorldLocation, EBiomeType BiomeType)
{
    EBiomeType LocalBiome = GetBiomeAtLocation(WorldLocation);
    
    if (LocalBiome == BiomeType)
    {
        return 1.0f;
    }
    
    // Calculate influence based on distance to biome boundaries
    TArray<EBiomeType> AdjacentBiomes = GetAdjacentBiomes(WorldLocation, BiomeTransitionDistance);
    
    if (AdjacentBiomes.Contains(BiomeType))
    {
        return 0.3f; // Partial influence in transition zones
    }
    
    return 0.0f; // No influence
}

EBiomeType UEng_BiomeManager::CalculateBiomeFromEnvironment(float Temperature, float Humidity, float Elevation)
{
    // Desert: hot and dry
    if (Temperature > 30.0f && Humidity < 0.3f)
    {
        return EBiomeType::Desert;
    }
    
    // Swamp: warm and very humid, low elevation
    if (Temperature > 24.0f && Humidity > 0.8f && Elevation < 50.0f)
    {
        return EBiomeType::Swamp;
    }
    
    // Forest: moderate temperature, high humidity
    if (Temperature > 15.0f && Temperature < 25.0f && Humidity > 0.7f)
    {
        return EBiomeType::Forest;
    }
    
    // Default to grassland
    return EBiomeType::Grassland;
}