#include "World_BiomeTerrainGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/AssetManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h"

UWorld_BiomeTerrainGenerator::UWorld_BiomeTerrainGenerator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    RandomSeed = 12345;
    bAutoGenerateOnBeginPlay = false;
    MinDistanceBetweenFeatures = 500.0f;
    TerrainVariationScale = 1.0f;
    
    SetupDefaultBiomes();
}

void UWorld_BiomeTerrainGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoGenerateOnBeginPlay)
    {
        InitializeBiomes();
        GenerateAllBiomeTerrain();
    }
}

void UWorld_BiomeTerrainGenerator::SetupDefaultBiomes()
{
    BiomeConfigurations.Empty();
    
    // Savana biome
    FWorld_BiomeConfiguration SavanaBiome;
    SavanaBiome.BiomeName = TEXT("Savana");
    SavanaBiome.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    SavanaBiome.BiomeRadius = 15000.0f;
    SavanaBiome.VegetationDensity = 30;
    SavanaBiome.TerrainFeatureDensity = 20;
    SavanaBiome.VegetationAssetPaths.Add(TEXT("/Game/LandscapePackOne/"));
    SavanaBiome.TerrainFeatureAssetPaths.Add(TEXT("/Game/LandscapePackOne/"));
    BiomeConfigurations.Add(SavanaBiome);
    
    // Pantano biome
    FWorld_BiomeConfiguration PantanoBiome;
    PantanoBiome.BiomeName = TEXT("Pantano");
    PantanoBiome.CenterLocation = FVector(-50000.0f, -45000.0f, 50.0f);
    PantanoBiome.BiomeRadius = 12000.0f;
    PantanoBiome.VegetationDensity = 60;
    PantanoBiome.TerrainFeatureDensity = 15;
    PantanoBiome.VegetationAssetPaths.Add(TEXT("/Game/Tropical_Jungle_Pack/"));
    PantanoBiome.TerrainFeatureAssetPaths.Add(TEXT("/Game/WaterPlane/"));
    BiomeConfigurations.Add(PantanoBiome);
    
    // Floresta biome
    FWorld_BiomeConfiguration FlorestaBiome;
    FlorestaBiome.BiomeName = TEXT("Floresta");
    FlorestaBiome.CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    FlorestaBiome.BiomeRadius = 18000.0f;
    FlorestaBiome.VegetationDensity = 80;
    FlorestaBiome.TerrainFeatureDensity = 25;
    FlorestaBiome.VegetationAssetPaths.Add(TEXT("/Game/Tropical_Jungle_Pack/"));
    FlorestaBiome.TerrainFeatureAssetPaths.Add(TEXT("/Game/LandscapePackOne/"));
    BiomeConfigurations.Add(FlorestaBiome);
    
    // Deserto biome
    FWorld_BiomeConfiguration DesertoBiome;
    DesertoBiome.BiomeName = TEXT("Deserto");
    DesertoBiome.CenterLocation = FVector(55000.0f, 0.0f, 100.0f);
    DesertoBiome.BiomeRadius = 14000.0f;
    DesertoBiome.VegetationDensity = 10;
    DesertoBiome.TerrainFeatureDensity = 35;
    DesertoBiome.VegetationAssetPaths.Add(TEXT("/Game/Desert_Oasis/"));
    DesertoBiome.TerrainFeatureAssetPaths.Add(TEXT("/Game/Desert_Oasis/"));
    BiomeConfigurations.Add(DesertoBiome);
    
    // Montanha biome
    FWorld_BiomeConfiguration MontanhaBiome;
    MontanhaBiome.BiomeName = TEXT("Montanha");
    MontanhaBiome.CenterLocation = FVector(40000.0f, 50000.0f, 200.0f);
    MontanhaBiome.BiomeRadius = 16000.0f;
    MontanhaBiome.VegetationDensity = 20;
    MontanhaBiome.TerrainFeatureDensity = 50;
    MontanhaBiome.VegetationAssetPaths.Add(TEXT("/Game/ANGRY_MESH/"));
    MontanhaBiome.TerrainFeatureAssetPaths.Add(TEXT("/Game/ANGRY_MESH/"));
    BiomeConfigurations.Add(MontanhaBiome);
}

void UWorld_BiomeTerrainGenerator::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeTerrainGenerator: Initializing biomes with seed %d"), RandomSeed);
    FMath::RandInit(RandomSeed);
    
    GeneratedTerrainFeatures.Empty();
}

