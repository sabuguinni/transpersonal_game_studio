#include "World_TerrainSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EditorLevelLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

AWorld_TerrainSystem::AWorld_TerrainSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Default terrain generation parameters
    MaxRockFormations = 20;
    TerrainSeed = 12345.0f;
    NoiseScale = 0.001f;
    HeightMultiplier = 1000.0f;

    // Initialize default biome configurations
    FWorld_BiomeConfiguration ForestBiome;
    ForestBiome.BiomeName = TEXT("Forest");
    ForestBiome.CenterLocation = FVector(2000.0f, 0.0f, 0.0f);
    ForestBiome.BiomeRadius = 3000.0f;
    ForestBiome.BiomeColor = FLinearColor(0.2f, 0.8f, 0.3f, 1.0f);
    ForestBiome.TemperatureRange = 25.0f;
    ForestBiome.HumidityLevel = 0.8f;
    ForestBiome.VegetationTypes.Add(TEXT("Oak"));
    ForestBiome.VegetationTypes.Add(TEXT("Pine"));
    ForestBiome.VegetationTypes.Add(TEXT("Fern"));
    BiomeConfigurations.Add(ForestBiome);

    FWorld_BiomeConfiguration DesertBiome;
    DesertBiome.BiomeName = TEXT("Desert");
    DesertBiome.CenterLocation = FVector(-2000.0f, 0.0f, 0.0f);
    DesertBiome.BiomeRadius = 2500.0f;
    DesertBiome.BiomeColor = FLinearColor(0.9f, 0.7f, 0.3f, 1.0f);
    DesertBiome.TemperatureRange = 45.0f;
    DesertBiome.HumidityLevel = 0.1f;
    DesertBiome.VegetationTypes.Add(TEXT("Cactus"));
    DesertBiome.VegetationTypes.Add(TEXT("SagebrushPrehistoric"));
    BiomeConfigurations.Add(DesertBiome);

    FWorld_BiomeConfiguration MountainBiome;
    MountainBiome.BiomeName = TEXT("Mountain");
    MountainBiome.CenterLocation = FVector(0.0f, 2000.0f, 500.0f);
    MountainBiome.BiomeRadius = 2000.0f;
    MountainBiome.BiomeColor = FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);
    MountainBiome.TemperatureRange = 10.0f;
    MountainBiome.HumidityLevel = 0.4f;
    MountainBiome.VegetationTypes.Add(TEXT("PineAlpine"));
    MountainBiome.VegetationTypes.Add(TEXT("MossRock"));
    BiomeConfigurations.Add(MountainBiome);

    FWorld_BiomeConfiguration SwampBiome;
    SwampBiome.BiomeName = TEXT("Swamp");
    SwampBiome.CenterLocation = FVector(0.0f, -2000.0f, -100.0f);
    SwampBiome.BiomeRadius = 2200.0f;
    SwampBiome.BiomeColor = FLinearColor(0.3f, 0.5f, 0.2f, 1.0f);
    SwampBiome.TemperatureRange = 30.0f;
    SwampBiome.HumidityLevel = 0.95f;
    SwampBiome.VegetationTypes.Add(TEXT("CypressPrehistoric"));
    SwampBiome.VegetationTypes.Add(TEXT("Mangrove"));
    SwampBiome.VegetationTypes.Add(TEXT("WaterLily"));
    BiomeConfigurations.Add(SwampBiome);

    // Initialize default water bodies
    FWorld_WaterBodyData MainRiver;
    MainRiver.WaterBodyName = TEXT("MainRiver");
    MainRiver.Location = FVector(0.0f, 0.0f, -50.0f);
    MainRiver.Scale = FVector(100.0f, 20.0f, 1.0f);
    MainRiver.WaterType = EWaterType::River;
    MainRiver.FlowSpeed = 5.0f;
    MainRiver.bIsNavigable = true;
    WaterBodies.Add(MainRiver);

    FWorld_WaterBodyData ForestLake;
    ForestLake.WaterBodyName = TEXT("ForestLake");
    ForestLake.Location = FVector(1800.0f, -300.0f, -30.0f);
    ForestLake.Scale = FVector(30.0f, 30.0f, 1.0f);
    ForestLake.WaterType = EWaterType::Lake;
    ForestLake.FlowSpeed = 0.0f;
    ForestLake.bIsNavigable = true;
    WaterBodies.Add(ForestLake);

    FWorld_WaterBodyData MountainStream;
    MountainStream.WaterBodyName = TEXT("MountainStream");
    MountainStream.Location = FVector(-200.0f, 1500.0f, 200.0f);
    MountainStream.Scale = FVector(80.0f, 8.0f, 1.0f);
    MountainStream.WaterType = EWaterType::Stream;
    MountainStream.FlowSpeed = 8.0f;
    MountainStream.bIsNavigable = false;
    WaterBodies.Add(MountainStream);
}

