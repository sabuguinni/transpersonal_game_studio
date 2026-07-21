#include "EngArch_BiomeSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

// UEng_BiomeManager Implementation
UEng_BiomeManager::UEng_BiomeManager()
{
    DefaultBiomeRadius = 15000.0f;
    bBiomesInitialized = false;
}

void UEng_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initializing biome system"));
    InitializeBiomes();
}

void UEng_BiomeManager::Deinitialize()
{
    BiomeDatabase.Empty();
    BiomeTransitions.Empty();
    bBiomesInitialized = false;
    
    Super::Deinitialize();
}

void UEng_BiomeManager::InitializeBiomes()
{
    if (bBiomesInitialized)
    {
        return;
    }

    SetupDefaultBiomes();
    SetupBiomeTransitions();
    
    bBiomesInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Biomes initialized with %d biome types"), BiomeDatabase.Num());
}

void UEng_BiomeManager::SetupDefaultBiomes()
{
    // Savanna Biome (Center of map)
    FEng_BiomeData SavannaData;
    SavannaData.BiomeType = EEng_BiomeType::Savanna;
    SavannaData.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    SavannaData.Radius = DefaultBiomeRadius;
    SavannaData.Temperature = 28.0f;
    SavannaData.Humidity = 0.3f;
    SavannaData.Elevation = 100.0f;
    SavannaData.VegetationAssets.Add(TEXT("/Game/LandscapePackOne/Grass/"));
    SavannaData.VegetationAssets.Add(TEXT("/Game/LandscapePackOne/Trees/Acacia/"));
    SavannaData.DinosaurSpecies.Add(TEXT("TRex"));
    SavannaData.DinosaurSpecies.Add(TEXT("Triceratops"));
    SavannaData.DinosaurSpecies.Add(TEXT("Velociraptor"));
    BiomeDatabase.Add(EEng_BiomeType::Savanna, SavannaData);

    // Forest Biome (Northwest)
    FEng_BiomeData ForestData;
    ForestData.BiomeType = EEng_BiomeType::Forest;
    ForestData.CenterLocation = FVector(-45000.0f, 40000.0f, 200.0f);
    ForestData.Radius = DefaultBiomeRadius;
    ForestData.Temperature = 22.0f;
    ForestData.Humidity = 0.8f;
    ForestData.Elevation = 200.0f;
    ForestData.VegetationAssets.Add(TEXT("/Game/Tropical_Jungle_Pack/"));
    ForestData.VegetationAssets.Add(TEXT("/Game/Landscape_AutoMaterial_5_RainForest/"));
    ForestData.DinosaurSpecies.Add(TEXT("Brachiosaurus"));
    ForestData.DinosaurSpecies.Add(TEXT("Parasaurolophus"));
    ForestData.DinosaurSpecies.Add(TEXT("Protoceratops"));
    BiomeDatabase.Add(EEng_BiomeType::Forest, ForestData);

    // Desert Biome (Southeast)
    FEng_BiomeData DesertData;
    DesertData.BiomeType = EEng_BiomeType::Desert;
    DesertData.CenterLocation = FVector(50000.0f, -40000.0f, 50.0f);
    DesertData.Radius = DefaultBiomeRadius;
    DesertData.Temperature = 35.0f;
    DesertData.Humidity = 0.1f;
    DesertData.Elevation = 50.0f;
    DesertData.VegetationAssets.Add(TEXT("/Game/Desert_Oasis/"));
    DesertData.DinosaurSpecies.Add(TEXT("Ankylosaurus"));
    DesertData.DinosaurSpecies.Add(TEXT("Pachycephalo"));
    BiomeDatabase.Add(EEng_BiomeType::Desert, DesertData);

    // Mountain Biome (North)
    FEng_BiomeData MountainData;
    MountainData.BiomeType = EEng_BiomeType::Mountain;
    MountainData.CenterLocation = FVector(0.0f, 60000.0f, 800.0f);
    MountainData.Radius = DefaultBiomeRadius;
    MountainData.Temperature = 15.0f;
    MountainData.Humidity = 0.6f;
    MountainData.Elevation = 800.0f;
    MountainData.VegetationAssets.Add(TEXT("/Game/ANGRY_MESH/"));
    MountainData.DinosaurSpecies.Add(TEXT("Tsintaosaurus"));
    BiomeDatabase.Add(EEng_BiomeType::Mountain, MountainData);

    // Swamp Biome (Southwest)
    FEng_BiomeData SwampData;
    SwampData.BiomeType = EEng_BiomeType::Swamp;
    SwampData.CenterLocation = FVector(-30000.0f, -30000.0f, 80.0f);
    SwampData.Radius = DefaultBiomeRadius;
    SwampData.Temperature = 26.0f;
    SwampData.Humidity = 0.9f;
    SwampData.Elevation = 80.0f;
    SwampData.VegetationAssets.Add(TEXT("/Game/WaterPlane/"));
    SwampData.VegetationAssets.Add(TEXT("/Game/Landscape_AutoMaterial_5_RainForest/"));
    SwampData.DinosaurSpecies.Add(TEXT("Brachiosaurus"));
    SwampData.DinosaurSpecies.Add(TEXT("Parasaurolophus"));
    BiomeDatabase.Add(EEng_BiomeType::Swamp, SwampData);
}

