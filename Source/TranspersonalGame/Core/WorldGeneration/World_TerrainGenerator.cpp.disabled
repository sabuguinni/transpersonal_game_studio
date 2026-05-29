#include "World_TerrainGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UWorld_TerrainGenerator::UWorld_TerrainGenerator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    // Initialize default settings
    ChunkSize = 2000.0f;
    MaxChunks = 25;
    NoiseScale = 0.01f;
    HeightMultiplier = 300.0f;
    bAutoGenerate = false;
    CurrentChunkCount = 0;

    // Initialize biome settings
    InitializeBiomeSettings();
}

void UWorld_TerrainGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoGenerate)
    {
        RegenerateAllTerrain();
    }
}

void UWorld_TerrainGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWorld_TerrainGenerator::GenerateTerrainChunk(const FVector& ChunkCenter, EWorld_TerrainType TerrainType)
{
    if (CurrentChunkCount >= MaxChunks)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum chunk count reached: %d"), MaxChunks);
        return;
    }

    FWorld_TerrainChunk NewChunk;
    NewChunk.ChunkLocation = ChunkCenter;
    NewChunk.TerrainType = TerrainType;

    // Get biome settings
    const FWorld_BiomeSettings* BiomeSettings = BiomeSettingsMap.Find(TerrainType);
    if (!BiomeSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("No biome settings found for terrain type"));
        return;
    }

    // Calculate height variation
    NewChunk.HeightVariation = FMath::RandRange(BiomeSettings->MinHeight, BiomeSettings->MaxHeight);

    // Set vegetation density
    NewChunk.VegetationDensity = BiomeSettings->VegetationCount;

    // Create biome area
    CreateBiomeArea(ChunkCenter, *BiomeSettings);

    // Add to generated chunks
    GeneratedChunks.Add(NewChunk);
    CurrentChunkCount++;

    UE_LOG(LogTemp, Log, TEXT("Generated terrain chunk at %s with type %d"), 
           *ChunkCenter.ToString(), (int32)TerrainType);
}

void UWorld_TerrainGenerator::CreateBiomeArea(const FVector& Center, const FWorld_BiomeSettings& BiomeSettings)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found"));
        return;
    }

    // Spawn vegetation based on biome type
    switch (BiomeSettings.BiomeType)
    {
        case EWorld_TerrainType::Forest:
            SpawnVegetationCluster(Center, BiomeSettings.VegetationCount, BiomeSettings.SpawnRadius);
            break;

        case EWorld_TerrainType::Mountain:
            GenerateRockFormations(Center, BiomeSettings.VegetationCount / 2, BiomeSettings.MaxHeight);
            break;

        case EWorld_TerrainType::River:
            CreateWaterBody(Center + FVector(-500, 0, 0), Center + FVector(500, 0, 0), 200.0f);
            break;

        case EWorld_TerrainType::Swamp:
            SpawnVegetationCluster(Center, BiomeSettings.VegetationCount, BiomeSettings.SpawnRadius * 0.7f);
            CreateWaterBody(Center + FVector(-200, -200, -20), Center + FVector(200, 200, -20), 100.0f);
            break;

        case EWorld_TerrainType::Plains:
            SpawnVegetationCluster(Center, BiomeSettings.VegetationCount / 3, BiomeSettings.SpawnRadius * 1.5f);
            break;

        case EWorld_TerrainType::Desert:
            GenerateRockFormations(Center, BiomeSettings.VegetationCount / 4, BiomeSettings.MaxHeight * 0.5f);
            break;
    }
}

