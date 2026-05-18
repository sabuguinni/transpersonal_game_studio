// Copyright Transpersonal Game Studio

#include "World_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h"

AWorld_BiomeManager::AWorld_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxFeaturesPerBiome = 50;
    FeatureSpawnRadius = 20000.0f;
    bAutoGenerateOnBeginPlay = false;
    
    SetupDefaultBiomes();
}

void AWorld_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoGenerateOnBeginPlay)
    {
        InitializeBiomes();
        GenerateTerrainFeatures();
    }
}

void AWorld_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorld_BiomeManager::SetupDefaultBiomes()
{
    BiomeDefinitions.Empty();
    
    // Savana Biome
    FWorld_BiomeData Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    Savana.Radius = 20000.0f;
    Savana.TerrainHeight = 100.0f;
    Savana.VegetationDensity = 0.3f;
    Savana.Temperature = 28.0f;
    Savana.Humidity = 0.4f;
    Savana.DinosaurSpecies.Add(TEXT("Trex"));
    Savana.DinosaurSpecies.Add(TEXT("Velociraptor"));
    Savana.DinosaurSpecies.Add(TEXT("Triceratops"));
    BiomeDefinitions.Add(Savana);
    
    // Floresta Biome
    FWorld_BiomeData Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.CenterLocation = FVector(-45000.0f, 40000.0f, 200.0f);
    Floresta.Radius = 18000.0f;
    Floresta.TerrainHeight = 200.0f;
    Floresta.VegetationDensity = 0.8f;
    Floresta.Temperature = 24.0f;
    Floresta.Humidity = 0.9f;
    Floresta.DinosaurSpecies.Add(TEXT("Brachiosaurus"));
    Floresta.DinosaurSpecies.Add(TEXT("Parasaurolophus"));
    BiomeDefinitions.Add(Floresta);
    
    // Pantano Biome
    FWorld_BiomeData Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.CenterLocation = FVector(-50000.0f, -45000.0f, 80.0f);
    Pantano.Radius = 15000.0f;
    Pantano.TerrainHeight = 80.0f;
    Pantano.VegetationDensity = 0.7f;
    Pantano.Temperature = 26.0f;
    Pantano.Humidity = 1.0f;
    Pantano.DinosaurSpecies.Add(TEXT("Tsintaosaurus"));
    BiomeDefinitions.Add(Pantano);
    
    // Deserto Biome
    FWorld_BiomeData Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.CenterLocation = FVector(55000.0f, 0.0f, 50.0f);
    Deserto.Radius = 22000.0f;
    Deserto.TerrainHeight = 50.0f;
    Deserto.VegetationDensity = 0.1f;
    Deserto.Temperature = 35.0f;
    Deserto.Humidity = 0.2f;
    Deserto.DinosaurSpecies.Add(TEXT("Ankylosaurus"));
    Deserto.DinosaurSpecies.Add(TEXT("Protoceratops"));
    BiomeDefinitions.Add(Deserto);
    
    // Montanha Biome
    FWorld_BiomeData Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.CenterLocation = FVector(40000.0f, 50000.0f, 400.0f);
    Montanha.Radius = 16000.0f;
    Montanha.TerrainHeight = 400.0f;
    Montanha.VegetationDensity = 0.2f;
    Montanha.Temperature = 18.0f;
    Montanha.Humidity = 0.6f;
    Montanha.DinosaurSpecies.Add(TEXT("Pachycephalo"));
    BiomeDefinitions.Add(Montanha);
}

void AWorld_BiomeManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initializing %d biomes"), BiomeDefinitions.Num());
    
    for (const FWorld_BiomeData& Biome : BiomeDefinitions)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized biome %s at location (%f, %f, %f)"),
            *Biome.BiomeName,
            Biome.CenterLocation.X,
            Biome.CenterLocation.Y,
            Biome.CenterLocation.Z);
    }
}

FWorld_BiomeData AWorld_BiomeManager::GetBiomeAtLocation(FVector Location)
{
    FWorld_BiomeData ClosestBiome;
    float ClosestDistance = FLT_MAX;
    
    for (const FWorld_BiomeData& Biome : BiomeDefinitions)
    {
        float Distance = FVector::Dist(Location, Biome.CenterLocation);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Biome;
        }
    }
    
    return ClosestBiome;
}

TArray<FWorld_BiomeData> AWorld_BiomeManager::GetAllBiomes()
{
    return BiomeDefinitions;
}