void UEng_BiomeManager::SetupBiomeTransitions()
{
    // Savanna to Forest
    FEng_BiomeTransition SavannaToForest;
    SavannaToForest.FromBiome = EEng_BiomeType::Savanna;
    SavannaToForest.ToBiome = EEng_BiomeType::Forest;
    SavannaToForest.TransitionDistance = 2000.0f;
    SavannaToForest.BlendFactor = 0.5f;
    BiomeTransitions.Add(SavannaToForest);

    // Savanna to Desert
    FEng_BiomeTransition SavannaToDesert;
    SavannaToDesert.FromBiome = EEng_BiomeType::Savanna;
    SavannaToDesert.ToBiome = EEng_BiomeType::Desert;
    SavannaToDesert.TransitionDistance = 1500.0f;
    SavannaToDesert.BlendFactor = 0.3f;
    BiomeTransitions.Add(SavannaToDesert);

    // Forest to Mountain
    FEng_BiomeTransition ForestToMountain;
    ForestToMountain.FromBiome = EEng_BiomeType::Forest;
    ForestToMountain.ToBiome = EEng_BiomeType::Mountain;
    ForestToMountain.TransitionDistance = 2500.0f;
    ForestToMountain.BlendFactor = 0.7f;
    BiomeTransitions.Add(ForestToMountain);

    // Savanna to Swamp
    FEng_BiomeTransition SavannaToSwamp;
    SavannaToSwamp.FromBiome = EEng_BiomeType::Savanna;
    SavannaToSwamp.ToBiome = EEng_BiomeType::Swamp;
    SavannaToSwamp.TransitionDistance = 1800.0f;
    SavannaToSwamp.BlendFactor = 0.4f;
    BiomeTransitions.Add(SavannaToSwamp);
}

EEng_BiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    if (!bBiomesInitialized)
    {
        return EEng_BiomeType::Savanna;
    }

    float MinDistance = FLT_MAX;
    EEng_BiomeType ClosestBiome = EEng_BiomeType::Savanna;

    for (const auto& BiomePair : BiomeDatabase)
    {
        const FEng_BiomeData& BiomeData = BiomePair.Value;
        float Distance = CalculateDistanceToBiome(Location, BiomeData);
        
        if (Distance < BiomeData.Radius && Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomeData.BiomeType;
        }
    }

    return ClosestBiome;
}

FEng_BiomeData UEng_BiomeManager::GetBiomeData(EEng_BiomeType BiomeType) const
{
    if (const FEng_BiomeData* FoundData = BiomeDatabase.Find(BiomeType))
    {
        return *FoundData;
    }
    
    return FEng_BiomeData();
}

