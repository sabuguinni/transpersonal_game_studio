#include "World_ProceduralTerrainGenerator.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AWorld_ProceduralTerrainGenerator::AWorld_ProceduralTerrainGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MaxFeaturesPerBiome = 100;
    MinFeatureDistance = 500.0f;
    bAutoGenerateOnBeginPlay = false;
    RandomSeed = 12345;
    bIsGenerating = false;

    InitializeBiomeConfigurations();
}

void AWorld_ProceduralTerrainGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoGenerateOnBeginPlay)
    {
        GenerateAllBiomes();
    }
}

void AWorld_ProceduralTerrainGenerator::InitializeBiomeConfigurations()
{
    BiomeConfigurations.Empty();

    // Savana biome
    FWorld_BiomeConfiguration SavanaConfig;
    SavanaConfig.BiomeType = EBiomeType::Savana;
    SavanaConfig.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavanaConfig.Radius = 15000.0f;
    SavanaConfig.FeatureDensity = 75;
    BiomeConfigurations.Add(SavanaConfig);

    // Floresta biome
    FWorld_BiomeConfiguration FlorestaConfig;
    FlorestaConfig.BiomeType = EBiomeType::Floresta;
    FlorestaConfig.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaConfig.Radius = 12000.0f;
    FlorestaConfig.FeatureDensity = 100;
    BiomeConfigurations.Add(FlorestaConfig);

    // Deserto biome
    FWorld_BiomeConfiguration DesertoConfig;
    DesertoConfig.BiomeType = EBiomeType::Deserto;
    DesertoConfig.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertoConfig.Radius = 10000.0f;
    DesertoConfig.FeatureDensity = 40;
    BiomeConfigurations.Add(DesertoConfig);

    // Pantano biome
    FWorld_BiomeConfiguration PantanoConfig;
    PantanoConfig.BiomeType = EBiomeType::Pantano;
    PantanoConfig.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoConfig.Radius = 8000.0f;
    PantanoConfig.FeatureDensity = 60;
    BiomeConfigurations.Add(PantanoConfig);

    // Montanha biome
    FWorld_BiomeConfiguration MontanhaConfig;
    MontanhaConfig.BiomeType = EBiomeType::Montanha;
    MontanhaConfig.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    MontanhaConfig.Radius = 14000.0f;
    MontanhaConfig.FeatureDensity = 80;
    BiomeConfigurations.Add(MontanhaConfig);
}

void AWorld_ProceduralTerrainGenerator::GenerateAllBiomes()
{
    if (bIsGenerating)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already generating biomes, skipping"));
        return;
    }

    bIsGenerating = true;
    ClearGeneratedFeatures();

    FMath::RandInit(RandomSeed);

    for (const FWorld_BiomeConfiguration& BiomeConfig : BiomeConfigurations)
    {
        GenerateBiome(BiomeConfig.BiomeType);
    }

    bIsGenerating = false;
    UE_LOG(LogTemp, Log, TEXT("Generated %d terrain features across all biomes"), GeneratedFeatures.Num());
}

void AWorld_ProceduralTerrainGenerator::GenerateBiome(EBiomeType BiomeType)
{
    const FWorld_BiomeConfiguration* BiomeConfig = nullptr;
    for (const FWorld_BiomeConfiguration& Config : BiomeConfigurations)
    {
        if (Config.BiomeType == BiomeType)
        {
            BiomeConfig = &Config;
            break;
        }
    }

    if (!BiomeConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("Biome configuration not found for type: %d"), (int32)BiomeType);
        return;
    }

    int32 FeaturesGenerated = 0;
    int32 Attempts = 0;
    const int32 MaxAttempts = BiomeConfig->FeatureDensity * 3;

    while (FeaturesGenerated < BiomeConfig->FeatureDensity && Attempts < MaxAttempts)
    {
        FWorld_TerrainFeature NewFeature = CreateRandomFeature(*BiomeConfig);
        
        if (IsLocationValid(NewFeature.SpawnLocation, *BiomeConfig))
        {
            GeneratedFeatures.Add(NewFeature);
            SpawnTerrainFeature(NewFeature);
            FeaturesGenerated++;
        }

        Attempts++;
    }

    UE_LOG(LogTemp, Log, TEXT("Generated %d features for biome %d"), FeaturesGenerated, (int32)BiomeType);
}

