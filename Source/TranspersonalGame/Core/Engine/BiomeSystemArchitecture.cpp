#include "BiomeSystemArchitecture.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UEng_BiomeManager::UEng_BiomeManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetupDefaultBiomes();
}

void UEng_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeBiomes();
}

void UEng_BiomeManager::SetupDefaultBiomes()
{
    BiomeConfigurations.Empty();

    // Savana Biome
    FEng_BiomeConfiguration SavanaBiome;
    SavanaBiome.BiomeType = EBiomeType::Savana;
    SavanaBiome.CenterLocation = FVector(0, 0, 100);
    SavanaBiome.Radius = 50000.0f;
    SavanaBiome.Temperature = 28.0f;
    SavanaBiome.Humidity = 0.3f;
    SavanaBiome.Vegetation = 0.4f;
    SavanaBiome.AllowedDinosaurSpecies = {"Trex", "Velociraptor", "Triceratops"};
    SavanaBiome.MaxDinosaurCount = 30;
    BiomeConfigurations.Add(SavanaBiome);

    // Floresta Biome
    FEng_BiomeConfiguration FlorestaBiome;
    FlorestaBiome.BiomeType = EBiomeType::Floresta;
    FlorestaBiome.CenterLocation = FVector(-45000, 40000, 100);
    FlorestaBiome.Radius = 40000.0f;
    FlorestaBiome.Temperature = 22.0f;
    FlorestaBiome.Humidity = 0.8f;
    FlorestaBiome.Vegetation = 0.9f;
    FlorestaBiome.AllowedDinosaurSpecies = {"Brachiosaurus", "Parasaurolophus", "Protoceratops"};
    FlorestaBiome.MaxDinosaurCount = 25;
    BiomeConfigurations.Add(FlorestaBiome);

    // Deserto Biome
    FEng_BiomeConfiguration DesertoBiome;
    DesertoBiome.BiomeType = EBiomeType::Deserto;
    DesertoBiome.CenterLocation = FVector(55000, 0, 100);
    DesertoBiome.Radius = 35000.0f;
    DesertoBiome.Temperature = 35.0f;
    DesertoBiome.Humidity = 0.1f;
    DesertoBiome.Vegetation = 0.2f;
    DesertoBiome.AllowedDinosaurSpecies = {"Ankylosaurus", "Pachycephalo"};
    DesertoBiome.MaxDinosaurCount = 15;
    BiomeConfigurations.Add(DesertoBiome);

    // Pantano Biome
    FEng_BiomeConfiguration PantanoBiome;
    PantanoBiome.BiomeType = EBiomeType::Pantano;
    PantanoBiome.CenterLocation = FVector(-50000, -45000, 50);
    PantanoBiome.Radius = 30000.0f;
    PantanoBiome.Temperature = 26.0f;
    PantanoBiome.Humidity = 0.95f;
    PantanoBiome.Vegetation = 0.7f;
    PantanoBiome.AllowedDinosaurSpecies = {"Tsintaosaurus", "Parasaurolophus"};
    PantanoBiome.MaxDinosaurCount = 20;
    BiomeConfigurations.Add(PantanoBiome);

    // Montanha Biome
    FEng_BiomeConfiguration MontanhaBiome;
    MontanhaBiome.BiomeType = EBiomeType::Montanha;
    MontanhaBiome.CenterLocation = FVector(40000, 50000, 500);
    MontanhaBiome.Radius = 25000.0f;
    MontanhaBiome.Temperature = 15.0f;
    MontanhaBiome.Humidity = 0.6f;
    MontanhaBiome.Vegetation = 0.3f;
    MontanhaBiome.AllowedDinosaurSpecies = {"Ankylosaurus", "Protoceratops"};
    MontanhaBiome.MaxDinosaurCount = 12;
    BiomeConfigurations.Add(MontanhaBiome);
}

void UEng_BiomeManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initializing %d biomes"), BiomeConfigurations.Num());
}

EBiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    for (const FEng_BiomeConfiguration& BiomeConfig : BiomeConfigurations)
    {
        float Distance = FVector::Dist(Location, BiomeConfig.CenterLocation);
        if (Distance <= BiomeConfig.Radius)
        {
            return BiomeConfig.BiomeType;
        }
    }
    return EBiomeType::Savana; // Default fallback
}

FEng_BiomeConfiguration UEng_BiomeManager::GetBiomeConfiguration(EBiomeType BiomeType) const
{
    for (const FEng_BiomeConfiguration& BiomeConfig : BiomeConfigurations)
    {
        if (BiomeConfig.BiomeType == BiomeType)
        {
            return BiomeConfig;
        }
    }
    return BiomeConfigurations[0]; // Default to first biome
}

bool UEng_BiomeManager::IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const
{
    return GetBiomeAtLocation(Location) == BiomeType;
}

TArray<FVector> UEng_BiomeManager::GetSpawnLocationsInBiome(EBiomeType BiomeType, int32 Count) const
{
    TArray<FVector> SpawnLocations;
    FEng_BiomeConfiguration BiomeConfig = GetBiomeConfiguration(BiomeType);
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeConfig);
        SpawnLocations.Add(SpawnLocation);
    }
    
    return SpawnLocations;
}

FVector UEng_BiomeManager::GetRandomLocationInBiome(const FEng_BiomeConfiguration& BiomeConfig) const
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, BiomeConfig.Radius * 0.8f);
    
    FVector Offset;
    Offset.X = FMath::Cos(RandomAngle) * RandomRadius;
    Offset.Y = FMath::Sin(RandomAngle) * RandomRadius;
    Offset.Z = 0.0f;
    
    return BiomeConfig.CenterLocation + Offset;
}

void UEng_BiomeManager::PopulateBiomeWithActors(EBiomeType BiomeType, const TArray<FString>& AssetPaths)
{
    FEng_BiomeConfiguration BiomeConfig = GetBiomeConfiguration(BiomeType);
    UWorld* World = GetWorld();
    
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager: World is null"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Populating biome %d with %d asset types"), 
           (int32)BiomeType, AssetPaths.Num());
}

// AEng_BiomeActor Implementation
AEng_BiomeActor::AEng_BiomeActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create biome mesh component
    BiomeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BiomeMeshComponent"));
    BiomeMeshComponent->SetupAttachment(RootComponent);

    // Create biome manager component
    BiomeManagerComponent = CreateDefaultSubobject<UEng_BiomeManager>(TEXT("BiomeManagerComponent"));
}

void AEng_BiomeActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (BiomeManagerComponent)
    {
        BiomeManagerComponent->InitializeBiomes();
    }
}

void AEng_BiomeActor::SetBiomeType(EBiomeType NewBiomeType)
{
    AssignedBiomeType = NewBiomeType;
    
    if (BiomeManagerComponent)
    {
        FEng_BiomeConfiguration BiomeConfig = BiomeManagerComponent->GetBiomeConfiguration(NewBiomeType);
        SetActorLocation(BiomeConfig.CenterLocation);
    }
}

void AEng_BiomeActor::SpawnDinosaursInBiome()
{
    if (!BiomeManagerComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeActor: BiomeManagerComponent is null"));
        return;
    }

    FEng_BiomeConfiguration BiomeConfig = BiomeManagerComponent->GetBiomeConfiguration(AssignedBiomeType);
    TArray<FVector> SpawnLocations = BiomeManagerComponent->GetSpawnLocationsInBiome(AssignedBiomeType, 5);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeActor: Spawning dinosaurs at %d locations in biome %d"), 
           SpawnLocations.Num(), (int32)AssignedBiomeType);
}