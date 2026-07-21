#include "World_BiomeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Landscape/Landscape.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Math/UnrealMathUtility.h"

UWorld_BiomeManager::UWorld_BiomeManager()
{
    bIsInitialized = false;
    bLODBasedGeneration = true;
    MaxGenerationDistance = 10000.0f;
    MaxActiveFeatures = 500;
}

void UWorld_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Initializing biome management system"));
    
    InitializeBiomePresets();
    bIsInitialized = true;
    
    // Auto-generate world biomes on startup
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UWorld_BiomeManager::GenerateWorldBiomes, 2.0f, false);
}

void UWorld_BiomeManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Cleaning up biome system"));
    
    ClearGeneratedContent();
    ActiveBiomes.Empty();
    TerrainFeatures.Empty();
    BiomePresets.Empty();
    
    Super::Deinitialize();
}

bool UWorld_BiomeManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UWorld_BiomeManager::InitializeBiomePresets()
{
    // Forest Biome
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.VegetationDensity = 0.8f;
    ForestBiome.TerrainRoughness = 0.6f;
    ForestBiome.FogColor = FLinearColor(0.4f, 0.6f, 0.4f, 1.0f);
    ForestBiome.FogDensity = 0.015f;
    BiomePresets.Add(EWorld_BiomeType::Forest, ForestBiome);

    // Swampland Biome
    FWorld_BiomeData SwampBiome;
    SwampBiome.BiomeType = EWorld_BiomeType::Swampland;
    SwampBiome.VegetationDensity = 0.6f;
    SwampBiome.TerrainRoughness = 0.3f;
    SwampBiome.FogColor = FLinearColor(0.5f, 0.5f, 0.4f, 1.0f);
    SwampBiome.FogDensity = 0.04f;
    BiomePresets.Add(EWorld_BiomeType::Swampland, SwampBiome);

    // Canyon Biome
    FWorld_BiomeData CanyonBiome;
    CanyonBiome.BiomeType = EWorld_BiomeType::Canyon;
    CanyonBiome.VegetationDensity = 0.2f;
    CanyonBiome.TerrainRoughness = 0.9f;
    CanyonBiome.FogColor = FLinearColor(0.7f, 0.5f, 0.4f, 1.0f);
    CanyonBiome.FogDensity = 0.01f;
    BiomePresets.Add(EWorld_BiomeType::Canyon, CanyonBiome);

    // Plains Biome
    FWorld_BiomeData PlainsBiome;
    PlainsBiome.BiomeType = EWorld_BiomeType::Plains;
    PlainsBiome.VegetationDensity = 0.4f;
    PlainsBiome.TerrainRoughness = 0.2f;
    PlainsBiome.FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    PlainsBiome.FogDensity = 0.005f;
    BiomePresets.Add(EWorld_BiomeType::Plains, PlainsBiome);

    // Riverbank Biome
    FWorld_BiomeData RiverbankBiome;
    RiverbankBiome.BiomeType = EWorld_BiomeType::Riverbank;
    RiverbankBiome.VegetationDensity = 0.7f;
    RiverbankBiome.TerrainRoughness = 0.4f;
    RiverbankBiome.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    RiverbankBiome.FogDensity = 0.02f;
    BiomePresets.Add(EWorld_BiomeType::Riverbank, RiverbankBiome);

    // Highlands Biome
    FWorld_BiomeData HighlandsBiome;
    HighlandsBiome.BiomeType = EWorld_BiomeType::Highlands;
    HighlandsBiome.VegetationDensity = 0.3f;
    HighlandsBiome.TerrainRoughness = 0.8f;
    HighlandsBiome.FogColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
    HighlandsBiome.FogDensity = 0.025f;
    BiomePresets.Add(EWorld_BiomeType::Highlands, HighlandsBiome);

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Initialized %d biome presets"), BiomePresets.Num());
}

