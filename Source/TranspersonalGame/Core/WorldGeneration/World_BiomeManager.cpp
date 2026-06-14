#include "World_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UWorld_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Initialized"));
    
    // Initialize default biome configurations
    ActiveBiomes.Empty();
    BiomeActorCounts.Empty();
    
    // Initialize biome actor counters
    BiomeActorCounts.Add(EBiomeType::Forest, 0);
    BiomeActorCounts.Add(EBiomeType::Desert, 0);
    BiomeActorCounts.Add(EBiomeType::Swampland, 0);
    BiomeActorCounts.Add(EBiomeType::Canyon, 0);
}

void UWorld_BiomeManager::Deinitialize()
{
    ActiveBiomes.Empty();
    BiomeActorCounts.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Deinitialized"));
    Super::Deinitialize();
}

void UWorld_BiomeManager::CreateBiome(const FWorld_BiomeData& BiomeData)
{
    // Add to active biomes
    ActiveBiomes.Add(BiomeData);
    
    // Generate terrain and vegetation for this biome
    GenerateTerrainForBiome(BiomeData);
    SpawnVegetationInBiome(BiomeData);
    
    if (BiomeData.WaterLevel > 0.0f)
    {
        CreateWaterBodies(BiomeData);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Created biome of type %d at location %s"), 
           (int32)BiomeData.BiomeType, *BiomeData.CenterLocation.ToString());
}

void UWorld_BiomeManager::GenerateSwamplandBiome(FVector CenterLocation, float Radius)
{
    FWorld_BiomeData SwampData;
    SwampData.BiomeType = EBiomeType::Swampland;
    SwampData.CenterLocation = CenterLocation;
    SwampData.Radius = Radius;
    SwampData.VegetationDensity = 0.7f;
    SwampData.WaterLevel = 50.0f;
    SwampData.RockDensity = 0.1f;
    SwampData.AmbientTint = FLinearColor(0.8f, 1.0f, 0.9f, 1.0f); // Greenish tint
    
    CreateBiome(SwampData);
    
    // Spawn specific swampland features
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 WaterBodyCount = FMath::RandRange(8, 15);
    for (int32 i = 0; i < WaterBodyCount; i++)
    {
        FVector WaterLocation = CenterLocation + FVector(
            FMath::RandRange(-Radius * 0.8f, Radius * 0.8f),
            FMath::RandRange(-Radius * 0.8f, Radius * 0.8f),
            -20.0f
        );
        
        SpawnBiomeActor(WaterLocation, FVector(3, 3, 0.1f), FString::Printf(TEXT("SwampWater_%d"), i));
        BiomeActorCounts[EBiomeType::Swampland]++;
    }
    
    // Spawn cypress-like tall trees
    int32 TreeCount = FMath::RandRange(12, 20);
    for (int32 i = 0; i < TreeCount; i++)
    {
        FVector TreeLocation = CenterLocation + FVector(
            FMath::RandRange(-Radius * 0.9f, Radius * 0.9f),
            FMath::RandRange(-Radius * 0.9f, Radius * 0.9f),
            FMath::RandRange(0, 50)
        );
        
        SpawnBiomeActor(TreeLocation, FVector(2, 2, 4), FString::Printf(TEXT("SwampTree_%d"), i));
        BiomeActorCounts[EBiomeType::Swampland]++;
    }
}

void UWorld_BiomeManager::GenerateCanyonBiome(FVector CenterLocation, float Radius)
{
    FWorld_BiomeData CanyonData;
    CanyonData.BiomeType = EBiomeType::Canyon;
    CanyonData.CenterLocation = CenterLocation;
    CanyonData.Radius = Radius;
    CanyonData.VegetationDensity = 0.2f;
    CanyonData.WaterLevel = 0.0f;
    CanyonData.RockDensity = 0.8f;
    CanyonData.AmbientTint = FLinearColor(1.0f, 0.8f, 0.7f, 1.0f); // Reddish tint
    
    CreateBiome(CanyonData);
    
    // Spawn canyon rock formations
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 RockCount = FMath::RandRange(10, 18);
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector RockLocation = CenterLocation + FVector(
            FMath::RandRange(-Radius * 0.9f, Radius * 0.9f),
            FMath::RandRange(-Radius * 0.9f, Radius * 0.9f),
            FMath::RandRange(100, 400)
        );
        
        FVector RockScale = FVector(
            FMath::RandRange(3.0f, 8.0f),
            FMath::RandRange(2.0f, 5.0f),
            FMath::RandRange(4.0f, 10.0f)
        );
        
        SpawnBiomeActor(RockLocation, RockScale, FString::Printf(TEXT("CanyonRock_%d"), i));
        BiomeActorCounts[EBiomeType::Canyon]++;
    }
}

void UWorld_BiomeManager::GenerateForestBiome(FVector CenterLocation, float Radius)
{
    FWorld_BiomeData ForestData;
    ForestData.BiomeType = EBiomeType::Forest;
    ForestData.CenterLocation = CenterLocation;
    ForestData.Radius = Radius;
    ForestData.VegetationDensity = 0.8f;
    ForestData.WaterLevel = 0.0f;
    ForestData.RockDensity = 0.3f;
    ForestData.AmbientTint = FLinearColor(0.9f, 1.0f, 0.8f, 1.0f); // Green tint
    
    CreateBiome(ForestData);
    
    // Spawn dense forest vegetation
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 TreeCount = FMath::RandRange(25, 40);
    for (int32 i = 0; i < TreeCount; i++)
    {
        FVector TreeLocation = CenterLocation + FVector(
            FMath::RandRange(-Radius * 0.95f, Radius * 0.95f),
            FMath::RandRange(-Radius * 0.95f, Radius * 0.95f),
            FMath::RandRange(-20, 80)
        );
        
        FVector TreeScale = FVector(
            FMath::RandRange(1.5f, 3.0f),
            FMath::RandRange(1.5f, 3.0f),
            FMath::RandRange(3.0f, 6.0f)
        );
        
        SpawnBiomeActor(TreeLocation, TreeScale, FString::Printf(TEXT("ForestTree_%d"), i));
        BiomeActorCounts[EBiomeType::Forest]++;
    }
}

