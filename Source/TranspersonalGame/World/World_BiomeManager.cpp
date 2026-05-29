#include "World_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWorld_BiomeManager::AWorld_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Default world settings
    WorldSize = 100000.0f;
    BiomeTransitionDistance = 5000;
    bEnableProceduralGeneration = true;
    bBiomesInitialized = false;
    LastUpdateTime = 0.0f;
}

void AWorld_BiomeManager::BeginPlay()
{
    Super::BeginPlay();

    if (!bBiomesInitialized)
    {
        InitializeDefaultBiomes();
        GenerateWorldBiomes();
        SpawnWaterBodies();
        bBiomesInitialized = true;
    }

    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized with %d biomes and %d water bodies"), 
           BiomeRegions.Num(), WaterBodies.Num());
}

void AWorld_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastUpdateTime += DeltaTime;
    
    // Update environmental effects every 5 seconds
    if (LastUpdateTime >= 5.0f)
    {
        UpdateEnvironmentalEffects();
        LastUpdateTime = 0.0f;
    }
}

void AWorld_BiomeManager::InitializeDefaultBiomes()
{
    BiomeRegions.Empty();

    // Savanna biome (central)
    FWorld_BiomeData SavannaBiome;
    SavannaBiome.BiomeType = EWorld_BiomeType::Savanna;
    SavannaBiome.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    SavannaBiome.Radius = 15000.0f;
    SavannaBiome.Temperature = 28.0f;
    SavannaBiome.Humidity = 40.0f;
    SavannaBiome.Elevation = 100.0f;
    SavannaBiome.bHasWaterBodies = true;
    SavannaBiome.VegetationDensity = 30;
    SavannaBiome.AvailableSpecies.Add("TRex");
    SavannaBiome.AvailableSpecies.Add("Velociraptor");
    SavannaBiome.AvailableSpecies.Add("Triceratops");
    BiomeRegions.Add(SavannaBiome);

    // Forest biome (northwest)
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.CenterLocation = FVector(-45000.0f, 40000.0f, 150.0f);
    ForestBiome.Radius = 20000.0f;
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 80.0f;
    ForestBiome.Elevation = 150.0f;
    ForestBiome.bHasWaterBodies = true;
    ForestBiome.VegetationDensity = 90;
    ForestBiome.AvailableSpecies.Add("Brachiosaurus");
    ForestBiome.AvailableSpecies.Add("Parasaurolophus");
    ForestBiome.AvailableSpecies.Add("Protoceratops");
    BiomeRegions.Add(ForestBiome);

    // Desert biome (southeast)
    FWorld_BiomeData DesertBiome;
    DesertBiome.BiomeType = EWorld_BiomeType::Desert;
    DesertBiome.CenterLocation = FVector(50000.0f, -40000.0f, 50.0f);
    DesertBiome.Radius = 18000.0f;
    DesertBiome.Temperature = 35.0f;
    DesertBiome.Humidity = 15.0f;
    DesertBiome.Elevation = 50.0f;
    DesertBiome.bHasWaterBodies = false;
    DesertBiome.VegetationDensity = 10;
    DesertBiome.AvailableSpecies.Add("Ankylosaurus");
    DesertBiome.AvailableSpecies.Add("Pachycephalo");
    BiomeRegions.Add(DesertBiome);

    // Mountain biome (north)
    FWorld_BiomeData MountainBiome;
    MountainBiome.BiomeType = EWorld_BiomeType::Mountain;
    MountainBiome.CenterLocation = FVector(0.0f, 50000.0f, 300.0f);
    MountainBiome.Radius = 12000.0f;
    MountainBiome.Temperature = 15.0f;
    MountainBiome.Humidity = 60.0f;
    MountainBiome.Elevation = 300.0f;
    MountainBiome.bHasWaterBodies = true;
    MountainBiome.VegetationDensity = 20;
    MountainBiome.AvailableSpecies.Add("Tsintaosaurus");
    BiomeRegions.Add(MountainBiome);

    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized %d default biomes"), BiomeRegions.Num());
}

EWorld_BiomeType AWorld_BiomeManager::GetBiomeAtLocation(const FVector& Location)
{
    float ClosestDistance = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Savanna;

    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        float Distance = FVector::Dist(Location, Biome.CenterLocation);
        
        if (Distance < Biome.Radius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }

    return ClosestBiome;
}

FWorld_BiomeData AWorld_BiomeManager::GetBiomeData(EWorld_BiomeType BiomeType)
{
    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }

    // Return default savanna if not found
    return BiomeRegions.Num() > 0 ? BiomeRegions[0] : FWorld_BiomeData();
}

float AWorld_BiomeManager::GetTemperatureAtLocation(const FVector& Location)
{
    EWorld_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Add some variation based on elevation
    float ElevationModifier = (Location.Z - BiomeData.Elevation) * -0.01f;
    return BiomeData.Temperature + ElevationModifier;
}

float AWorld_BiomeManager::GetHumidityAtLocation(const FVector& Location)
{
    EWorld_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Increase humidity near water bodies
    float WaterModifier = IsLocationNearWater(Location, 2000.0f) ? 15.0f : 0.0f;
    return FMath::Clamp(BiomeData.Humidity + WaterModifier, 0.0f, 100.0f);
}

bool AWorld_BiomeManager::IsLocationNearWater(const FVector& Location, float SearchRadius)
{
    for (const FWorld_WaterBodyData& WaterBody : WaterBodies)
    {
        float Distance = FVector::Dist(Location, WaterBody.Location);
        if (Distance <= SearchRadius)
        {
            return true;
        }
    }
    return false;
}

