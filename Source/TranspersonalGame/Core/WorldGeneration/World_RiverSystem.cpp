#include "RiverSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogRiverSystem);

UWorld_RiverSystem::UWorld_RiverSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // River generation parameters
    RiverWidth = 1000.0f; // 10m wide rivers
    RiverDepth = 200.0f;  // 2m deep
    FlowSpeed = 100.0f;   // 1m/s flow
    
    // Biome river configurations
    InitializeBiomeRiverData();
    
    UE_LOG(LogRiverSystem, Warning, TEXT("RiverSystem initialized for 5 biomes"));
}

void UWorld_RiverSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Generate rivers for all biomes
    GenerateAllBiomeRivers();
}

void UWorld_RiverSystem::InitializeBiomeRiverData()
{
    BiomeRiverConfigs.Empty();
    
    // PANTANO (Swamp) - Dense water network
    FWorld_BiomeRiverConfig SwampConfig;
    SwampConfig.BiomeName = TEXT("Pantano");
    SwampConfig.BiomeCenter = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampConfig.BiomeBounds = FBox(FVector(-77500, -76500, -100), FVector(-25000, -15000, 100));
    SwampConfig.RiverCount = 8;
    SwampConfig.RiverWidth = 800.0f;
    SwampConfig.RiverDepth = 150.0f;
    SwampConfig.bHasLakes = true;
    SwampConfig.LakeCount = 4;
    SwampConfig.WaterColor = FLinearColor(0.2f, 0.4f, 0.3f, 0.8f); // Murky green
    BiomeRiverConfigs.Add(SwampConfig);
    
    // FLORESTA (Forest) - Clear streams
    FWorld_BiomeRiverConfig ForestConfig;
    ForestConfig.BiomeName = TEXT("Floresta");
    ForestConfig.BiomeCenter = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestConfig.BiomeBounds = FBox(FVector(-77500, 15000, -50), FVector(-15000, 76500, 200));
    ForestConfig.RiverCount = 5;
    ForestConfig.RiverWidth = 600.0f;
    ForestConfig.RiverDepth = 180.0f;
    ForestConfig.bHasLakes = true;
    ForestConfig.LakeCount = 2;
    ForestConfig.WaterColor = FLinearColor(0.1f, 0.3f, 0.6f, 0.9f); // Clear blue
    BiomeRiverConfigs.Add(ForestConfig);
    
    // SAVANA (Savanna) - Seasonal rivers
    FWorld_BiomeRiverConfig SavannaConfig;
    SavannaConfig.BiomeName = TEXT("Savana");
    SavannaConfig.BiomeCenter = FVector(0.0f, 0.0f, 0.0f);
    SavannaConfig.BiomeBounds = FBox(FVector(-20000, -20000, -30), FVector(20000, 20000, 50));
    SavannaConfig.RiverCount = 3;
    SavannaConfig.RiverWidth = 1200.0f;
    SavannaConfig.RiverDepth = 250.0f;
    SavannaConfig.bHasLakes = true;
    SavannaConfig.LakeCount = 1;
    SavannaConfig.WaterColor = FLinearColor(0.3f, 0.4f, 0.5f, 0.7f); // Muddy brown
    BiomeRiverConfigs.Add(SavannaConfig);
    
    // DESERTO (Desert) - Rare oases
    FWorld_BiomeRiverConfig DesertConfig;
    DesertConfig.BiomeName = TEXT("Deserto");
    DesertConfig.BiomeCenter = FVector(55000.0f, 0.0f, 0.0f);
    DesertConfig.BiomeBounds = FBox(FVector(25000, -30000, -20), FVector(79500, 30000, 100));
    DesertConfig.RiverCount = 1;
    DesertConfig.RiverWidth = 400.0f;
    DesertConfig.RiverDepth = 100.0f;
    DesertConfig.bHasLakes = true;
    DesertConfig.LakeCount = 3; // Oases
    DesertConfig.WaterColor = FLinearColor(0.2f, 0.5f, 0.7f, 0.95f); // Pure blue oasis
    BiomeRiverConfigs.Add(DesertConfig);
    
    // MONTANHA NEVADA (Snowy Mountain) - Glacial streams
    FWorld_BiomeRiverConfig MountainConfig;
    MountainConfig.BiomeName = TEXT("Montanha");
    MountainConfig.BiomeCenter = FVector(40000.0f, 50000.0f, 500.0f);
    MountainConfig.BiomeBounds = FBox(FVector(15000, 20000, 200), FVector(79500, 76500, 1000));
    MountainConfig.RiverCount = 4;
    MountainConfig.RiverWidth = 500.0f;
    MountainConfig.RiverDepth = 120.0f;
    MountainConfig.bHasLakes = true;
    MountainConfig.LakeCount = 2;
    MountainConfig.WaterColor = FLinearColor(0.8f, 0.9f, 1.0f, 0.95f); // Icy blue
    BiomeRiverConfigs.Add(MountainConfig);
    
    UE_LOG(LogRiverSystem, Warning, TEXT("Initialized river configs for %d biomes"), BiomeRiverConfigs.Num());
}

