#include "Eng_BiomeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

AEng_BiomeManager::AEng_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    BiomeTransitionDistance = 1000.0f;
    MaxDinosaursPerBiome = 25;

    InitializeDefaultBiomes();
}

void AEng_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized with %d biome configs"), BiomeConfigs.Num());
    GenerateBiomeLayout();
}

void AEng_BiomeManager::InitializeDefaultBiomes()
{
    BiomeConfigs.Empty();

    // Savanna biome
    FEng_BiomeConfig SavannaConfig;
    SavannaConfig.BiomeType = EEng_BiomeType::Savanna;
    SavannaConfig.Temperature = 28.0f;
    SavannaConfig.Humidity = 0.3f;
    SavannaConfig.VegetationDensity = 0.4f;
    SavannaConfig.DinosaurSpecies.Add(TEXT("TRex"));
    SavannaConfig.DinosaurSpecies.Add(TEXT("Triceratops"));
    BiomeConfigs.Add(SavannaConfig);

    // Forest biome
    FEng_BiomeConfig ForestConfig;
    ForestConfig.BiomeType = EEng_BiomeType::Forest;
    ForestConfig.Temperature = 22.0f;
    ForestConfig.Humidity = 0.8f;
    ForestConfig.VegetationDensity = 0.9f;
    ForestConfig.DinosaurSpecies.Add(TEXT("Velociraptor"));
    ForestConfig.DinosaurSpecies.Add(TEXT("Parasaurolophus"));
    BiomeConfigs.Add(ForestConfig);

    // Swamp biome
    FEng_BiomeConfig SwampConfig;
    SwampConfig.BiomeType = EEng_BiomeType::Swamp;
    SwampConfig.Temperature = 26.0f;
    SwampConfig.Humidity = 0.95f;
    SwampConfig.VegetationDensity = 0.7f;
    SwampConfig.DinosaurSpecies.Add(TEXT("Brachiosaurus"));
    SwampConfig.DinosaurSpecies.Add(TEXT("Ankylosaurus"));
    BiomeConfigs.Add(SwampConfig);
}

EEng_BiomeType AEng_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    // Simple biome determination based on location
    float X = Location.X;
    float Y = Location.Y;

    if (X > 2000.0f && Y > 2000.0f)
        return EEng_BiomeType::Forest;
    else if (X < -2000.0f && Y > 2000.0f)
        return EEng_BiomeType::Swamp;
    else if (X > 2000.0f && Y < -2000.0f)
        return EEng_BiomeType::Desert;
    else if (X < -2000.0f && Y < -2000.0f)
        return EEng_BiomeType::Mountain;
    else if (FMath::Abs(X) < 500.0f || FMath::Abs(Y) < 500.0f)
        return EEng_BiomeType::River;
    else
        return EEng_BiomeType::Savanna;
}

FEng_BiomeConfig AEng_BiomeManager::GetBiomeConfig(EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeType == BiomeType)
        {
            return Config;
        }
    }

    // Return default savanna config if not found
    FEng_BiomeConfig DefaultConfig;
    DefaultConfig.BiomeType = EEng_BiomeType::Savanna;
    return DefaultConfig;
}

void AEng_BiomeManager::GenerateBiomeLayout()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Generating biome layout for Milestone 1"));

    // Generate dinosaurs for each biome
    TArray<EEng_BiomeType> BiomeTypes = {
        EEng_BiomeType::Savanna,
        EEng_BiomeType::Forest,
        EEng_BiomeType::Swamp
    };

    for (EEng_BiomeType BiomeType : BiomeTypes)
    {
        FVector BiomeCenter = GetBiomeCenterLocation(BiomeType);
        SpawnDinosaursForBiome(BiomeType, BiomeCenter);
    }
}

void AEng_BiomeManager::SpawnDinosaursForBiome(EEng_BiomeType BiomeType, const FVector& BiomeCenter)
{
    FEng_BiomeConfig Config = GetBiomeConfig(BiomeType);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Spawning dinosaurs for biome %d at location %s"), 
           (int32)BiomeType, *BiomeCenter.ToString());

    // This will be implemented by the Dinosaur AI agent
    // For now, just log the intent
    for (const FString& Species : Config.DinosaurSpecies)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Should spawn %s in biome %d"), 
               *Species, (int32)BiomeType);
    }
}

FVector AEng_BiomeManager::GetBiomeCenterLocation(EEng_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EEng_BiomeType::Savanna:
            return FVector(0.0f, 0.0f, 0.0f);
        case EEng_BiomeType::Forest:
            return FVector(3000.0f, 3000.0f, 0.0f);
        case EEng_BiomeType::Swamp:
            return FVector(-3000.0f, 3000.0f, 0.0f);
        case EEng_BiomeType::Desert:
            return FVector(3000.0f, -3000.0f, 0.0f);
        case EEng_BiomeType::Mountain:
            return FVector(-3000.0f, -3000.0f, 200.0f);
        case EEng_BiomeType::River:
            return FVector(0.0f, 0.0f, -50.0f);
        default:
            return FVector::ZeroVector;
    }
}