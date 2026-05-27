#include "World_ProceduralBiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWorld_ProceduralBiomeManager::AWorld_ProceduralBiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Performance settings
    PerformanceLODDistance = 40000.0f;
    MaxFeaturesPerFrame = 5;
    LastOptimizationTime = 0.0f;
    CurrentFeatureIndex = 0;

    // Initialize default biome configurations
    BiomeConfigurations.Empty();
    
    // Savana biome
    FWorld_BiomeConfiguration SavanaBiome;
    SavanaBiome.BiomeName = TEXT("Savana");
    SavanaBiome.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    SavanaBiome.Radius = 15000.0f;
    SavanaBiome.TerrainFeatureCount = 8;
    SavanaBiome.VegetationDensity = 0.3f;
    SavanaBiome.DinosaurSpecies.Add(TEXT("TRex"));
    SavanaBiome.DinosaurSpecies.Add(TEXT("Velociraptor"));
    SavanaBiome.DinosaurSpecies.Add(TEXT("Triceratops"));
    BiomeConfigurations.Add(SavanaBiome);

    // Floresta biome
    FWorld_BiomeConfiguration FlorestaBiome;
    FlorestaBiome.BiomeName = TEXT("Floresta");
    FlorestaBiome.CenterLocation = FVector(-45000.0f, 40000.0f, 150.0f);
    FlorestaBiome.Radius = 20000.0f;
    FlorestaBiome.TerrainFeatureCount = 15;
    FlorestaBiome.VegetationDensity = 0.8f;
    FlorestaBiome.DinosaurSpecies.Add(TEXT("Brachiosaurus"));
    FlorestaBiome.DinosaurSpecies.Add(TEXT("Parasaurolophus"));
    BiomeConfigurations.Add(FlorestaBiome);

    // Deserto biome
    FWorld_BiomeConfiguration DesertoBiome;
    DesertoBiome.BiomeName = TEXT("Deserto");
    DesertoBiome.CenterLocation = FVector(50000.0f, -40000.0f, 120.0f);
    DesertoBiome.Radius = 18000.0f;
    DesertoBiome.TerrainFeatureCount = 6;
    DesertoBiome.VegetationDensity = 0.1f;
    DesertoBiome.DinosaurSpecies.Add(TEXT("Ankylosaurus"));
    DesertoBiome.DinosaurSpecies.Add(TEXT("Protoceratops"));
    BiomeConfigurations.Add(DesertoBiome);

    // Pantano biome
    FWorld_BiomeConfiguration PantanoBiome;
    PantanoBiome.BiomeName = TEXT("Pantano");
    PantanoBiome.CenterLocation = FVector(-30000.0f, -30000.0f, 80.0f);
    PantanoBiome.Radius = 12000.0f;
    PantanoBiome.TerrainFeatureCount = 10;
    PantanoBiome.VegetationDensity = 0.6f;
    PantanoBiome.DinosaurSpecies.Add(TEXT("Tsintaosaurus"));
    BiomeConfigurations.Add(PantanoBiome);

    // Montanha biome
    FWorld_BiomeConfiguration MontanhaBiome;
    MontanhaBiome.BiomeName = TEXT("Montanha");
    MontanhaBiome.CenterLocation = FVector(30000.0f, 30000.0f, 300.0f);
    MontanhaBiome.Radius = 16000.0f;
    MontanhaBiome.TerrainFeatureCount = 12;
    MontanhaBiome.VegetationDensity = 0.4f;
    MontanhaBiome.DinosaurSpecies.Add(TEXT("Pachycephalo"));
    BiomeConfigurations.Add(MontanhaBiome);
}

void AWorld_ProceduralBiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeManager: BeginPlay - Initializing biome system"));
    InitializeBiomes();
}

void AWorld_ProceduralBiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Performance optimization every 2 seconds
    LastOptimizationTime += DeltaTime;
    if (LastOptimizationTime >= 2.0f)
    {
        OptimizeBiomePerformance();
        LastOptimizationTime = 0.0f;
    }
}

void AWorld_ProceduralBiomeManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeManager: Initializing %d biomes"), BiomeConfigurations.Num());
    
    for (const FWorld_BiomeConfiguration& BiomeConfig : BiomeConfigurations)
    {
        CreateBiomeMarker(BiomeConfig);
        GenerateTerrainFeatures(BiomeConfig);
        PopulateBiomeWithDinosaurs(BiomeConfig);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeManager: Biome initialization complete"));
}