void UWorld_BiomeTerrainGenerator::GenerateTerrainForBiome(const FWorld_BiomeConfiguration& BiomeConfig)
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeTerrainGenerator: Generating terrain for biome %s"), *BiomeConfig.BiomeName);
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World_BiomeTerrainGenerator: No valid world found"));
        return;
    }
    
    // Generate vegetation
    for (int32 i = 0; i < BiomeConfig.VegetationDensity; ++i)
    {
        FVector Location = GetRandomLocationInBiome(BiomeConfig);
        
        if (IsLocationValidForFeature(Location, MinDistanceBetweenFeatures))
        {
            // Apply terrain height variation using Perlin noise
            float HeightVariation = GetPerlinNoise(Location.X, Location.Y, 0.001f) * TerrainVariationScale * 200.0f;
            Location.Z += HeightVariation;
            
            FRotator Rotation = GetRandomRotationVariation();
            FVector Scale = GetRandomScaleVariation();
            
            // Create terrain feature record
            FWorld_TerrainFeature Feature;
            Feature.FeatureName = FString::Printf(TEXT("%s_Vegetation_%d"), *BiomeConfig.BiomeName, i);
            Feature.Location = Location;
            Feature.Rotation = Rotation;
            Feature.Scale = Scale;
            Feature.AssetPath = BiomeConfig.VegetationAssetPaths.Num() > 0 ? BiomeConfig.VegetationAssetPaths[0] : TEXT("");
            
            GeneratedTerrainFeatures.Add(Feature);
        }
    }
    
    // Generate terrain features
    for (int32 i = 0; i < BiomeConfig.TerrainFeatureDensity; ++i)
    {
        FVector Location = GetRandomLocationInBiome(BiomeConfig);
        
        if (IsLocationValidForFeature(Location, MinDistanceBetweenFeatures * 1.5f))
        {
            // Apply terrain height variation
            float HeightVariation = GetPerlinNoise(Location.X, Location.Y, 0.0005f) * TerrainVariationScale * 300.0f;
            Location.Z += HeightVariation;
            
            FRotator Rotation = GetRandomRotationVariation();
            FVector Scale = GetRandomScaleVariation();
            
            // Create terrain feature record
            FWorld_TerrainFeature Feature;
            Feature.FeatureName = FString::Printf(TEXT("%s_Terrain_%d"), *BiomeConfig.BiomeName, i);
            Feature.Location = Location;
            Feature.Rotation = Rotation;
            Feature.Scale = Scale;
            Feature.AssetPath = BiomeConfig.TerrainFeatureAssetPaths.Num() > 0 ? BiomeConfig.TerrainFeatureAssetPaths[0] : TEXT("");
            
            GeneratedTerrainFeatures.Add(Feature);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeTerrainGenerator: Generated %d terrain features for %s"), 
           BiomeConfig.VegetationDensity + BiomeConfig.TerrainFeatureDensity, *BiomeConfig.BiomeName);
}

void UWorld_BiomeTerrainGenerator::GenerateAllBiomeTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeTerrainGenerator: Starting full biome terrain generation"));
    
    InitializeBiomes();
    
    for (const FWorld_BiomeConfiguration& BiomeConfig : BiomeConfigurations)
    {
        GenerateTerrainForBiome(BiomeConfig);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeTerrainGenerator: Completed terrain generation. Total features: %d"), GeneratedTerrainFeatures.Num());
}

void UWorld_BiomeTerrainGenerator::ClearGeneratedTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeTerrainGenerator: Clearing generated terrain"));
    GeneratedTerrainFeatures.Empty();
}

FVector UWorld_BiomeTerrainGenerator::GetRandomLocationInBiome(const FWorld_BiomeConfiguration& BiomeConfig)
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, BiomeConfig.BiomeRadius);
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomRadius;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomRadius;
    RandomOffset.Z = FMath::RandRange(-50.0f, 50.0f);
    
    return BiomeConfig.CenterLocation + RandomOffset;
}

bool UWorld_BiomeTerrainGenerator::IsLocationValidForFeature(const FVector& Location, float MinDistance)
{
    for (const FWorld_TerrainFeature& Feature : GeneratedTerrainFeatures)
    {
        if (FVector::Dist(Location, Feature.Location) < MinDistance)
        {
            return false;
        }
    }
    return true;
}

AStaticMeshActor* UWorld_BiomeTerrainGenerator::SpawnTerrainFeature(const FString& AssetPath, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* SpawnedActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
    
    if (SpawnedActor)
    {
        SpawnedActor->SetActorScale3D(Scale);
        UE_LOG(LogTemp, Log, TEXT("World_BiomeTerrainGenerator: Spawned terrain feature at %s"), *Location.ToString());
    }
    
    return SpawnedActor;
}

void UWorld_BiomeTerrainGenerator::SetBiomeConfiguration(const TArray<FWorld_BiomeConfiguration>& NewConfigurations)
{
    BiomeConfigurations = NewConfigurations;
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeTerrainGenerator: Updated biome configurations. Count: %d"), BiomeConfigurations.Num());
}

TArray<FWorld_BiomeConfiguration> UWorld_BiomeTerrainGenerator::GetBiomeConfigurations() const
{
    return BiomeConfigurations;
}

int32 UWorld_BiomeTerrainGenerator::GetTerrainFeatureCount() const
{
    return GeneratedTerrainFeatures.Num();
}

void UWorld_BiomeTerrainGenerator::EditorGenerateTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeTerrainGenerator: Editor terrain generation triggered"));
    GenerateAllBiomeTerrain();
}

void UWorld_BiomeTerrainGenerator::EditorClearTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeTerrainGenerator: Editor terrain clearing triggered"));
    ClearGeneratedTerrain();
}

float UWorld_BiomeTerrainGenerator::GetPerlinNoise(float X, float Y, float Scale) const
{
    // Simple Perlin noise implementation for terrain variation
    float ScaledX = X * Scale;
    float ScaledY = Y * Scale;
    
    // Use UE5's built-in noise functions
    return FMath::PerlinNoise2D(FVector2D(ScaledX, ScaledY));
}

FRotator UWorld_BiomeTerrainGenerator::GetRandomRotationVariation() const
{
    return FRotator(
        FMath::RandRange(-15.0f, 15.0f),  // Pitch
        FMath::RandRange(0.0f, 360.0f),   // Yaw
        FMath::RandRange(-10.0f, 10.0f)   // Roll
    );
}

FVector UWorld_BiomeTerrainGenerator::GetRandomScaleVariation() const
{
    float BaseScale = FMath::RandRange(0.8f, 1.4f);
    return FVector(
        BaseScale * FMath::RandRange(0.9f, 1.1f),
        BaseScale * FMath::RandRange(0.9f, 1.1f),
        BaseScale * FMath::RandRange(0.8f, 1.2f)
    );
}