#include "Eng_BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AEng_BiomeManager::AEng_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    BiomeTransitionRadius = 1000.0f;
    MaxBiomeZones = 10;
    
    InitializeBiomeDatabase();
}

void AEng_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager initialized with %d biome types"), BiomeDatabase.Num());
}

void AEng_BiomeManager::InitializeBiomeDatabase()
{
    // Savana biome
    FEng_BiomeData SavanaData;
    SavanaData.BiomeType = EEng_BiomeType::Savana;
    SavanaData.Temperature = 30.0f;
    SavanaData.Humidity = 0.3f;
    SavanaData.Elevation = 100.0f;
    SavanaData.AllowedVegetation.Add("Grass");
    SavanaData.AllowedVegetation.Add("AcaciaTree");
    SavanaData.AllowedDinosaurs.Add("TRex");
    SavanaData.AllowedDinosaurs.Add("Triceratops");
    SavanaData.AllowedDinosaurs.Add("Brachiosaurus");
    BiomeDatabase.Add(EEng_BiomeType::Savana, SavanaData);

    // Forest biome
    FEng_BiomeData ForestData;
    ForestData.BiomeType = EEng_BiomeType::Forest;
    ForestData.Temperature = 20.0f;
    ForestData.Humidity = 0.8f;
    ForestData.Elevation = 150.0f;
    ForestData.AllowedVegetation.Add("Pine");
    ForestData.AllowedVegetation.Add("Oak");
    ForestData.AllowedVegetation.Add("Fern");
    ForestData.AllowedDinosaurs.Add("Velociraptor");
    ForestData.AllowedDinosaurs.Add("Parasaurolophus");
    BiomeDatabase.Add(EEng_BiomeType::Forest, ForestData);

    // Swamp biome
    FEng_BiomeData SwampData;
    SwampData.BiomeType = EEng_BiomeType::Swamp;
    SwampData.Temperature = 25.0f;
    SwampData.Humidity = 0.9f;
    SwampData.Elevation = 50.0f;
    SwampData.AllowedVegetation.Add("Moss");
    SwampData.AllowedVegetation.Add("Cypress");
    SwampData.AllowedDinosaurs.Add("Spinosaurus");
    SwampData.AllowedDinosaurs.Add("Ankylosaurus");
    BiomeDatabase.Add(EEng_BiomeType::Swamp, SwampData);

    // Mountain biome
    FEng_BiomeData MountainData;
    MountainData.BiomeType = EEng_BiomeType::Mountain;
    MountainData.Temperature = 10.0f;
    MountainData.Humidity = 0.4f;
    MountainData.Elevation = 500.0f;
    MountainData.AllowedVegetation.Add("Pine");
    MountainData.AllowedVegetation.Add("Rock");
    MountainData.AllowedDinosaurs.Add("Pteranodon");
    BiomeDatabase.Add(EEng_BiomeType::Mountain, MountainData);
}

EEng_BiomeType AEng_BiomeManager::GetBiomeAtLocation(FVector Location)
{
    // Simple biome determination based on location
    // This would be enhanced with noise functions and terrain analysis
    
    float X = Location.X;
    float Y = Location.Y;
    float Z = Location.Z;
    
    if (Z > 400.0f)
    {
        return EEng_BiomeType::Mountain;
    }
    else if (Z < 75.0f && FMath::Abs(Y) > 1500.0f)
    {
        return EEng_BiomeType::Swamp;
    }
    else if (X < -1000.0f)
    {
        return EEng_BiomeType::Forest;
    }
    else
    {
        return EEng_BiomeType::Savana;
    }
}

FEng_BiomeData AEng_BiomeManager::GetBiomeData(EEng_BiomeType BiomeType)
{
    if (BiomeDatabase.Contains(BiomeType))
    {
        return BiomeDatabase[BiomeType];
    }
    
    return FEng_BiomeData();
}

bool AEng_BiomeManager::CanSpawnDinosaurInBiome(const FString& DinosaurType, EEng_BiomeType BiomeType)
{
    if (BiomeDatabase.Contains(BiomeType))
    {
        const FEng_BiomeData& BiomeData = BiomeDatabase[BiomeType];
        return BiomeData.AllowedDinosaurs.Contains(DinosaurType);
    }
    
    return false;
}

TArray<FVector> AEng_BiomeManager::GetValidSpawnLocationsInBiome(EEng_BiomeType BiomeType, int32 Count)
{
    TArray<FVector> SpawnLocations;
    
    // Generate spawn locations based on biome type
    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLoc;
        
        switch (BiomeType)
        {
            case EEng_BiomeType::Savana:
                SpawnLoc = FVector(
                    FMath::RandRange(500.0f, 3000.0f),
                    FMath::RandRange(-1000.0f, 1000.0f),
                    100.0f
                );
                break;
                
            case EEng_BiomeType::Forest:
                SpawnLoc = FVector(
                    FMath::RandRange(-3000.0f, -500.0f),
                    FMath::RandRange(-1000.0f, 1000.0f),
                    150.0f
                );
                break;
                
            case EEng_BiomeType::Swamp:
                SpawnLoc = FVector(
                    FMath::RandRange(-1000.0f, 1000.0f),
                    FMath::RandRange(1500.0f, 3000.0f),
                    50.0f
                );
                break;
                
            case EEng_BiomeType::Mountain:
                SpawnLoc = FVector(
                    FMath::RandRange(-1000.0f, 1000.0f),
                    FMath::RandRange(-4000.0f, -2000.0f),
                    FMath::RandRange(300.0f, 600.0f)
                );
                break;
                
            default:
                SpawnLoc = FVector::ZeroVector;
                break;
        }
        
        SpawnLocations.Add(SpawnLoc);
    }
    
    return SpawnLocations;
}