void AWorld_ProceduralBiomeManager::GenerateTerrainFeatures(const FWorld_BiomeConfiguration& BiomeConfig)
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeManager: Generating %d terrain features for %s"), 
           BiomeConfig.TerrainFeatureCount, *BiomeConfig.BiomeName);
    
    for (int32 i = 0; i < BiomeConfig.TerrainFeatureCount; i++)
    {
        // Random location within biome radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(BiomeConfig.Radius * 0.3f, BiomeConfig.Radius * 0.9f);
        
        FVector FeatureLocation = BiomeConfig.CenterLocation;
        FeatureLocation.X += FMath::Cos(Angle) * Distance;
        FeatureLocation.Y += FMath::Sin(Angle) * Distance;
        FeatureLocation.Z += FMath::RandRange(-50.0f, 100.0f);
        
        // Determine feature type based on biome
        FString FeatureType;
        if (BiomeConfig.BiomeName == TEXT("Floresta") || BiomeConfig.BiomeName == TEXT("Pantano"))
        {
            FeatureType = TEXT("Tree");
        }
        else if (BiomeConfig.BiomeName == TEXT("Deserto") || BiomeConfig.BiomeName == TEXT("Montanha"))
        {
            FeatureType = TEXT("Rock");
        }
        else
        {
            FeatureType = FMath::RandBool() ? TEXT("Tree") : TEXT("Rock");
        }
        
        // Calculate LOD level based on distance from origin
        float DistanceFromOrigin = FVector::Dist(FeatureLocation, FVector::ZeroVector);
        int32 LODLevel = (DistanceFromOrigin > PerformanceLODDistance) ? 2 : 0;
        
        float Scale = FMath::RandRange(0.8f, 1.5f);
        SpawnTerrainFeature(FeatureLocation, FeatureType, Scale, LODLevel);
        
        // Store feature data
        FWorld_TerrainFeature Feature;
        Feature.Location = FeatureLocation;
        Feature.FeatureType = FeatureType;
        Feature.Scale = Scale;
        Feature.LODLevel = LODLevel;
        GeneratedFeatures.Add(Feature);
    }
}

void AWorld_ProceduralBiomeManager::PopulateBiomeWithDinosaurs(const FWorld_BiomeConfiguration& BiomeConfig)
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeManager: Populating %s with %d dinosaur species"), 
           *BiomeConfig.BiomeName, BiomeConfig.DinosaurSpecies.Num());
    
    for (const FString& DinosaurSpecies : BiomeConfig.DinosaurSpecies)
    {
        // Spawn 1-3 dinosaurs of each species in the biome
        int32 DinosaurCount = FMath::RandRange(1, 3);
        
        for (int32 i = 0; i < DinosaurCount; i++)
        {
            // Random location within biome
            float Angle = FMath::RandRange(0.0f, 2.0f * PI);
            float Distance = FMath::RandRange(BiomeConfig.Radius * 0.2f, BiomeConfig.Radius * 0.8f);
            
            FVector DinosaurLocation = BiomeConfig.CenterLocation;
            DinosaurLocation.X += FMath::Cos(Angle) * Distance;
            DinosaurLocation.Y += FMath::Sin(Angle) * Distance;
            
            AActor* DinosaurActor = SpawnDinosaurInBiome(DinosaurSpecies, DinosaurLocation);
            if (DinosaurActor)
            {
                SpawnedDinosaurActors.Add(DinosaurActor);
            }
        }
    }
}

