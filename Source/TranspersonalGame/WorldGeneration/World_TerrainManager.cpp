#include "World_TerrainManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWorld_TerrainManager::AWorld_TerrainManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    TerrainSeed = 12345;
    NoiseScale = 0.01f;
    HeightMultiplier = 1000.0f;
    TerrainSize = 100000; // 1km x 1km per biome

    // Initialize biome feature arrays with default features
    SavannaFeatures.Add(FWorld_TerrainFeature());
    ForestFeatures.Add(FWorld_TerrainFeature());
    DesertFeatures.Add(FWorld_TerrainFeature());
    SwampFeatures.Add(FWorld_TerrainFeature());
    MountainFeatures.Add(FWorld_TerrainFeature());
}

void AWorld_TerrainManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_TerrainManager initialized with seed %d"), TerrainSeed);
}

void AWorld_TerrainManager::GenerateTerrainForBiome(EBiomeType BiomeType, FVector BiomeCenter)
{
    UE_LOG(LogTemp, Warning, TEXT("Generating terrain for biome %d at location %s"), 
           (int32)BiomeType, *BiomeCenter.ToString());

    // Get biome-specific features
    TArray<FWorld_TerrainFeature> Features = GetBiomeFeatures(BiomeType);
    UMaterialInterface* BiomeMaterial = GetBiomeMaterial(BiomeType);

    // Generate terrain features for this biome
    for (const FWorld_TerrainFeature& Feature : Features)
    {
        CreateTerrainFeature(Feature, BiomeCenter);
    }

    // Add biome-specific terrain generation
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            GenerateHills(BiomeCenter, 15000.0f, 8);
            GenerateRockFormations(BiomeCenter + FVector(5000, 0, 0), 12);
            break;
            
        case EBiomeType::Forest:
            GenerateHills(BiomeCenter, 20000.0f, 15);
            GenerateCliffs(BiomeCenter + FVector(0, 10000, 0), 800.0f, 2000.0f);
            break;
            
        case EBiomeType::Desert:
            GenerateCanyon(BiomeCenter + FVector(-8000, -5000, 0), 
                          BiomeCenter + FVector(8000, 5000, 0), 500.0f);
            GenerateRockFormations(BiomeCenter, 20);
            break;
            
        case EBiomeType::Swamp:
            // Lower, flatter terrain with water features
            GenerateHills(BiomeCenter, 10000.0f, 5);
            break;
            
        case EBiomeType::Mountain:
            GenerateCliffs(BiomeCenter, 1500.0f, 3000.0f);
            GenerateRockFormations(BiomeCenter + FVector(0, 0, 500), 25);
            GenerateCanyon(BiomeCenter + FVector(-10000, 0, 0), 
                          BiomeCenter + FVector(10000, 0, 0), 800.0f);
            break;
    }
}

void AWorld_TerrainManager::CreateTerrainFeature(const FWorld_TerrainFeature& Feature, FVector BiomeCenter)
{
    if (!GetWorld())
    {
        return;
    }

    FVector SpawnLocation = BiomeCenter + Feature.Location;
    SpawnLocation.Z += Feature.Elevation;

    // Choose appropriate mesh based on terrain type
    UStaticMesh* FeatureMesh = nullptr;
    switch (Feature.TerrainType)
    {
        case EWorld_TerrainType::Cliffs:
            FeatureMesh = CliffMesh;
            break;
        case EWorld_TerrainType::Mountains:
        case EWorld_TerrainType::Hills:
            FeatureMesh = HillMesh;
            break;
        case EWorld_TerrainType::Canyons:
            FeatureMesh = CanyonMesh;
            break;
        default:
            FeatureMesh = RockFormationMesh;
            break;
    }

    if (FeatureMesh)
    {
        // Spawn static mesh actor
        AStaticMeshActor* TerrainActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
            
        if (TerrainActor)
        {
            TerrainActor->GetStaticMeshComponent()->SetStaticMesh(FeatureMesh);
            TerrainActor->SetActorScale3D(Feature.Scale);
            TerrainActor->SetActorLabel(FString::Printf(TEXT("TerrainFeature_%s"), 
                                       *UEnum::GetValueAsString(Feature.TerrainType)));
        }
    }
}