void UWorld_BiomeManager::GenerateWorldBiomes()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("World_BiomeManager: Cannot generate biomes - system not initialized"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Generating world biomes"));

    // Clear existing biomes
    ActiveBiomes.Empty();

    // Create biome layout - 6 biomes in a hex pattern
    CreateBiome(EWorld_BiomeType::Forest, FVector2D(0, 0), 4000.0f);
    CreateBiome(EWorld_BiomeType::Plains, FVector2D(6000, 0), 3500.0f);
    CreateBiome(EWorld_BiomeType::Swampland, FVector2D(-3000, 5000), 3000.0f);
    CreateBiome(EWorld_BiomeType::Canyon, FVector2D(3000, 5000), 2500.0f);
    CreateBiome(EWorld_BiomeType::Riverbank, FVector2D(-3000, -5000), 3500.0f);
    CreateBiome(EWorld_BiomeType::Highlands, FVector2D(3000, -5000), 4000.0f);

    // Generate terrain features for each biome
    SpawnTerrainFeatures();
    CreateRiverSystem();
    GenerateVegetationClusters();

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Generated %d biomes with terrain features"), ActiveBiomes.Num());
}

void UWorld_BiomeManager::CreateBiome(EWorld_BiomeType BiomeType, FVector2D Location, float Radius)
{
    if (!BiomePresets.Contains(BiomeType))
    {
        UE_LOG(LogTemp, Error, TEXT("World_BiomeManager: Unknown biome type"));
        return;
    }

    FWorld_BiomeData NewBiome = BiomePresets[BiomeType];
    NewBiome.CenterLocation = Location;
    NewBiome.Radius = Radius;

    ActiveBiomes.Add(NewBiome);

    // Create biome-specific terrain features
    switch (BiomeType)
    {
        case EWorld_BiomeType::Forest:
            CreateForestBiome(Location, Radius);
            break;
        case EWorld_BiomeType::Swampland:
            CreateSwamplandBiome(Location, Radius);
            break;
        case EWorld_BiomeType::Canyon:
            CreateCanyonBiome(Location, Radius);
            break;
        case EWorld_BiomeType::Plains:
            CreatePlainsBiome(Location, Radius);
            break;
        case EWorld_BiomeType::Riverbank:
            CreateRiverbankBiome(Location, Radius);
            break;
        case EWorld_BiomeType::Highlands:
            CreateHighlandsBiome(Location, Radius);
            break;
    }
}