void AWorld_ProceduralBiomeManager::OptimizeBiomePerformance()
{
    // Optimize a subset of features each frame to avoid performance spikes
    int32 FeaturesProcessed = 0;
    int32 TotalFeatures = SpawnedFeatureActors.Num();
    
    if (TotalFeatures == 0) return;
    
    for (int32 i = 0; i < MaxFeaturesPerFrame && CurrentFeatureIndex < TotalFeatures; i++)
    {
        AActor* FeatureActor = SpawnedFeatureActors[CurrentFeatureIndex];
        if (IsValid(FeatureActor))
        {
            // Calculate distance from player or origin
            FVector ActorLocation = FeatureActor->GetActorLocation();
            float DistanceFromOrigin = FVector::Dist(ActorLocation, FVector::ZeroVector);
            
            // Apply LOD based on distance
            UStaticMeshComponent* MeshComp = FeatureActor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp)
            {
                if (DistanceFromOrigin > PerformanceLODDistance)
                {
                    MeshComp->SetForcedLodModel(2); // High LOD for distant objects
                }
                else if (DistanceFromOrigin > PerformanceLODDistance * 0.5f)
                {
                    MeshComp->SetForcedLodModel(1); // Medium LOD
                }
                else
                {
                    MeshComp->SetForcedLodModel(0); // Full detail for close objects
                }
            }
        }
        
        CurrentFeatureIndex++;
        FeaturesProcessed++;
    }
    
    // Reset index when we've processed all features
    if (CurrentFeatureIndex >= TotalFeatures)
    {
        CurrentFeatureIndex = 0;
    }
}

FWorld_BiomeConfiguration AWorld_ProceduralBiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float MinDistance = MAX_FLT;
    FWorld_BiomeConfiguration ClosestBiome;
    
    for (const FWorld_BiomeConfiguration& BiomeConfig : BiomeConfigurations)
    {
        float Distance = FVector::Dist(Location, BiomeConfig.CenterLocation);
        if (Distance < MinDistance && Distance <= BiomeConfig.Radius)
        {
            MinDistance = Distance;
            ClosestBiome = BiomeConfig;
        }
    }
    
    return ClosestBiome;
}

void AWorld_ProceduralBiomeManager::RegenerateBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeManager: Regenerating all biomes"));
    
    ClearAllBiomes();
    InitializeBiomes();
}

void AWorld_ProceduralBiomeManager::ClearAllBiomes()
{
    // Clean up spawned actors
    for (AActor* Actor : SpawnedBiomeActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    for (AActor* Actor : SpawnedFeatureActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    for (AActor* Actor : SpawnedDinosaurActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    SpawnedBiomeActors.Empty();
    SpawnedFeatureActors.Empty();
    SpawnedDinosaurActors.Empty();
    GeneratedFeatures.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeManager: All biomes cleared"));
}

void AWorld_ProceduralBiomeManager::CreateBiomeMarker(const FWorld_BiomeConfiguration& BiomeConfig)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Spawn biome center marker
    AActor* BiomeMarker = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        BiomeConfig.CenterLocation,
        FRotator::ZeroRotator
    );
    
    if (BiomeMarker)
    {
        BiomeMarker->SetActorLabel(FString::Printf(TEXT("BiomeCenter_%s"), *BiomeConfig.BiomeName));
        SpawnedBiomeActors.Add(BiomeMarker);
        
        UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeManager: Created biome marker for %s"), *BiomeConfig.BiomeName);
    }
}

void AWorld_ProceduralBiomeManager::SpawnTerrainFeature(const FVector& Location, const FString& FeatureType, float Scale, int32 LODLevel)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    AActor* FeatureActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        FRotator::ZeroRotator
    );
    
    if (FeatureActor)
    {
        FeatureActor->SetActorLabel(FString::Printf(TEXT("TerrainFeature_%s"), *FeatureType));
        FeatureActor->SetActorScale3D(FVector(Scale));
        
        // Apply LOD settings
        UStaticMeshComponent* MeshComp = FeatureActor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            MeshComp->SetForcedLodModel(LODLevel);
        }
        
        SpawnedFeatureActors.Add(FeatureActor);
    }
}

AActor* AWorld_ProceduralBiomeManager::SpawnDinosaurInBiome(const FString& DinosaurSpecies, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    
    // For now, spawn a basic actor as placeholder
    // In a full implementation, this would load the actual dinosaur mesh/blueprint
    AActor* DinosaurActor = World->SpawnActor<AActor>(
        AActor::StaticClass(),
        Location,
        FRotator::ZeroRotator
    );
    
    if (DinosaurActor)
    {
        DinosaurActor->SetActorLabel(FString::Printf(TEXT("Dinosaur_%s"), *DinosaurSpecies));
        UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeManager: Spawned %s at %s"), 
               *DinosaurSpecies, *Location.ToString());
    }
    
    return DinosaurActor;
}