void UWorld_RiverSystem::GenerateAllBiomeRivers()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    GeneratedRivers.Empty();
    GeneratedLakes.Empty();
    
    for (const FWorld_BiomeRiverConfig& Config : BiomeRiverConfigs)
    {
        GenerateBiomeRivers(Config);
    }
    
    UE_LOG(LogRiverSystem, Warning, TEXT("Generated %d rivers and %d lakes across all biomes"), 
           GeneratedRivers.Num(), GeneratedLakes.Num());
}

void UWorld_RiverSystem::GenerateBiomeRivers(const FWorld_BiomeRiverConfig& Config)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Generate rivers for this biome
    for (int32 i = 0; i < Config.RiverCount; i++)
    {
        FWorld_RiverData RiverData = GenerateRiverInBounds(Config, i);
        GeneratedRivers.Add(RiverData);
        
        // Create visual representation
        CreateRiverActor(RiverData);
    }
    
    // Generate lakes/oases for this biome
    if (Config.bHasLakes)
    {
        for (int32 i = 0; i < Config.LakeCount; i++)
        {
            FWorld_LakeData LakeData = GenerateLakeInBounds(Config, i);
            GeneratedLakes.Add(LakeData);
            
            // Create visual representation
            CreateLakeActor(LakeData);
        }
    }
    
    UE_LOG(LogRiverSystem, Log, TEXT("Generated %d rivers and %d lakes for biome: %s"), 
           Config.RiverCount, Config.LakeCount, *Config.BiomeName);
}

FWorld_RiverData UWorld_RiverSystem::GenerateRiverInBounds(const FWorld_BiomeRiverConfig& Config, int32 RiverIndex)
{
    FWorld_RiverData RiverData;
    RiverData.BiomeName = Config.BiomeName;
    RiverData.RiverIndex = RiverIndex;
    RiverData.Width = Config.RiverWidth;
    RiverData.Depth = Config.RiverDepth;
    RiverData.WaterColor = Config.WaterColor;
    
    // Generate river path within biome bounds
    FVector BiomeSize = Config.BiomeBounds.GetSize();
    FVector BiomeMin = Config.BiomeBounds.Min;
    
    // Create a winding river path
    int32 PathPoints = 8;
    RiverData.PathPoints.Empty();
    
    for (int32 i = 0; i < PathPoints; i++)
    {
        float Alpha = (float)i / (PathPoints - 1);
        
        // Base path from one side of biome to another
        FVector BasePoint = FVector(
            BiomeMin.X + BiomeSize.X * Alpha,
            BiomeMin.Y + BiomeSize.Y * 0.5f,
            Config.BiomeCenter.Z
        );
        
        // Add random variation for natural curves
        FVector Variation = FVector(
            FMath::RandRange(-BiomeSize.X * 0.2f, BiomeSize.X * 0.2f),
            FMath::RandRange(-BiomeSize.Y * 0.3f, BiomeSize.Y * 0.3f),
            FMath::RandRange(-50.0f, 50.0f)
        );
        
        FVector RiverPoint = BasePoint + Variation;
        
        // Ensure point stays within biome bounds
        RiverPoint.X = FMath::Clamp(RiverPoint.X, BiomeMin.X, Config.BiomeBounds.Max.X);
        RiverPoint.Y = FMath::Clamp(RiverPoint.Y, BiomeMin.Y, Config.BiomeBounds.Max.Y);
        
        RiverData.PathPoints.Add(RiverPoint);
    }
    
    return RiverData;
}

FWorld_LakeData UWorld_RiverSystem::GenerateLakeInBounds(const FWorld_BiomeRiverConfig& Config, int32 LakeIndex)
{
    FWorld_LakeData LakeData;
    LakeData.BiomeName = Config.BiomeName;
    LakeData.LakeIndex = LakeIndex;
    LakeData.WaterColor = Config.WaterColor;
    
    // Generate random lake position within biome
    FVector BiomeSize = Config.BiomeBounds.GetSize();
    FVector BiomeMin = Config.BiomeBounds.Min;
    
    LakeData.Location = FVector(
        BiomeMin.X + FMath::RandRange(BiomeSize.X * 0.2f, BiomeSize.X * 0.8f),
        BiomeMin.Y + FMath::RandRange(BiomeSize.Y * 0.2f, BiomeSize.Y * 0.8f),
        Config.BiomeCenter.Z - 50.0f // Slightly below ground
    );
    
    // Lake size varies by biome type
    if (Config.BiomeName == TEXT("Pantano"))
    {
        LakeData.Radius = FMath::RandRange(2000.0f, 4000.0f); // Large swamp pools
    }
    else if (Config.BiomeName == TEXT("Deserto"))
    {
        LakeData.Radius = FMath::RandRange(800.0f, 1500.0f); // Small oases
    }
    else
    {
        LakeData.Radius = FMath::RandRange(1500.0f, 3000.0f); // Medium lakes
    }
    
    LakeData.Depth = Config.RiverDepth * 1.5f;
    
    return LakeData;
}