void AWorld_BiomeManager::GenerateWorldBiomes()
{
    if (!bEnableProceduralGeneration) return;

    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Generating world biomes..."));
    
    // Biomes are already initialized in InitializeDefaultBiomes()
    // This function can be extended for runtime biome generation
    
    ValidateBiomeConfiguration();
}

void AWorld_BiomeManager::SpawnWaterBodies()
{
    WaterBodies.Empty();

    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        if (Biome.bHasWaterBodies)
        {
            // Create river for this biome
            FWorld_WaterBodyData River;
            River.WaterType = EWorld_WaterType::River;
            River.Location = Biome.CenterLocation + FVector(2000.0f, 0.0f, -50.0f);
            River.Scale = FVector(5000.0f, 300.0f, 50.0f);
            River.FlowSpeed = 150.0f;
            River.Depth = 200.0f;
            River.bIsDrinkable = true;
            WaterBodies.Add(River);

            // Create lake for this biome
            FWorld_WaterBodyData Lake;
            Lake.WaterType = EWorld_WaterType::Lake;
            Lake.Location = Biome.CenterLocation + FVector(-3000.0f, 2000.0f, -30.0f);
            Lake.Scale = FVector(2000.0f, 2000.0f, 100.0f);
            Lake.FlowSpeed = 0.0f;
            Lake.Depth = 500.0f;
            Lake.bIsDrinkable = true;
            WaterBodies.Add(Lake);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Created %d water bodies"), WaterBodies.Num());
}

void AWorld_BiomeManager::PopulateBiomeVegetation(EWorld_BiomeType BiomeType)
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Populating vegetation for biome %d with density %d"), 
           (int32)BiomeType, BiomeData.VegetationDensity);
    
    // Vegetation spawning logic would go here
    // This is a framework for the Environment Artist agent to build upon
}

void AWorld_BiomeManager::SpawnBiomeSpecificDinosaurs()
{
    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        for (const FString& Species : Biome.AvailableSpecies)
        {
            UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Spawning %s in %d biome"), 
                   *Species, (int32)Biome.BiomeType);
            
            // Dinosaur spawning logic would interface with AI behavior system
        }
    }
}

void AWorld_BiomeManager::UpdateEnvironmentalEffects()
{
    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        ApplyBiomeWeatherEffects(Biome.BiomeType);
    }
}

void AWorld_BiomeManager::ApplyBiomeWeatherEffects(EWorld_BiomeType BiomeType)
{
    // Weather effects implementation
    // This would control particle systems, lighting, and atmospheric effects
}

void AWorld_BiomeManager::DebugVisualizeBiomes()
{
    if (!GetWorld()) return;

    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        FColor BiomeColor = FColor::Green;
        switch (Biome.BiomeType)
        {
            case EWorld_BiomeType::Savanna: BiomeColor = FColor::Yellow; break;
            case EWorld_BiomeType::Forest: BiomeColor = FColor::Green; break;
            case EWorld_BiomeType::Desert: BiomeColor = FColor::Orange; break;
            case EWorld_BiomeType::Mountain: BiomeColor = FColor::Blue; break;
            default: BiomeColor = FColor::White; break;
        }

        DrawDebugSphere(GetWorld(), Biome.CenterLocation, Biome.Radius, 32, BiomeColor, false, 10.0f, 0, 50.0f);
        DrawDebugString(GetWorld(), Biome.CenterLocation + FVector(0, 0, 500), 
                       FString::Printf(TEXT("Biome: %d"), (int32)Biome.BiomeType), 
                       nullptr, BiomeColor, 10.0f);
    }

    for (const FWorld_WaterBodyData& Water : WaterBodies)
    {
        FColor WaterColor = (Water.WaterType == EWorld_WaterType::River) ? FColor::Cyan : FColor::Blue;
        DrawDebugBox(GetWorld(), Water.Location, Water.Scale, WaterColor, false, 10.0f, 0, 25.0f);
    }
}

void AWorld_BiomeManager::ClearDebugVisualization()
{
    if (GetWorld())
    {
        FlushDebugStrings(GetWorld());
    }
}

float AWorld_BiomeManager::CalculateBiomeInfluence(const FVector& Location, const FWorld_BiomeData& BiomeData)
{
    float Distance = FVector::Dist(Location, BiomeData.CenterLocation);
    float Influence = FMath::Clamp(1.0f - (Distance / BiomeData.Radius), 0.0f, 1.0f);
    return Influence;
}

FVector AWorld_BiomeManager::GetNearestWaterBody(const FVector& Location)
{
    float ClosestDistance = FLT_MAX;
    FVector ClosestWater = FVector::ZeroVector;

    for (const FWorld_WaterBodyData& Water : WaterBodies)
    {
        float Distance = FVector::Dist(Location, Water.Location);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestWater = Water.Location;
        }
    }

    return ClosestWater;
}

void AWorld_BiomeManager::ValidateBiomeConfiguration()
{
    bool bValid = true;
    
    if (BiomeRegions.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager: No biomes configured!"));
        bValid = false;
    }

    for (const FWorld_BiomeData& Biome : BiomeRegions)
    {
        if (Biome.Radius <= 0.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("BiomeManager: Invalid radius for biome %d"), (int32)Biome.BiomeType);
            bValid = false;
        }
    }

    if (bValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Configuration validation passed"));
    }
}