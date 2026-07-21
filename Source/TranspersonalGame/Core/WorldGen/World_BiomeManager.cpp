#include "World_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

AWorld_BiomeManager::AWorld_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default biome regions
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.BiomeCenter = FVector(0, 0, 0);
    ForestBiome.BiomeRadius = 3000.0f;
    ForestBiome.VegetationDensity = 1.0f;
    ForestBiome.BiomeTint = FLinearColor::Green;
    BiomeRegions.Add(ForestBiome);

    FWorld_BiomeData SwampBiome;
    SwampBiome.BiomeType = EWorld_BiomeType::Swampland;
    SwampBiome.BiomeCenter = FVector(-5000, 3000, 0);
    SwampBiome.BiomeRadius = 2500.0f;
    SwampBiome.VegetationDensity = 0.8f;
    SwampBiome.WaterLevel = 50.0f;
    SwampBiome.BiomeTint = FLinearColor(0.2f, 0.4f, 0.3f, 1.0f);
    BiomeRegions.Add(SwampBiome);

    FWorld_BiomeData CanyonBiome;
    CanyonBiome.BiomeType = EWorld_BiomeType::Canyon;
    CanyonBiome.BiomeCenter = FVector(5000, -3000, 0);
    CanyonBiome.BiomeRadius = 3500.0f;
    CanyonBiome.VegetationDensity = 0.3f;
    CanyonBiome.BiomeTint = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    BiomeRegions.Add(CanyonBiome);
}

void AWorld_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-generate biomes on play
    GenerateAllBiomes();
}

void AWorld_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorld_BiomeManager::GenerateAllBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating all biomes..."));
    
    for (const FWorld_BiomeData& BiomeData : BiomeRegions)
    {
        GenerateBiome(BiomeData);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("All biomes generated successfully"));
}

void AWorld_BiomeManager::GenerateBiome(const FWorld_BiomeData& BiomeData)
{
    switch (BiomeData.BiomeType)
    {
        case EWorld_BiomeType::Swampland:
            CreateSwamplandBiome(BiomeData.BiomeCenter, BiomeData.BiomeRadius);
            break;
        case EWorld_BiomeType::Canyon:
            CreateCanyonBiome(BiomeData.BiomeCenter, BiomeData.BiomeRadius);
            break;
        case EWorld_BiomeType::Forest:
            CreateForestBiome(BiomeData.BiomeCenter, BiomeData.BiomeRadius);
            break;
        default:
            SpawnVegetationInBiome(BiomeData);
            break;
    }
    
    SpawnWaterFeatures(BiomeData);
    ApplyBiomeLighting(BiomeData);
}

EWorld_BiomeType AWorld_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float ClosestDistance = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Forest;
    
    for (const FWorld_BiomeData& BiomeData : BiomeRegions)
    {
        float Distance = FVector::Dist(Location, BiomeData.BiomeCenter);
        if (Distance < BiomeData.BiomeRadius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = BiomeData.BiomeType;
        }
    }
    
    return ClosestBiome;
}

void AWorld_BiomeManager::CreateSwamplandBiome(const FVector& Center, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Creating swampland biome at %s"), *Center.ToString());
    
    // Create water pools
    int32 WaterPoolCount = FMath::RandRange(3, 6);
    for (int32 i = 0; i < WaterPoolCount; i++)
    {
        FVector WaterLocation = Center + FVector(
            FMath::RandRange(-Radius * 0.8f, Radius * 0.8f),
            FMath::RandRange(-Radius * 0.8f, Radius * 0.8f),
            50.0f
        );
        
        AActor* WaterPlane = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), WaterLocation, FRotator::ZeroRotator);
        if (WaterPlane)
        {
            WaterPlane->SetActorLabel(FString::Printf(TEXT("SwampWater_%d"), i));
            SpawnedBiomeActors.Add(WaterPlane);
        }
    }
    
    // Create swamp trees
    int32 TreeCount = FMath::RandRange(15, 25);
    for (int32 i = 0; i < TreeCount; i++)
    {
        FVector TreeLocation = Center + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        AActor* SwampTree = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), TreeLocation, FRotator::ZeroRotator);
        if (SwampTree)
        {
            SwampTree->SetActorLabel(FString::Printf(TEXT("SwampTree_%d"), i));
            SpawnedBiomeActors.Add(SwampTree);
        }
    }
}