void AWorld_TerrainSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (SpawnedTerrainActors.Num() == 0)
    {
        GenerateProceduralTerrain();
    }
}

void AWorld_TerrainSystem::GenerateProceduralTerrain()
{
    ClearGeneratedTerrain();
    
    CreateBiomeZones();
    CreateWaterBodies();
    GenerateRockFormations();
    
    UE_LOG(LogTemp, Warning, TEXT("Procedural terrain generation completed"));
}

void AWorld_TerrainSystem::CreateBiomeZones()
{
    for (const FWorld_BiomeConfiguration& BiomeConfig : BiomeConfigurations)
    {
        // Create terrain mesh for each biome
        SpawnTerrainMeshAtLocation(BiomeConfig.CenterLocation, BiomeConfig.BiomeName);
        
        // Create transition zones between biomes
        for (const FWorld_BiomeConfiguration& OtherBiome : BiomeConfigurations)
        {
            if (BiomeConfig.BiomeName != OtherBiome.BiomeName)
            {
                FVector MidPoint = (BiomeConfig.CenterLocation + OtherBiome.CenterLocation) * 0.5f;
                float Distance = FVector::Dist(BiomeConfig.CenterLocation, OtherBiome.CenterLocation);
                
                if (Distance < (BiomeConfig.BiomeRadius + OtherBiome.BiomeRadius) * 0.8f)
                {
                    // Create transition vegetation
                    for (int32 i = 0; i < 8; i++)
                    {
                        FVector TransitionPoint = MidPoint + FVector(
                            FMath::RandRange(-500.0f, 500.0f),
                            FMath::RandRange(-500.0f, 500.0f),
                            FMath::RandRange(0.0f, 100.0f)
                        );
                        SpawnTerrainMeshAtLocation(TransitionPoint, FString::Printf(TEXT("Transition_%s_%s"), *BiomeConfig.BiomeName, *OtherBiome.BiomeName));
                    }
                }
            }
        }
    }
}

void AWorld_TerrainSystem::CreateWaterBodies()
{
    for (const FWorld_WaterBodyData& WaterData : WaterBodies)
    {
        SpawnWaterMeshAtLocation(WaterData);
    }
}

void AWorld_TerrainSystem::GenerateRockFormations()
{
    for (int32 i = 0; i < MaxRockFormations; i++)
    {
        FVector RockLocation = FVector(
            FMath::RandRange(-3000.0f, 3000.0f),
            FMath::RandRange(-3000.0f, 3000.0f),
            FMath::RandRange(0.0f, 800.0f)
        );
        
        float RockScale = FMath::RandRange(2.0f, 8.0f);
        SpawnRockFormationAtLocation(RockLocation, RockScale);
    }
}

FWorld_BiomeConfiguration AWorld_TerrainSystem::GetBiomeAtLocation(const FVector& Location) const
{
    float MinDistance = MAX_FLT;
    FWorld_BiomeConfiguration ClosestBiome;
    
    for (const FWorld_BiomeConfiguration& BiomeConfig : BiomeConfigurations)
    {
        float Distance = FVector::Dist(Location, BiomeConfig.CenterLocation);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomeConfig;
        }
    }
    
    return ClosestBiome;
}

