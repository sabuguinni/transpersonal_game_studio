#include "World_BiomePopulator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY_STATIC(LogBiomePopulator, Log, All);

UWorld_BiomePopulator::UWorld_BiomePopulator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize biome population settings
    ForestTreeDensity = 0.8f;
    SwampVegetationDensity = 0.6f;
    DesertRockDensity = 0.3f;
    MountainRockDensity = 0.4f;
    PlainsGrassDensity = 0.5f;
    
    PopulationRadius = 10000.0f;
    MaxActorsPerBiome = 100;
    
    // Set default biome centers (200km2 world)
    BiomeCenters.Add(EWorld_BiomeType::Forest, FVector(-50000, -50000, 500));
    BiomeCenters.Add(EWorld_BiomeType::Swamp, FVector(-50000, 50000, 300));
    BiomeCenters.Add(EWorld_BiomeType::Desert, FVector(50000, -50000, 600));
    BiomeCenters.Add(EWorld_BiomeType::Mountain, FVector(50000, 50000, 1200));
    BiomeCenters.Add(EWorld_BiomeType::Plains, FVector(0, 0, 400));
}

void UWorld_BiomePopulator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBiomePopulator, Log, TEXT("BiomePopulator initialized - ready for population"));
}

void UWorld_BiomePopulator::PopulateAllBiomes()
{
    UE_LOG(LogBiomePopulator, Log, TEXT("Starting population of all biomes"));
    
    for (const auto& BiomePair : BiomeCenters)
    {
        EWorld_BiomeType BiomeType = BiomePair.Key;
        FVector BiomeCenter = BiomePair.Value;
        
        PopulateBiome(BiomeType, BiomeCenter);
    }
    
    UE_LOG(LogBiomePopulator, Log, TEXT("All biomes populated successfully"));
}

void UWorld_BiomePopulator::PopulateBiome(EWorld_BiomeType BiomeType, const FVector& BiomeCenter)
{
    switch (BiomeType)
    {
        case EWorld_BiomeType::Forest:
            PopulateForestBiome(BiomeCenter);
            break;
        case EWorld_BiomeType::Swamp:
            PopulateSwampBiome(BiomeCenter);
            break;
        case EWorld_BiomeType::Desert:
            PopulateDesertBiome(BiomeCenter);
            break;
        case EWorld_BiomeType::Mountain:
            PopulateMountainBiome(BiomeCenter);
            break;
        case EWorld_BiomeType::Plains:
            PopulatePlainsBiome(BiomeCenter);
            break;
        default:
            UE_LOG(LogBiomePopulator, Warning, TEXT("Unknown biome type for population"));
            break;
    }
}

void UWorld_BiomePopulator::PopulateForestBiome(const FVector& BiomeCenter)
{
    int32 TreeCount = FMath::RoundToInt(MaxActorsPerBiome * ForestTreeDensity);
    
    for (int32 i = 0; i < TreeCount; i++)
    {
        FVector TreeLocation = GenerateRandomLocationInRadius(BiomeCenter, PopulationRadius);
        TreeLocation.Z = BiomeCenter.Z; // Keep forest at biome elevation
        
        SpawnVegetationActor(TreeLocation, FString::Printf(TEXT("ForestTree_%d"), i + 1), 
                           FVector(1.0f, 1.0f, 2.0f)); // Tall trees
    }
    
    UE_LOG(LogBiomePopulator, Log, TEXT("Populated forest biome with %d trees"), TreeCount);
}

void UWorld_BiomePopulator::PopulateSwampBiome(const FVector& BiomeCenter)
{
    int32 VegetationCount = FMath::RoundToInt(MaxActorsPerBiome * SwampVegetationDensity);
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector VegLocation = GenerateRandomLocationInRadius(BiomeCenter, PopulationRadius * 0.8f);
        VegLocation.Z = BiomeCenter.Z - 50.0f; // Lower in swamp
        
        SpawnVegetationActor(VegLocation, FString::Printf(TEXT("SwampVeg_%d"), i + 1),
                           FVector(0.8f, 0.8f, 1.5f)); // Medium swamp vegetation
    }
    
    // Create water bodies
    int32 WaterBodyCount = 8;
    for (int32 i = 0; i < WaterBodyCount; i++)
    {
        FVector WaterLocation = GenerateRandomLocationInRadius(BiomeCenter, PopulationRadius * 0.6f);
        WaterLocation.Z = BiomeCenter.Z - 100.0f; // Water level
        
        SpawnWaterBody(WaterLocation, FString::Printf(TEXT("SwampWater_%d"), i + 1));
    }
    
    UE_LOG(LogBiomePopulator, Log, TEXT("Populated swamp biome with %d vegetation and %d water bodies"), 
           VegetationCount, WaterBodyCount);
}