FWorld_TerrainFeature AWorld_ProceduralTerrainGenerator::CreateRandomFeature(const FWorld_BiomeConfiguration& BiomeConfig)
{
    FWorld_TerrainFeature Feature;
    Feature.BiomeType = BiomeConfig.BiomeType;
    Feature.SpawnLocation = GetRandomLocationInBiome(BiomeConfig);
    Feature.SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
    
    float ScaleVariation = FMath::RandRange(0.7f, 1.5f);
    Feature.Scale = FVector(ScaleVariation);

    // Set feature name based on biome
    switch (BiomeConfig.BiomeType)
    {
        case EBiomeType::Savana:
            Feature.FeatureName = FString::Printf(TEXT("SavanaRock_%d"), FMath::RandRange(1, 5));
            break;
        case EBiomeType::Floresta:
            Feature.FeatureName = FString::Printf(TEXT("ForestTree_%d"), FMath::RandRange(1, 8));
            break;
        case EBiomeType::Deserto:
            Feature.FeatureName = FString::Printf(TEXT("DesertRock_%d"), FMath::RandRange(1, 4));
            break;
        case EBiomeType::Pantano:
            Feature.FeatureName = FString::Printf(TEXT("SwampLog_%d"), FMath::RandRange(1, 6));
            break;
        case EBiomeType::Montanha:
            Feature.FeatureName = FString::Printf(TEXT("MountainBoulder_%d"), FMath::RandRange(1, 7));
            break;
        default:
            Feature.FeatureName = TEXT("GenericFeature");
            break;
    }

    return Feature;
}

FVector AWorld_ProceduralTerrainGenerator::GetRandomLocationInBiome(const FWorld_BiomeConfiguration& BiomeConfig)
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, BiomeConfig.Radius);
    
    float X = BiomeConfig.CenterLocation.X + RandomRadius * FMath::Cos(RandomAngle);
    float Y = BiomeConfig.CenterLocation.Y + RandomRadius * FMath::Sin(RandomAngle);
    float Z = BiomeConfig.CenterLocation.Z + FMath::RandRange(0.0f, 200.0f);

    return FVector(X, Y, Z);
}

bool AWorld_ProceduralTerrainGenerator::IsLocationValid(const FVector& Location, const FWorld_BiomeConfiguration& BiomeConfig)
{
    // Check if location is within biome radius
    float DistanceFromCenter = FVector::Dist2D(Location, BiomeConfig.CenterLocation);
    if (DistanceFromCenter > BiomeConfig.Radius)
    {
        return false;
    }

    // Check minimum distance from existing features
    for (const FWorld_TerrainFeature& ExistingFeature : GeneratedFeatures)
    {
        if (ExistingFeature.BiomeType == BiomeConfig.BiomeType)
        {
            float Distance = FVector::Dist(Location, ExistingFeature.SpawnLocation);
            if (Distance < MinFeatureDistance)
            {
                return false;
            }
        }
    }

    return true;
}

void AWorld_ProceduralTerrainGenerator::SpawnTerrainFeature(const FWorld_TerrainFeature& Feature)
{
    if (!GetWorld())
    {
        return;
    }

    // Create a static mesh actor for the terrain feature
    AStaticMeshActor* NewActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Feature.SpawnLocation,
        Feature.SpawnRotation
    );

    if (NewActor)
    {
        NewActor->SetActorScale3D(Feature.Scale);
        NewActor->SetActorLabel(Feature.FeatureName);
        
        // Set tags for identification
        NewActor->Tags.Add(TEXT("ProceduralTerrain"));
        NewActor->Tags.Add(FName(*UEnum::GetValueAsString(Feature.BiomeType)));
        
        SpawnedActors.Add(NewActor);
    }
}

void AWorld_ProceduralTerrainGenerator::ClearGeneratedFeatures()
{
    // Destroy all spawned actors
    for (AActor* Actor : SpawnedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    SpawnedActors.Empty();
    GeneratedFeatures.Empty();
}

void AWorld_ProceduralTerrainGenerator::EditorGenerateAllBiomes()
{
    GenerateAllBiomes();
}

void AWorld_ProceduralTerrainGenerator::EditorClearFeatures()
{
    ClearGeneratedFeatures();
}

void AWorld_ProceduralTerrainGenerator::LoadBiomeAssets()
{
    // This method can be expanded to load specific meshes for each biome
    // Currently handled by the feature creation system
}