float AWorld_TerrainSystem::GetTerrainHeightAtLocation(const FVector& Location) const
{
    float NoiseValue = GeneratePerlinNoise(Location.X, Location.Y, NoiseScale);
    FVector BiomeInfluence = CalculateBiomeInfluence(Location);
    
    return NoiseValue * HeightMultiplier * BiomeInfluence.Z;
}

bool AWorld_TerrainSystem::IsLocationInWater(const FVector& Location) const
{
    for (const FWorld_WaterBodyData& WaterData : WaterBodies)
    {
        FVector WaterBounds = WaterData.Scale * 100.0f; // Convert to world units
        FBox WaterBox = FBox(
            WaterData.Location - WaterBounds,
            WaterData.Location + WaterBounds
        );
        
        if (WaterBox.IsInside(Location))
        {
            return true;
        }
    }
    
    return false;
}

void AWorld_TerrainSystem::ClearGeneratedTerrain()
{
    for (AActor* Actor : SpawnedTerrainActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedTerrainActors.Empty();
    
    for (AActor* Actor : SpawnedWaterActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedWaterActors.Empty();
    
    for (AActor* Actor : SpawnedRockFormations)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedRockFormations.Empty();
}

void AWorld_TerrainSystem::RegenerateAllTerrain()
{
    GenerateProceduralTerrain();
}

float AWorld_TerrainSystem::GeneratePerlinNoise(float X, float Y, float Scale) const
{
    return FMath::PerlinNoise2D(FVector2D(X * Scale + TerrainSeed, Y * Scale + TerrainSeed));
}

FVector AWorld_TerrainSystem::CalculateBiomeInfluence(const FVector& Location) const
{
    FVector TotalInfluence = FVector::ZeroVector;
    float TotalWeight = 0.0f;
    
    for (const FWorld_BiomeConfiguration& BiomeConfig : BiomeConfigurations)
    {
        float Distance = FVector::Dist(Location, BiomeConfig.CenterLocation);
        float Weight = FMath::Max(0.0f, 1.0f - (Distance / BiomeConfig.BiomeRadius));
        
        if (Weight > 0.0f)
        {
            TotalInfluence += FVector(BiomeConfig.BiomeColor.R, BiomeConfig.BiomeColor.G, BiomeConfig.BiomeColor.B) * Weight;
            TotalWeight += Weight;
        }
    }
    
    if (TotalWeight > 0.0f)
    {
        TotalInfluence /= TotalWeight;
    }
    
    return TotalInfluence;
}

void AWorld_TerrainSystem::SpawnTerrainMeshAtLocation(const FVector& Location, const FString& BiomeName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    AStaticMeshActor* TerrainActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (TerrainActor)
    {
        TerrainActor->SetActorLabel(FString::Printf(TEXT("Terrain_%s"), *BiomeName));
        SpawnedTerrainActors.Add(TerrainActor);
    }
}

void AWorld_TerrainSystem::SpawnWaterMeshAtLocation(const FWorld_WaterBodyData& WaterData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    AStaticMeshActor* WaterActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), WaterData.Location, FRotator::ZeroRotator);
    if (WaterActor)
    {
        WaterActor->SetActorLabel(FString::Printf(TEXT("Water_%s"), *WaterData.WaterBodyName));
        WaterActor->SetActorScale3D(WaterData.Scale);
        SpawnedWaterActors.Add(WaterActor);
    }
}

void AWorld_TerrainSystem::SpawnRockFormationAtLocation(const FVector& Location, float Scale)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (RockActor)
    {
        RockActor->SetActorLabel(TEXT("RockFormation"));
        RockActor->SetActorScale3D(FVector(Scale));
        SpawnedRockFormations.Add(RockActor);
    }
}