void UWorld_RiverSystem::CreateRiverActor(const FWorld_RiverData& RiverData)
{
    UWorld* World = GetWorld();
    if (!World || RiverData.PathPoints.Num() < 2) return;
    
    // Create river segments between path points
    for (int32 i = 0; i < RiverData.PathPoints.Num() - 1; i++)
    {
        FVector StartPoint = RiverData.PathPoints[i];
        FVector EndPoint = RiverData.PathPoints[i + 1];
        
        // Create river segment actor
        AStaticMeshActor* RiverActor = World->SpawnActor<AStaticMeshActor>();
        if (RiverActor)
        {
            // Position and orient the river segment
            FVector MidPoint = (StartPoint + EndPoint) * 0.5f;
            FVector Direction = (EndPoint - StartPoint).GetSafeNormal();
            float Distance = FVector::Dist(StartPoint, EndPoint);
            
            RiverActor->SetActorLocation(MidPoint);
            RiverActor->SetActorRotation(Direction.Rotation());
            
            // Scale to match river width and length
            FVector Scale = FVector(Distance / 100.0f, RiverData.Width / 100.0f, 1.0f);
            RiverActor->SetActorScale3D(Scale);
            
            // Set actor name for identification
            FString ActorName = FString::Printf(TEXT("River_%s_%d_Seg_%d"), 
                                              *RiverData.BiomeName, RiverData.RiverIndex, i);
            RiverActor->SetActorLabel(ActorName);
            
            UE_LOG(LogRiverSystem, Log, TEXT("Created river segment: %s at %s"), 
                   *ActorName, *MidPoint.ToString());
        }
    }
}

void UWorld_RiverSystem::CreateLakeActor(const FWorld_LakeData& LakeData)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create lake actor
    AStaticMeshActor* LakeActor = World->SpawnActor<AStaticMeshActor>();
    if (LakeActor)
    {
        LakeActor->SetActorLocation(LakeData.Location);
        
        // Scale to match lake radius
        float Scale = LakeData.Radius / 100.0f; // Assuming 100cm base mesh
        LakeActor->SetActorScale3D(FVector(Scale, Scale, 1.0f));
        
        // Set actor name for identification
        FString ActorName = FString::Printf(TEXT("Lake_%s_%d"), 
                                          *LakeData.BiomeName, LakeData.LakeIndex);
        LakeActor->SetActorLabel(ActorName);
        
        UE_LOG(LogRiverSystem, Log, TEXT("Created lake: %s at %s (Radius: %.1f)"), 
               *ActorName, *LakeData.Location.ToString(), LakeData.Radius);
    }
}

TArray<FWorld_RiverData> UWorld_RiverSystem::GetRiversInBiome(const FString& BiomeName) const
{
    TArray<FWorld_RiverData> BiomeRivers;
    
    for (const FWorld_RiverData& River : GeneratedRivers)
    {
        if (River.BiomeName == BiomeName)
        {
            BiomeRivers.Add(River);
        }
    }
    
    return BiomeRivers;
}

TArray<FWorld_LakeData> UWorld_RiverSystem::GetLakesInBiome(const FString& BiomeName) const
{
    TArray<FWorld_LakeData> BiomeLakes;
    
    for (const FWorld_LakeData& Lake : GeneratedLakes)
    {
        if (Lake.BiomeName == BiomeName)
        {
            BiomeLakes.Add(Lake);
        }
    }
    
    return BiomeLakes;
}

int32 UWorld_RiverSystem::GetTotalWaterBodies() const
{
    return GeneratedRivers.Num() + GeneratedLakes.Num();
}

void UWorld_RiverSystem::RegenerateAllRivers()
{
    // Clear existing rivers
    ClearAllWaterBodies();
    
    // Regenerate all rivers and lakes
    GenerateAllBiomeRivers();
    
    UE_LOG(LogRiverSystem, Warning, TEXT("Regenerated all water bodies - Total: %d"), GetTotalWaterBodies());
}

void UWorld_RiverSystem::ClearAllWaterBodies()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find and destroy all river/lake actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);
    
    int32 DestroyedCount = 0;
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetActorLabel();
        if (ActorName.StartsWith(TEXT("River_")) || ActorName.StartsWith(TEXT("Lake_")))
        {
            Actor->Destroy();
            DestroyedCount++;
        }
    }
    
    GeneratedRivers.Empty();
    GeneratedLakes.Empty();
    
    UE_LOG(LogRiverSystem, Warning, TEXT("Cleared %d water body actors"), DestroyedCount);
}