void AWorld_BiomeManager::CreateCanyonBiome(const FVector& Center, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Creating canyon biome at %s"), *Center.ToString());
    
    // Create rock formations
    int32 RockCount = FMath::RandRange(8, 15);
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector RockLocation = Center + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(0.0f, 500.0f)
        );
        
        AActor* CanyonRock = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), RockLocation, FRotator::ZeroRotator);
        if (CanyonRock)
        {
            CanyonRock->SetActorLabel(FString::Printf(TEXT("CanyonRock_%d"), i));
            SpawnedBiomeActors.Add(CanyonRock);
        }
    }
    
    // Create desert vegetation
    int32 CactusCount = FMath::RandRange(5, 12);
    for (int32 i = 0; i < CactusCount; i++)
    {
        FVector CactusLocation = Center + FVector(
            FMath::RandRange(-Radius * 0.7f, Radius * 0.7f),
            FMath::RandRange(-Radius * 0.7f, Radius * 0.7f),
            0.0f
        );
        
        AActor* DesertCactus = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), CactusLocation, FRotator::ZeroRotator);
        if (DesertCactus)
        {
            DesertCactus->SetActorLabel(FString::Printf(TEXT("DesertCactus_%d"), i));
            SpawnedBiomeActors.Add(DesertCactus);
        }
    }
}

void AWorld_BiomeManager::CreateForestBiome(const FVector& Center, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Creating forest biome at %s"), *Center.ToString());
    
    // Create dense forest trees
    int32 TreeCount = FMath::RandRange(20, 35);
    for (int32 i = 0; i < TreeCount; i++)
    {
        FVector TreeLocation = Center + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        AActor* ForestTree = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), TreeLocation, FRotator::ZeroRotator);
        if (ForestTree)
        {
            ForestTree->SetActorLabel(FString::Printf(TEXT("ForestTree_%d"), i));
            SpawnedBiomeActors.Add(ForestTree);
        }
    }
    
    // Create undergrowth
    int32 BushCount = FMath::RandRange(30, 50);
    for (int32 i = 0; i < BushCount; i++)
    {
        FVector BushLocation = Center + FVector(
            FMath::RandRange(-Radius * 0.9f, Radius * 0.9f),
            FMath::RandRange(-Radius * 0.9f, Radius * 0.9f),
            0.0f
        );
        
        AActor* ForestBush = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), BushLocation, FRotator::ZeroRotator);
        if (ForestBush)
        {
            ForestBush->SetActorLabel(FString::Printf(TEXT("ForestBush_%d"), i));
            SpawnedBiomeActors.Add(ForestBush);
        }
    }
}

void AWorld_BiomeManager::SpawnVegetationInBiome(const FWorld_BiomeData& BiomeData)
{
    int32 VegetationCount = FMath::RoundToInt(MaxVegetationPerBiome * BiomeData.VegetationDensity);
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector SpawnLocation = BiomeData.BiomeCenter + FVector(
            FMath::RandRange(-BiomeData.BiomeRadius, BiomeData.BiomeRadius),
            FMath::RandRange(-BiomeData.BiomeRadius, BiomeData.BiomeRadius),
            0.0f
        );
        
        AActor* Vegetation = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
        if (Vegetation)
        {
            Vegetation->SetActorLabel(FString::Printf(TEXT("Vegetation_%d"), i));
            SpawnedBiomeActors.Add(Vegetation);
        }
    }
}

void AWorld_BiomeManager::SpawnWaterFeatures(const FWorld_BiomeData& BiomeData)
{
    if (BiomeData.WaterLevel > 0.0f)
    {
        FVector WaterLocation = BiomeData.BiomeCenter;
        WaterLocation.Z = BiomeData.WaterLevel;
        
        AActor* WaterFeature = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), WaterLocation, FRotator::ZeroRotator);
        if (WaterFeature)
        {
            WaterFeature->SetActorLabel(TEXT("BiomeWater"));
            SpawnedBiomeActors.Add(WaterFeature);
        }
    }
}

void AWorld_BiomeManager::ApplyBiomeLighting(const FWorld_BiomeData& BiomeData)
{
    // Apply biome-specific lighting effects
    UE_LOG(LogTemp, Log, TEXT("Applying lighting for biome at %s"), *BiomeData.BiomeCenter.ToString());
}

void AWorld_BiomeManager::DebugShowBiomeBounds()
{
    if (!GetWorld()) return;
    
    for (const FWorld_BiomeData& BiomeData : BiomeRegions)
    {
        DrawDebugSphere(GetWorld(), BiomeData.BiomeCenter, BiomeData.BiomeRadius, 32, BiomeData.BiomeTint.ToFColor(false), false, 10.0f, 0, 5.0f);
    }
}

void AWorld_BiomeManager::ClearAllBiomes()
{
    for (AActor* Actor : SpawnedBiomeActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedBiomeActors.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("All biome actors cleared"));
}