void UWorld_BiomeManager::GenerateDesertBiome(FVector CenterLocation, float Radius)
{
    FWorld_BiomeData DesertData;
    DesertData.BiomeType = EBiomeType::Desert;
    DesertData.CenterLocation = CenterLocation;
    DesertData.Radius = Radius;
    DesertData.VegetationDensity = 0.1f;
    DesertData.WaterLevel = 0.0f;
    DesertData.RockDensity = 0.4f;
    DesertData.AmbientTint = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Sandy tint
    
    CreateBiome(DesertData);
    
    // Spawn sparse desert features
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Sparse vegetation (cacti, dead trees)
    int32 VegCount = FMath::RandRange(5, 12);
    for (int32 i = 0; i < VegCount; i++)
    {
        FVector VegLocation = CenterLocation + FVector(
            FMath::RandRange(-Radius * 0.8f, Radius * 0.8f),
            FMath::RandRange(-Radius * 0.8f, Radius * 0.8f),
            FMath::RandRange(0, 30)
        );
        
        SpawnBiomeActor(VegLocation, FVector(0.8f, 0.8f, 2.0f), FString::Printf(TEXT("DesertPlant_%d"), i));
        BiomeActorCounts[EBiomeType::Desert]++;
    }
    
    // Rock outcroppings
    int32 RockCount = FMath::RandRange(8, 15);
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector RockLocation = CenterLocation + FVector(
            FMath::RandRange(-Radius * 0.9f, Radius * 0.9f),
            FMath::RandRange(-Radius * 0.9f, Radius * 0.9f),
            FMath::RandRange(0, 100)
        );
        
        SpawnBiomeActor(RockLocation, FVector(2, 2, 1.5f), FString::Printf(TEXT("DesertRock_%d"), i));
        BiomeActorCounts[EBiomeType::Desert]++;
    }
}

EBiomeType UWorld_BiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    for (const FWorld_BiomeData& Biome : ActiveBiomes)
    {
        if (IsLocationInBiome(WorldLocation, Biome))
        {
            return Biome.BiomeType;
        }
    }
    
    return EBiomeType::Forest; // Default fallback
}

FWorld_BiomeData UWorld_BiomeManager::GetBiomeData(EBiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : ActiveBiomes)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    // Return default biome data if not found
    FWorld_BiomeData DefaultBiome;
    DefaultBiome.BiomeType = BiomeType;
    return DefaultBiome;
}

void UWorld_BiomeManager::GenerateTerrainForBiome(const FWorld_BiomeData& BiomeData)
{
    // Terrain generation logic would go here
    // For now, we'll just log the terrain generation
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Generated terrain for biome %d"), (int32)BiomeData.BiomeType);
}

void UWorld_BiomeManager::SpawnVegetationInBiome(const FWorld_BiomeData& BiomeData)
{
    // Vegetation spawning logic based on biome density
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Spawned vegetation for biome %d with density %f"), 
           (int32)BiomeData.BiomeType, BiomeData.VegetationDensity);
}

void UWorld_BiomeManager::CreateWaterBodies(const FWorld_BiomeData& BiomeData)
{
    if (BiomeData.WaterLevel <= 0.0f) return;
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Created water bodies for biome %d at level %f"), 
           (int32)BiomeData.BiomeType, BiomeData.WaterLevel);
}

void UWorld_BiomeManager::OptimizeBiomePerformance()
{
    int32 TotalActors = GetTotalBiomeActors();
    
    if (TotalActors > 5000)
    {
        UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Performance optimization needed - %d actors"), TotalActors);
        
        // Implement LOD or culling logic here
        for (auto& BiomeCount : BiomeActorCounts)
        {
            if (BiomeCount.Value > 1000)
            {
                UE_LOG(LogTemp, Warning, TEXT("Biome %d has %d actors - consider optimization"), 
                       (int32)BiomeCount.Key, BiomeCount.Value);
            }
        }
    }
}

int32 UWorld_BiomeManager::GetTotalBiomeActors() const
{
    int32 Total = 0;
    for (const auto& BiomeCount : BiomeActorCounts)
    {
        Total += BiomeCount.Value;
    }
    return Total;
}

void UWorld_BiomeManager::SpawnBiomeActor(FVector Location, FVector Scale, const FString& Label)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // For now, we'll use basic static mesh actors as placeholders
    // In a full implementation, this would spawn appropriate meshes based on biome type
    AActor* SpawnedActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (SpawnedActor)
    {
        SpawnedActor->SetActorLabel(Label);
        SpawnedActor->SetActorScale3D(Scale);
    }
}

float UWorld_BiomeManager::CalculateDistanceFromBiomeCenter(FVector Location, const FWorld_BiomeData& BiomeData) const
{
    return FVector::Dist2D(Location, BiomeData.CenterLocation);
}

bool UWorld_BiomeManager::IsLocationInBiome(FVector Location, const FWorld_BiomeData& BiomeData) const
{
    float Distance = CalculateDistanceFromBiomeCenter(Location, BiomeData);
    return Distance <= BiomeData.Radius;
}