void AWorld_TerrainManager::GenerateCliffs(FVector Location, float Height, float Width)
{
    if (!CliffMesh || !GetWorld())
    {
        return;
    }

    // Generate multiple cliff segments
    int32 CliffSegments = FMath::RandRange(3, 8);
    float SegmentWidth = Width / CliffSegments;

    for (int32 i = 0; i < CliffSegments; i++)
    {
        FVector CliffLocation = Location + FVector(i * SegmentWidth - Width * 0.5f, 
                                                  FMath::RandRange(-500.0f, 500.0f), 0);
        
        AStaticMeshActor* CliffActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), CliffLocation, FRotator::ZeroRotator);
            
        if (CliffActor)
        {
            CliffActor->GetStaticMeshComponent()->SetStaticMesh(CliffMesh);
            CliffActor->SetActorScale3D(FVector(1.0f, 1.0f, Height / 1000.0f));
            CliffActor->SetActorLabel(FString::Printf(TEXT("Cliff_%d"), i));
        }
    }
}

void AWorld_TerrainManager::GenerateRockFormations(FVector Location, int32 Count)
{
    if (!RockFormationMesh || !GetWorld())
    {
        return;
    }

    for (int32 i = 0; i < Count; i++)
    {
        FVector RockLocation = Location + FVector(
            FMath::RandRange(-10000.0f, 10000.0f),
            FMath::RandRange(-10000.0f, 10000.0f),
            0);

        AStaticMeshActor* RockActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), RockLocation, 
            FRotator(0, FMath::RandRange(0.0f, 360.0f), 0));
            
        if (RockActor)
        {
            RockActor->GetStaticMeshComponent()->SetStaticMesh(RockFormationMesh);
            float Scale = FMath::RandRange(0.5f, 2.0f);
            RockActor->SetActorScale3D(FVector(Scale));
            RockActor->SetActorLabel(FString::Printf(TEXT("RockFormation_%d"), i));
        }
    }
}

void AWorld_TerrainManager::GenerateCanyon(FVector StartLocation, FVector EndLocation, float Depth)
{
    if (!CanyonMesh || !GetWorld())
    {
        return;
    }

    FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
    float Distance = FVector::Dist(StartLocation, EndLocation);
    int32 CanyonSegments = FMath::CeilToInt(Distance / 2000.0f);

    for (int32 i = 0; i < CanyonSegments; i++)
    {
        float Alpha = (float)i / (float)CanyonSegments;
        FVector CanyonLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
        CanyonLocation.Z -= Depth * 0.5f;

        AStaticMeshActor* CanyonActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), CanyonLocation, 
            FRotationMatrix::MakeFromX(Direction).Rotator());
            
        if (CanyonActor)
        {
            CanyonActor->GetStaticMeshComponent()->SetStaticMesh(CanyonMesh);
            CanyonActor->SetActorScale3D(FVector(1.0f, 1.0f, Depth / 1000.0f));
            CanyonActor->SetActorLabel(FString::Printf(TEXT("Canyon_%d"), i));
        }
    }
}

void AWorld_TerrainManager::GenerateHills(FVector CenterLocation, float Radius, int32 HillCount)
{
    if (!HillMesh || !GetWorld())
    {
        return;
    }

    for (int32 i = 0; i < HillCount; i++)
    {
        float Angle = (2.0f * PI * i) / HillCount + FMath::RandRange(-0.5f, 0.5f);
        float Distance = FMath::RandRange(Radius * 0.3f, Radius);
        
        FVector HillLocation = CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0);

        AStaticMeshActor* HillActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), HillLocation, FRotator::ZeroRotator);
            
        if (HillActor)
        {
            HillActor->GetStaticMeshComponent()->SetStaticMesh(HillMesh);
            float Scale = FMath::RandRange(0.8f, 1.5f);
            HillActor->SetActorScale3D(FVector(Scale, Scale, Scale * 0.6f));
            HillActor->SetActorLabel(FString::Printf(TEXT("Hill_%d"), i));
        }
    }
}

float AWorld_TerrainManager::GetTerrainHeight(FVector Location)
{
    // Simple Perlin noise-based height calculation
    float Height = GeneratePerlinNoise(Location.X, Location.Y, NoiseScale) * HeightMultiplier;
    return Height;
}

EWorld_TerrainType AWorld_TerrainManager::GetTerrainTypeAtLocation(FVector Location)
{
    float Height = GetTerrainHeight(Location);
    
    if (Height > 800.0f)
    {
        return EWorld_TerrainType::Mountains;
    }
    else if (Height > 400.0f)
    {
        return EWorld_TerrainType::Hills;
    }
    else if (Height < -100.0f)
    {
        return EWorld_TerrainType::Valleys;
    }
    else
    {
        return EWorld_TerrainType::Plains;
    }
}