float UEng_BiomeManager::GetTemperatureAtLocation(const FVector& Location) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Add elevation-based temperature variation
    float ElevationModifier = -(Location.Z - BiomeData.Elevation) * 0.01f;
    return BiomeData.Temperature + ElevationModifier;
}

float UEng_BiomeManager::GetHumidityAtLocation(const FVector& Location) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    return BiomeData.Humidity;
}

TArray<FString> UEng_BiomeManager::GetVegetationForBiome(EEng_BiomeType BiomeType) const
{
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    return BiomeData.VegetationAssets;
}

TArray<FString> UEng_BiomeManager::GetDinosaursForBiome(EEng_BiomeType BiomeType) const
{
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    return BiomeData.DinosaurSpecies;
}

bool UEng_BiomeManager::IsLocationInBiome(const FVector& Location, EEng_BiomeType BiomeType) const
{
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    float Distance = CalculateDistanceToBiome(Location, BiomeData);
    return Distance <= BiomeData.Radius;
}

FVector UEng_BiomeManager::GetNearestBiomeCenter(const FVector& Location, EEng_BiomeType BiomeType) const
{
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    return BiomeData.CenterLocation;
}

void UEng_BiomeManager::RegisterBiome(const FEng_BiomeData& BiomeData)
{
    BiomeDatabase.Add(BiomeData.BiomeType, BiomeData);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered biome %d at location %s"), 
           (int32)BiomeData.BiomeType, *BiomeData.CenterLocation.ToString());
}

float UEng_BiomeManager::CalculateDistanceToBiome(const FVector& Location, const FEng_BiomeData& BiomeData) const
{
    return FVector::Dist2D(Location, BiomeData.CenterLocation);
}

EEng_BiomeType UEng_BiomeManager::DetermineBiomeFromEnvironmentalFactors(const FVector& Location) const
{
    // Fallback method using location-based heuristics
    float X = Location.X;
    float Y = Location.Y;
    float Z = Location.Z;

    if (Z > 600.0f)
    {
        return EEng_BiomeType::Mountain;
    }
    else if (X < -20000.0f && Y > 20000.0f)
    {
        return EEng_BiomeType::Forest;
    }
    else if (X > 30000.0f && Y < -20000.0f)
    {
        return EEng_BiomeType::Desert;
    }
    else if (X < -15000.0f && Y < -15000.0f && Z < 150.0f)
    {
        return EEng_BiomeType::Swamp;
    }
    else
    {
        return EEng_BiomeType::Savanna;
    }
}

// AEng_BiomeActor Implementation
AEng_BiomeActor::AEng_BiomeActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create sphere component for biome bounds
    BiomeBounds = CreateDefaultSubobject<USphereComponent>(TEXT("BiomeBounds"));
    RootComponent = BiomeBounds;
    BiomeBounds->SetSphereRadius(15000.0f);
    BiomeBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BiomeBounds->SetCollisionResponseToAllChannels(ECR_Ignore);

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);
    VisualizationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualizationMesh->SetVisibility(false); // Hidden by default, can be enabled for debugging
}

void AEng_BiomeActor::BeginPlay()
{
    Super::BeginPlay();

    // Register this biome with the BiomeManager
    if (UEng_BiomeManager* BiomeManager = GetWorld()->GetSubsystem<UEng_BiomeManager>())
    {
        BiomeManager->RegisterBiome(BiomeData);
        UE_LOG(LogTemp, Log, TEXT("BiomeActor: Registered with BiomeManager at %s"), 
               *GetActorLocation().ToString());
    }
}

void AEng_BiomeActor::SetBiomeData(const FEng_BiomeData& NewBiomeData)
{
    BiomeData = NewBiomeData;
    
    // Update sphere radius
    if (BiomeBounds)
    {
        BiomeBounds->SetSphereRadius(BiomeData.Radius);
    }
    
    // Update actor location
    SetActorLocation(BiomeData.CenterLocation);
}

bool AEng_BiomeActor::IsLocationInBiome(const FVector& Location) const
{
    float Distance = FVector::Dist2D(Location, GetActorLocation());
    return Distance <= BiomeData.Radius;
}