void UWorld_TerrainGenerator::SpawnVegetationCluster(const FVector& Center, int32 Count, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (int32 i = 0; i < Count; i++)
    {
        // Calculate random position within radius
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(0.0f, Radius);
        
        FVector SpawnLocation = Center + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );

        // Add height variation
        SpawnLocation.Z += CalculateNoiseOffset(SpawnLocation).Z;

        // Spawn vegetation actor
        AActor* VegetationActor = SpawnTerrainActor(
            AStaticMeshActor::StaticClass(),
            SpawnLocation,
            FRotator(0, FMath::RandRange(0.0f, 360.0f), 0)
        );

        if (VegetationActor)
        {
            VegetationActor->SetActorLabel(FString::Printf(TEXT("Vegetation_%d"), i));
            
            // Random scale variation
            float Scale = FMath::RandRange(0.8f, 1.2f);
            VegetationActor->SetActorScale3D(FVector(Scale, Scale, Scale));

            SpawnedTerrainActors.Add(VegetationActor);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned %d vegetation actors at %s"), Count, *Center.ToString());
}

void UWorld_TerrainGenerator::CreateWaterBody(const FVector& Start, const FVector& End, float Width)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Calculate water segments
    FVector Direction = (End - Start).GetSafeNormal();
    float Distance = FVector::Dist(Start, End);
    int32 Segments = FMath::Max(1, FMath::FloorToInt(Distance / 200.0f));

    for (int32 i = 0; i < Segments; i++)
    {
        float Alpha = (float)i / (float)(Segments - 1);
        FVector SegmentLocation = FMath::Lerp(Start, End, Alpha);

        // Spawn water segment
        AActor* WaterActor = SpawnTerrainActor(
            AStaticMeshActor::StaticClass(),
            SegmentLocation,
            FRotator::ZeroRotator
        );

        if (WaterActor)
        {
            WaterActor->SetActorLabel(FString::Printf(TEXT("WaterSegment_%d"), i));
            
            // Scale to create water surface
            FVector WaterScale = FVector(Width / 100.0f, 2.0f, 0.1f);
            WaterActor->SetActorScale3D(WaterScale);

            SpawnedTerrainActors.Add(WaterActor);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Created water body from %s to %s with %d segments"), 
           *Start.ToString(), *End.ToString(), Segments);
}

void UWorld_TerrainGenerator::GenerateRockFormations(const FVector& Center, int32 Count, float HeightVariation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (int32 i = 0; i < Count; i++)
    {
        // Calculate random position in cluster
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(100.0f, 500.0f);
        
        FVector RockLocation = Center + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::RandRange(0.0f, HeightVariation)
        );

        // Spawn rock actor
        AActor* RockActor = SpawnTerrainActor(
            AStaticMeshActor::StaticClass(),
            RockLocation,
            FRotator(0, FMath::RandRange(0.0f, 360.0f), 0)
        );

        if (RockActor)
        {
            RockActor->SetActorLabel(FString::Printf(TEXT("Rock_%d"), i));
            
            // Random scale for variety
            float Scale = FMath::RandRange(1.0f, 2.5f);
            RockActor->SetActorScale3D(FVector(Scale, Scale, Scale));

            SpawnedTerrainActors.Add(RockActor);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Generated %d rock formations at %s"), Count, *Center.ToString());
}

void UWorld_TerrainGenerator::RegenerateAllTerrain()
{
    // Clear existing terrain
    ClearGeneratedTerrain();

    // Generate default biome areas
    TArray<FVector> BiomeCenters = {
        FVector(2000, 0, 100),      // Forest
        FVector(-2000, 1500, 200),  // Mountain
        FVector(0, -1000, 50),      // River
        FVector(-1000, -2000, 30),  // Swamp
        FVector(0, 2000, 80)        // Plains
    };

    TArray<EWorld_TerrainType> BiomeTypes = {
        EWorld_TerrainType::Forest,
        EWorld_TerrainType::Mountain,
        EWorld_TerrainType::River,
        EWorld_TerrainType::Swamp,
        EWorld_TerrainType::Plains
    };

    for (int32 i = 0; i < BiomeCenters.Num() && i < BiomeTypes.Num(); i++)
    {
        GenerateTerrainChunk(BiomeCenters[i], BiomeTypes[i]);
    }

    UE_LOG(LogTemp, Log, TEXT("Regenerated all terrain with %d biome areas"), BiomeCenters.Num());
}

void UWorld_TerrainGenerator::ClearGeneratedTerrain()
{
    CleanupTerrainActors();
    GeneratedChunks.Empty();
    CurrentChunkCount = 0;

    UE_LOG(LogTemp, Log, TEXT("Cleared all generated terrain"));
}

float UWorld_TerrainGenerator::GetHeightAtLocation(const FVector& Location) const
{
    FVector NoiseOffset = CalculateNoiseOffset(Location);
    return NoiseOffset.Z;
}

EWorld_TerrainType UWorld_TerrainGenerator::GetTerrainTypeAtLocation(const FVector& Location) const
{
    // Find closest chunk
    float ClosestDistance = MAX_FLT;
    EWorld_TerrainType ClosestType = EWorld_TerrainType::Plains;

    for (const FWorld_TerrainChunk& Chunk : GeneratedChunks)
    {
        float Distance = FVector::Dist(Location, Chunk.ChunkLocation);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestType = Chunk.TerrainType;
        }
    }

    return ClosestType;
}

void UWorld_TerrainGenerator::SetBiomeSettings(EWorld_TerrainType BiomeType, const FWorld_BiomeSettings& Settings)
{
    BiomeSettingsMap.Add(BiomeType, Settings);
    UE_LOG(LogTemp, Log, TEXT("Updated biome settings for type %d"), (int32)BiomeType);
}

void UWorld_TerrainGenerator::InitializeBiomeSettings()
{
    // Forest biome
    FWorld_BiomeSettings ForestSettings;
    ForestSettings.BiomeType = EWorld_TerrainType::Forest;
    ForestSettings.BiomeColor = FLinearColor::Green;
    ForestSettings.MinHeight = 50.0f;
    ForestSettings.MaxHeight = 150.0f;
    ForestSettings.VegetationCount = 12;
    ForestSettings.SpawnRadius = 600.0f;
    BiomeSettingsMap.Add(EWorld_TerrainType::Forest, ForestSettings);

    // Mountain biome
    FWorld_BiomeSettings MountainSettings;
    MountainSettings.BiomeType = EWorld_TerrainType::Mountain;
    MountainSettings.BiomeColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
    MountainSettings.MinHeight = 200.0f;
    MountainSettings.MaxHeight = 500.0f;
    MountainSettings.VegetationCount = 8;
    MountainSettings.SpawnRadius = 800.0f;
    BiomeSettingsMap.Add(EWorld_TerrainType::Mountain, MountainSettings);

    // River biome
    FWorld_BiomeSettings RiverSettings;
    RiverSettings.BiomeType = EWorld_TerrainType::River;
    RiverSettings.BiomeColor = FLinearColor::Blue;
    RiverSettings.MinHeight = 0.0f;
    RiverSettings.MaxHeight = 50.0f;
    RiverSettings.VegetationCount = 6;
    RiverSettings.SpawnRadius = 400.0f;
    BiomeSettingsMap.Add(EWorld_TerrainType::River, RiverSettings);

    // Swamp biome
    FWorld_BiomeSettings SwampSettings;
    SwampSettings.BiomeType = EWorld_TerrainType::Swamp;
    SwampSettings.BiomeColor = FLinearColor(0.3f, 0.5f, 0.2f, 1.0f);
    SwampSettings.MinHeight = 0.0f;
    SwampSettings.MaxHeight = 80.0f;
    SwampSettings.VegetationCount = 10;
    SwampSettings.SpawnRadius = 500.0f;
    BiomeSettingsMap.Add(EWorld_TerrainType::Swamp, SwampSettings);

    // Plains biome
    FWorld_BiomeSettings PlainsSettings;
    PlainsSettings.BiomeType = EWorld_TerrainType::Plains;
    PlainsSettings.BiomeColor = FLinearColor(0.6f, 0.8f, 0.3f, 1.0f);
    PlainsSettings.MinHeight = 30.0f;
    PlainsSettings.MaxHeight = 100.0f;
    PlainsSettings.VegetationCount = 5;
    PlainsSettings.SpawnRadius = 1200.0f;
    BiomeSettingsMap.Add(EWorld_TerrainType::Plains, PlainsSettings);

    // Desert biome
    FWorld_BiomeSettings DesertSettings;
    DesertSettings.BiomeType = EWorld_TerrainType::Desert;
    DesertSettings.BiomeColor = FLinearColor(0.9f, 0.7f, 0.3f, 1.0f);
    DesertSettings.MinHeight = 0.0f;
    DesertSettings.MaxHeight = 200.0f;
    DesertSettings.VegetationCount = 3;
    DesertSettings.SpawnRadius = 1000.0f;
    BiomeSettingsMap.Add(EWorld_TerrainType::Desert, DesertSettings);
}

FVector UWorld_TerrainGenerator::CalculateNoiseOffset(const FVector& Location) const
{
    // Simple Perlin-like noise calculation
    float NoiseX = Location.X * NoiseScale;
    float NoiseY = Location.Y * NoiseScale;
    
    float Height = FMath::Sin(NoiseX) * FMath::Cos(NoiseY) * HeightMultiplier * 0.5f;
    
    return FVector(0.0f, 0.0f, Height);
}

AActor* UWorld_TerrainGenerator::SpawnTerrainActor(UClass* ActorClass, const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World || !ActorClass)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
    
    if (SpawnedActor)
    {
        SpawnedTerrainActors.Add(SpawnedActor);
    }

    return SpawnedActor;
}

void UWorld_TerrainGenerator::CleanupTerrainActors()
{
    for (AActor* Actor : SpawnedTerrainActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    SpawnedTerrainActors.Empty();
}