TArray<FVector> AWorld_TerrainManager::GetSuitableSpawnLocations(EWorld_TerrainType TerrainType, int32 Count)
{
    TArray<FVector> SpawnLocations;
    
    // Generate random locations and filter by terrain type
    for (int32 i = 0; i < Count * 3; i++) // Generate more than needed for filtering
    {
        FVector TestLocation = FVector(
            FMath::RandRange(-TerrainSize * 0.5f, TerrainSize * 0.5f),
            FMath::RandRange(-TerrainSize * 0.5f, TerrainSize * 0.5f),
            0);
            
        if (GetTerrainTypeAtLocation(TestLocation) == TerrainType)
        {
            TestLocation.Z = GetTerrainHeight(TestLocation);
            SpawnLocations.Add(TestLocation);
            
            if (SpawnLocations.Num() >= Count)
            {
                break;
            }
        }
    }
    
    return SpawnLocations;
}

void AWorld_TerrainManager::ClearTerrainInRadius(FVector Location, float Radius)
{
    if (!GetWorld())
    {
        return;
    }

    // Find and destroy terrain actors in radius
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (FVector::Dist(Actor->GetActorLocation(), Location) <= Radius)
        {
            FString ActorLabel = Actor->GetActorLabel();
            if (ActorLabel.Contains(TEXT("Terrain")) || ActorLabel.Contains(TEXT("Cliff")) ||
                ActorLabel.Contains(TEXT("Rock")) || ActorLabel.Contains(TEXT("Hill")) ||
                ActorLabel.Contains(TEXT("Canyon")))
            {
                Actor->Destroy();
            }
        }
    }
}

void AWorld_TerrainManager::RegenerateAllTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("Regenerating all terrain features"));
    
    // Clear existing terrain
    ClearAllTerrain();
    
    // Generate terrain for all biomes
    TArray<EBiomeType> Biomes = {EBiomeType::Savanna, EBiomeType::Forest, EBiomeType::Desert, 
                                EBiomeType::Swamp, EBiomeType::Mountain};
    
    for (EBiomeType Biome : Biomes)
    {
        FVector BiomeCenter = GetBiomeCenter(Biome);
        GenerateTerrainForBiome(Biome, BiomeCenter);
    }
}

void AWorld_TerrainManager::ClearAllTerrain()
{
    if (!GetWorld())
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        FString ActorLabel = Actor->GetActorLabel();
        if (ActorLabel.Contains(TEXT("Terrain")) || ActorLabel.Contains(TEXT("Cliff")) ||
            ActorLabel.Contains(TEXT("Rock")) || ActorLabel.Contains(TEXT("Hill")) ||
            ActorLabel.Contains(TEXT("Canyon")))
        {
            Actor->Destroy();
        }
    }
}

float AWorld_TerrainManager::GeneratePerlinNoise(float X, float Y, float Scale)
{
    // Simple Perlin noise implementation
    float ScaledX = X * Scale;
    float ScaledY = Y * Scale;
    
    // Basic noise calculation using sin/cos
    float Noise = FMath::Sin(ScaledX * 0.1f) * FMath::Cos(ScaledY * 0.1f) * 0.5f +
                  FMath::Sin(ScaledX * 0.3f) * FMath::Cos(ScaledY * 0.3f) * 0.3f +
                  FMath::Sin(ScaledX * 0.7f) * FMath::Cos(ScaledY * 0.7f) * 0.2f;
                  
    return FMath::Clamp(Noise, -1.0f, 1.0f);
}

FVector AWorld_TerrainManager::GetBiomeCenter(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:   return FVector(0, 0, 100);
        case EBiomeType::Swamp:     return FVector(-50000, -45000, 50);
        case EBiomeType::Forest:    return FVector(-45000, 40000, 200);
        case EBiomeType::Desert:    return FVector(55000, 0, 150);
        case EBiomeType::Mountain:  return FVector(40000, 50000, 800);
        default:                    return FVector::ZeroVector;
    }
}

UMaterialInterface* AWorld_TerrainManager::GetBiomeMaterial(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:   return SavannaMaterial;
        case EBiomeType::Forest:    return ForestMaterial;
        case EBiomeType::Desert:    return DesertMaterial;
        case EBiomeType::Swamp:     return SwampMaterial;
        case EBiomeType::Mountain:  return MountainMaterial;
        default:                    return nullptr;
    }
}

TArray<FWorld_TerrainFeature> AWorld_TerrainManager::GetBiomeFeatures(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:   return SavannaFeatures;
        case EBiomeType::Forest:    return ForestFeatures;
        case EBiomeType::Desert:    return DesertFeatures;
        case EBiomeType::Swamp:     return SwampFeatures;
        case EBiomeType::Mountain:  return MountainFeatures;
        default:                    return TArray<FWorld_TerrainFeature>();
    }
}