void AWorld_BiomeManager::GenerateTerrainFeatures()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Generating terrain features"));
    
    ClearAllTerrainFeatures();
    TerrainFeatures.Empty();
    
    for (const FWorld_BiomeData& Biome : BiomeDefinitions)
    {
        int32 FeatureCount = FMath::RandRange(MaxFeaturesPerBiome / 2, MaxFeaturesPerBiome);
        
        for (int32 i = 0; i < FeatureCount; i++)
        {
            FWorld_TerrainFeature Feature;
            Feature.FeatureName = FString::Printf(TEXT("%s_Feature_%d"), *Biome.BiomeName, i + 1);
            Feature.Location = GetRandomLocationInBiome(Biome);
            Feature.Rotation = GetRandomRotation();
            Feature.Scale = GetRandomScale(1.0f, 5.0f);
            
            // Determine feature type based on biome
            if (Biome.BiomeName == TEXT("Deserto"))
            {
                Feature.FeatureType = TEXT("DesertRock");
            }
            else if (Biome.BiomeName == TEXT("Floresta"))
            {
                Feature.FeatureType = TEXT("TreeStump");
            }
            else if (Biome.BiomeName == TEXT("Montanha"))
            {
                Feature.FeatureType = TEXT("Boulder");
            }
            else if (Biome.BiomeName == TEXT("Pantano"))
            {
                Feature.FeatureType = TEXT("SwampLog");
            }
            else
            {
                Feature.FeatureType = TEXT("Rock");
            }
            
            TerrainFeatures.Add(Feature);
            SpawnTerrainFeature(Feature);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Generated %d features for biome %s"), FeatureCount, *Biome.BiomeName);
    }
}

void AWorld_BiomeManager::SpawnTerrainFeature(const FWorld_TerrainFeature& Feature)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn a StaticMeshActor as terrain feature
    AStaticMeshActor* FeatureActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Feature.Location,
        Feature.Rotation
    );
    
    if (FeatureActor)
    {
        FeatureActor->SetActorScale3D(Feature.Scale);
        FeatureActor->SetActorLabel(Feature.FeatureName);
        
        // Set the mesh component properties
        UStaticMeshComponent* MeshComp = FeatureActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Use default cube mesh as placeholder
            // In a real implementation, this would load specific meshes based on FeatureType
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        }
        
        SpawnedFeatureActors.Add(FeatureActor);
    }
}

float AWorld_BiomeManager::GetBiomeInfluenceAtLocation(FVector Location, const FWorld_BiomeData& Biome)
{
    float Distance = FVector::Dist(Location, Biome.CenterLocation);
    float NormalizedDistance = FMath::Clamp(Distance / Biome.Radius, 0.0f, 1.0f);
    
    // Inverse distance influence (closer = higher influence)
    return 1.0f - NormalizedDistance;
}

void AWorld_BiomeManager::ClearAllTerrainFeatures()
{
    for (AActor* Actor : SpawnedFeatureActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedFeatureActors.Empty();
}

void AWorld_BiomeManager::EditorGenerateBiomes()
{
    InitializeBiomes();
    GenerateTerrainFeatures();
}

void AWorld_BiomeManager::EditorClearBiomes()
{
    ClearAllTerrainFeatures();
    TerrainFeatures.Empty();
}

FVector AWorld_BiomeManager::GetRandomLocationInBiome(const FWorld_BiomeData& Biome)
{
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::RandRange(1000.0f, Biome.Radius * 0.8f);
    
    float X = Biome.CenterLocation.X + Distance * FMath::Cos(Angle);
    float Y = Biome.CenterLocation.Y + Distance * FMath::Sin(Angle);
    float Z = Biome.TerrainHeight + FMath::RandRange(0.0f, 50.0f);
    
    return FVector(X, Y, Z);
}

FRotator AWorld_BiomeManager::GetRandomRotation()
{
    return FRotator(
        FMath::RandRange(-10.0f, 10.0f),  // Pitch
        FMath::RandRange(0.0f, 360.0f),   // Yaw
        FMath::RandRange(-5.0f, 5.0f)     // Roll
    );
}

FVector AWorld_BiomeManager::GetRandomScale(float MinScale, float MaxScale)
{
    float BaseScale = FMath::RandRange(MinScale, MaxScale);
    return FVector(
        BaseScale * FMath::RandRange(0.8f, 1.2f),
        BaseScale * FMath::RandRange(0.8f, 1.2f),
        BaseScale * FMath::RandRange(0.6f, 1.0f)
    );
}