void UWorld_BiomePopulator::PopulateDesertBiome(const FVector& BiomeCenter)
{
    int32 RockCount = FMath::RoundToInt(MaxActorsPerBiome * DesertRockDensity);
    
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector RockLocation = GenerateRandomLocationInRadius(BiomeCenter, PopulationRadius);
        RockLocation.Z = BiomeCenter.Z;
        
        float RockScale = FMath::RandRange(2.0f, 6.0f);
        SpawnRockFormation(RockLocation, FString::Printf(TEXT("DesertRock_%d"), i + 1),
                          FVector(RockScale, RockScale, RockScale));
    }
    
    UE_LOG(LogBiomePopulator, Log, TEXT("Populated desert biome with %d rock formations"), RockCount);
}

void UWorld_BiomePopulator::PopulateMountainBiome(const FVector& BiomeCenter)
{
    int32 PeakCount = FMath::RoundToInt(MaxActorsPerBiome * MountainRockDensity);
    
    for (int32 i = 0; i < PeakCount; i++)
    {
        FVector PeakLocation = GenerateRandomLocationInRadius(BiomeCenter, PopulationRadius);
        PeakLocation.Z = BiomeCenter.Z + FMath::RandRange(0.0f, 500.0f); // Varying heights
        
        float PeakScale = FMath::RandRange(5.0f, 12.0f);
        SpawnRockFormation(PeakLocation, FString::Printf(TEXT("MountainPeak_%d"), i + 1),
                          FVector(PeakScale, PeakScale, PeakScale * 1.5f)); // Tall peaks
    }
    
    UE_LOG(LogBiomePopulator, Log, TEXT("Populated mountain biome with %d peaks"), PeakCount);
}

void UWorld_BiomePopulator::PopulatePlainsBiome(const FVector& BiomeCenter)
{
    int32 GrassCount = FMath::RoundToInt(MaxActorsPerBiome * PlainsGrassDensity);
    
    for (int32 i = 0; i < GrassCount; i++)
    {
        FVector GrassLocation = GenerateRandomLocationInRadius(BiomeCenter, PopulationRadius * 0.7f);
        GrassLocation.Z = BiomeCenter.Z;
        
        float GrassScale = FMath::RandRange(0.5f, 1.5f);
        SpawnVegetationActor(GrassLocation, FString::Printf(TEXT("PlainsGrass_%d"), i + 1),
                           FVector(GrassScale, GrassScale, GrassScale));
    }
    
    UE_LOG(LogBiomePopulator, Log, TEXT("Populated plains biome with %d grass patches"), GrassCount);
}

FVector UWorld_BiomePopulator::GenerateRandomLocationInRadius(const FVector& Center, float Radius)
{
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::RandRange(0.0f, Radius);
    
    FVector Offset;
    Offset.X = Distance * FMath::Cos(Angle);
    Offset.Y = Distance * FMath::Sin(Angle);
    Offset.Z = 0.0f;
    
    return Center + Offset;
}

void UWorld_BiomePopulator::SpawnVegetationActor(const FVector& Location, const FString& ActorName, const FVector& Scale)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn a StaticMeshActor as vegetation placeholder
    AStaticMeshActor* VegActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (VegActor)
    {
        VegActor->SetActorLabel(ActorName);
        VegActor->SetActorScale3D(Scale);
        
        // Random rotation for natural look
        FRotator RandomRotation(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        VegActor->SetActorRotation(RandomRotation);
        
        SpawnedActors.Add(VegActor);
    }
}

void UWorld_BiomePopulator::SpawnRockFormation(const FVector& Location, const FString& ActorName, const FVector& Scale)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn a StaticMeshActor as rock formation
    AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (RockActor)
    {
        RockActor->SetActorLabel(ActorName);
        RockActor->SetActorScale3D(Scale);
        
        // Random rotation and slight tilt for natural rock look
        FRotator RandomRotation(FMath::RandRange(-10.0f, 10.0f), FMath::RandRange(0.0f, 360.0f), FMath::RandRange(-5.0f, 5.0f));
        RockActor->SetActorRotation(RandomRotation);
        
        SpawnedActors.Add(RockActor);
    }
}

void UWorld_BiomePopulator::SpawnWaterBody(const FVector& Location, const FString& ActorName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn a StaticMeshActor as water body (plane)
    AStaticMeshActor* WaterActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (WaterActor)
    {
        WaterActor->SetActorLabel(ActorName);
        
        // Scale to create pond/lake size
        float WaterScale = FMath::RandRange(15.0f, 30.0f);
        WaterActor->SetActorScale3D(FVector(WaterScale, WaterScale, 1.0f));
        
        SpawnedActors.Add(WaterActor);
    }
}

void UWorld_BiomePopulator::ClearAllSpawnedActors()
{
    for (AActor* Actor : SpawnedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    SpawnedActors.Empty();
    UE_LOG(LogBiomePopulator, Log, TEXT("Cleared all spawned biome actors"));
}

int32 UWorld_BiomePopulator::GetSpawnedActorCount() const
{
    return SpawnedActors.Num();
}