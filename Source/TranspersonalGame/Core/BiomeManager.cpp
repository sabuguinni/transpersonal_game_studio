#include "BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

DEFINE_LOG_CATEGORY_STATIC(LogBiomeManager, Log, All);

UBiomeManager::UBiomeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize biome data
    InitializeBiomeData();
}

void UBiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBiomeManager, Log, TEXT("BiomeManager initialized"));
    
    // Setup biome boundaries
    SetupBiomeBoundaries();
}

void UBiomeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update biome effects if needed
    UpdateBiomeEffects(DeltaTime);
}

void UBiomeManager::InitializeBiomeData()
{
    // Swamp biome (Southwest)
    FBiomeData SwampBiome;
    SwampBiome.BiomeType = EBiomeType::Swamp;
    SwampBiome.Center = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampBiome.Radius = 25000.0f;
    SwampBiome.Temperature = 25.0f;
    SwampBiome.Humidity = 90.0f;
    BiomeData.Add(EBiomeType::Swamp, SwampBiome);
    
    // Forest biome (Northwest)
    FBiomeData ForestBiome;
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.Center = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestBiome.Radius = 30000.0f;
    ForestBiome.Temperature = 20.0f;
    ForestBiome.Humidity = 70.0f;
    BiomeData.Add(EBiomeType::Forest, ForestBiome);
    
    // Savanna biome (Center)
    FBiomeData SavannaBiome;
    SavannaBiome.BiomeType = EBiomeType::Savanna;
    SavannaBiome.Center = FVector(0.0f, 0.0f, 0.0f);
    SavannaBiome.Radius = 20000.0f;
    SavannaBiome.Temperature = 30.0f;
    SavannaBiome.Humidity = 40.0f;
    BiomeData.Add(EBiomeType::Savanna, SavannaBiome);
    
    // Desert biome (East)
    FBiomeData DesertBiome;
    DesertBiome.BiomeType = EBiomeType::Desert;
    DesertBiome.Center = FVector(55000.0f, 0.0f, 0.0f);
    DesertBiome.Radius = 25000.0f;
    DesertBiome.Temperature = 40.0f;
    DesertBiome.Humidity = 10.0f;
    BiomeData.Add(EBiomeType::Desert, DesertBiome);
    
    // Mountain biome (Northeast)
    FBiomeData MountainBiome;
    MountainBiome.BiomeType = EBiomeType::Mountain;
    MountainBiome.Center = FVector(40000.0f, 50000.0f, 500.0f);
    MountainBiome.Radius = 30000.0f;
    MountainBiome.Temperature = 5.0f;
    MountainBiome.Humidity = 60.0f;
    BiomeData.Add(EBiomeType::Mountain, MountainBiome);
}

void UBiomeManager::SetupBiomeBoundaries()
{
    UE_LOG(LogBiomeManager, Log, TEXT("Setting up biome boundaries for %d biomes"), BiomeData.Num());
}

void UBiomeManager::UpdateBiomeEffects(float DeltaTime)
{
    // Update environmental effects based on current biome
}

EBiomeType UBiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float MinDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savanna;
    
    for (const auto& BiomePair : BiomeData)
    {
        const FBiomeData& Biome = BiomePair.Value;
        float Distance = FVector::Dist(Location, Biome.Center);
        
        if (Distance < Biome.Radius && Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }
    
    return ClosestBiome;
}

FBiomeData UBiomeManager::GetBiomeData(EBiomeType BiomeType) const
{
    if (const FBiomeData* Found = BiomeData.Find(BiomeType))
    {
        return *Found;
    }
    
    // Return default savanna data if not found
    return BiomeData[EBiomeType::Savanna];
}

FVector UBiomeManager::GetRandomLocationInBiome(EBiomeType BiomeType) const
{
    const FBiomeData& Biome = GetBiomeData(BiomeType);
    
    // Generate random offset within biome radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, Biome.Radius * 0.8f); // Stay within 80% of radius
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomRadius;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomRadius;
    RandomOffset.Z = 0.0f;
    
    return Biome.Center + RandomOffset;
}