void UWorld_BiomeManager::CreateForestBiome(FVector2D Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Create dense tree clusters
    int32 TreeCount = FMath::RandRange(15, 25);
    for (int32 i = 0; i < TreeCount; i++)
    {
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(500.0f, Radius * 0.8f);
        FVector TreeLocation = FVector(
            Location.X + FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            Location.Y + FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            100.0f
        );

        if (ShouldGenerateAtLocation(TreeLocation))
        {
            AStaticMeshActor* TreeActor = World->SpawnActor<AStaticMeshActor>();
            if (TreeActor)
            {
                TreeActor->SetActorLocation(TreeLocation);
                TreeActor->SetActorScale3D(FVector(FMath::RandRange(1.5f, 3.0f)));
                TreeActor->SetActorLabel(FString::Printf(TEXT("ForestTree_%d"), i));
                GeneratedActors.Add(TreeActor);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("World_BiomeManager: Created forest biome at (%f, %f) with %d trees"), 
           Location.X, Location.Y, TreeCount);
}

void UWorld_BiomeManager::CreateSwamplandBiome(FVector2D Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Create water pools and cypress trees
    int32 PoolCount = FMath::RandRange(5, 8);
    for (int32 i = 0; i < PoolCount; i++)
    {
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(200.0f, Radius * 0.6f);
        FVector PoolLocation = FVector(
            Location.X + FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            Location.Y + FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            -20.0f
        );

        if (ShouldGenerateAtLocation(PoolLocation))
        {
            AStaticMeshActor* PoolActor = World->SpawnActor<AStaticMeshActor>();
            if (PoolActor)
            {
                PoolActor->SetActorLocation(PoolLocation);
                PoolActor->SetActorScale3D(FVector(FMath::RandRange(3.0f, 6.0f), FMath::RandRange(3.0f, 6.0f), 0.1f));
                PoolActor->SetActorLabel(FString::Printf(TEXT("SwampPool_%d"), i));
                GeneratedActors.Add(PoolActor);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("World_BiomeManager: Created swampland biome at (%f, %f) with %d pools"), 
           Location.X, Location.Y, PoolCount);
}

void UWorld_BiomeManager::CreateCanyonBiome(FVector2D Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Create rock formations and cliff faces
    int32 RockCount = FMath::RandRange(8, 12);
    for (int32 i = 0; i < RockCount; i++)
    {
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(300.0f, Radius * 0.7f);
        FVector RockLocation = FVector(
            Location.X + FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            Location.Y + FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::RandRange(200.0f, 800.0f)
        );

        if (ShouldGenerateAtLocation(RockLocation))
        {
            AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>();
            if (RockActor)
            {
                RockActor->SetActorLocation(RockLocation);
                RockActor->SetActorScale3D(FVector(FMath::RandRange(2.0f, 5.0f), FMath::RandRange(2.0f, 5.0f), FMath::RandRange(3.0f, 8.0f)));
                RockActor->SetActorLabel(FString::Printf(TEXT("CanyonRock_%d"), i));
                GeneratedActors.Add(RockActor);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("World_BiomeManager: Created canyon biome at (%f, %f) with %d rock formations"), 
           Location.X, Location.Y, RockCount);
}

void UWorld_BiomeManager::CreatePlainsBiome(FVector2D Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Create scattered grass clusters and occasional trees
    int32 GrassClusterCount = FMath::RandRange(20, 30);
    for (int32 i = 0; i < GrassClusterCount; i++)
    {
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(100.0f, Radius * 0.9f);
        FVector GrassLocation = FVector(
            Location.X + FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            Location.Y + FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            50.0f
        );

        if (ShouldGenerateAtLocation(GrassLocation))
        {
            AStaticMeshActor* GrassActor = World->SpawnActor<AStaticMeshActor>();
            if (GrassActor)
            {
                GrassActor->SetActorLocation(GrassLocation);
                GrassActor->SetActorScale3D(FVector(FMath::RandRange(1.0f, 2.0f)));
                GrassActor->SetActorLabel(FString::Printf(TEXT("PlainsGrass_%d"), i));
                GeneratedActors.Add(GrassActor);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("World_BiomeManager: Created plains biome at (%f, %f) with %d grass clusters"), 
           Location.X, Location.Y, GrassClusterCount);
}

void UWorld_BiomeManager::CreateRiverbankBiome(FVector2D Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Create river path with vegetation along banks
    int32 RiverSegments = 8;
    for (int32 i = 0; i < RiverSegments; i++)
    {
        float Progress = (float)i / (float)RiverSegments;
        FVector RiverLocation = FVector(
            Location.X + (Progress - 0.5f) * Radius * 1.5f,
            Location.Y + FMath::Sin(Progress * PI * 2) * Radius * 0.3f,
            -10.0f
        );

        if (ShouldGenerateAtLocation(RiverLocation))
        {
            AStaticMeshActor* WaterActor = World->SpawnActor<AStaticMeshActor>();
            if (WaterActor)
            {
                WaterActor->SetActorLocation(RiverLocation);
                WaterActor->SetActorScale3D(FVector(8.0f, 4.0f, 0.1f));
                WaterActor->SetActorLabel(FString::Printf(TEXT("RiverSegment_%d"), i));
                GeneratedActors.Add(WaterActor);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("World_BiomeManager: Created riverbank biome at (%f, %f) with %d river segments"), 
           Location.X, Location.Y, RiverSegments);
}

void UWorld_BiomeManager::CreateHighlandsBiome(FVector2D Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Create elevated terrain with sparse vegetation
    int32 HillCount = FMath::RandRange(6, 10);
    for (int32 i = 0; i < HillCount; i++)
    {
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(400.0f, Radius * 0.8f);
        FVector HillLocation = FVector(
            Location.X + FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            Location.Y + FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::RandRange(300.0f, 1000.0f)
        );

        if (ShouldGenerateAtLocation(HillLocation))
        {
            AStaticMeshActor* HillActor = World->SpawnActor<AStaticMeshActor>();
            if (HillActor)
            {
                HillActor->SetActorLocation(HillLocation);
                HillActor->SetActorScale3D(FVector(FMath::RandRange(4.0f, 8.0f), FMath::RandRange(4.0f, 8.0f), FMath::RandRange(2.0f, 4.0f)));
                HillActor->SetActorLabel(FString::Printf(TEXT("HighlandsHill_%d"), i));
                GeneratedActors.Add(HillActor);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("World_BiomeManager: Created highlands biome at (%f, %f) with %d hills"), 
           Location.X, Location.Y, HillCount);
}

EWorld_BiomeType UWorld_BiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    FVector2D Location2D(WorldLocation.X, WorldLocation.Y);
    
    for (const FWorld_BiomeData& Biome : ActiveBiomes)
    {
        float Distance = FVector2D::Distance(Location2D, Biome.CenterLocation);
        if (Distance <= Biome.Radius)
        {
            return Biome.BiomeType;
        }
    }
    
    return EWorld_BiomeType::Plains; // Default biome
}

FWorld_BiomeData UWorld_BiomeManager::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    if (BiomePresets.Contains(BiomeType))
    {
        return BiomePresets[BiomeType];
    }
    
    return FWorld_BiomeData(); // Default data
}

void UWorld_BiomeManager::SpawnTerrainFeatures()
{
    UE_LOG(LogTemp, Log, TEXT("World_BiomeManager: Spawning terrain features"));
    
    // Features are spawned as part of biome creation
    // This function can be used for additional feature passes
}

void UWorld_BiomeManager::CreateRiverSystem()
{
    UE_LOG(LogTemp, Log, TEXT("World_BiomeManager: Creating river system"));
    
    // River system is created as part of riverbank biome
    // Additional rivers can be added here
}

void UWorld_BiomeManager::GenerateVegetationClusters()
{
    UE_LOG(LogTemp, Log, TEXT("World_BiomeManager: Generating vegetation clusters"));
    
    // Vegetation is generated as part of each biome
    // Additional vegetation passes can be added here
}

void UWorld_BiomeManager::SetLODBasedGeneration(bool bEnabled)
{
    bLODBasedGeneration = bEnabled;
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: LOD-based generation %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UWorld_BiomeManager::UpdateGenerationBasedOnPerformance()
{
    if (!bLODBasedGeneration) return;
    
    // Clean up distant features to maintain performance
    CleanupDistantFeatures();
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeManager: Updated generation based on performance"));
}

void UWorld_BiomeManager::RegenerateAllBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Regenerating all biomes"));
    
    ClearGeneratedContent();
    GenerateWorldBiomes();
}

void UWorld_BiomeManager::ClearGeneratedContent()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Clearing generated content"));
    
    for (AActor* Actor : GeneratedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    GeneratedActors.Empty();
    TerrainFeatures.Empty();
}

void UWorld_BiomeManager::ValidateWorldGeneration() const
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Validation - %d active biomes, %d terrain features, %d generated actors"),
           ActiveBiomes.Num(), TerrainFeatures.Num(), GeneratedActors.Num());
}

float UWorld_BiomeManager::CalculateDistanceToPlayer(FVector Location) const
{
    UWorld* World = GetWorld();
    if (!World) return MAX_FLT;
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return MAX_FLT;
    
    return FVector::Distance(Location, PC->GetPawn()->GetActorLocation());
}

bool UWorld_BiomeManager::ShouldGenerateAtLocation(FVector Location) const
{
    if (!bLODBasedGeneration) return true;
    
    float DistanceToPlayer = CalculateDistanceToPlayer(Location);
    return DistanceToPlayer <= MaxGenerationDistance;
}

void UWorld_BiomeManager::CleanupDistantFeatures()
{
    TArray<AActor*> ActorsToRemove;
    
    for (AActor* Actor : GeneratedActors)
    {
        if (IsValid(Actor))
        {
            float Distance = CalculateDistanceToPlayer(Actor->GetActorLocation());
            if (Distance > MaxGenerationDistance * 1.2f) // Add hysteresis
            {
                ActorsToRemove.Add(Actor);
            }
        }
    }
    
    for (AActor* Actor : ActorsToRemove)
    {
        GeneratedActors.Remove(Actor);
        Actor->Destroy();
    }
    
    if (ActorsToRemove.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("World_BiomeManager: Cleaned up %d distant features"), ActorsToRemove.Num());
    }
}