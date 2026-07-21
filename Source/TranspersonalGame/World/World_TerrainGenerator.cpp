#include "World_TerrainGenerator.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "EditorLevelLibrary.h"

AWorld_TerrainGenerator::AWorld_TerrainGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Load default meshes
    static ConstructorHelpers::FObjectFinder<UStaticMesh> RockMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (RockMeshAsset.Succeeded())
    {
        RockMesh = RockMeshAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CliffMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CliffMeshAsset.Succeeded())
    {
        CliffMesh = CliffMeshAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CaveMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (CaveMeshAsset.Succeeded())
    {
        CaveMesh = CaveMeshAsset.Object;
    }
}

void AWorld_TerrainGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_TerrainGenerator: Ready for terrain generation"));
}

void AWorld_TerrainGenerator::GenerateTerrainForBiome(EBiomeType BiomeType, FVector BiomeCenter, int32 FeatureCount)
{
    if (!RockMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("World_TerrainGenerator: RockMesh is null"));
        return;
    }

    // Limit feature count to prevent performance issues
    FeatureCount = FMath::Clamp(FeatureCount, 1, MaxFeaturesPerBiome);

    UE_LOG(LogTemp, Warning, TEXT("Generating %d terrain features for biome %d at location (%f, %f, %f)"), 
           FeatureCount, (int32)BiomeType, BiomeCenter.X, BiomeCenter.Y, BiomeCenter.Z);

    TArray<FVector> FeatureLocations;
    
    // Generate random positions for terrain features
    for (int32 i = 0; i < FeatureCount; i++)
    {
        FVector FeaturePosition = GetRandomBiomePosition(BiomeCenter);
        
        if (IsValidTerrainPosition(FeaturePosition))
        {
            FeatureLocations.Add(FeaturePosition);
        }
    }

    // Create rock formations
    TArray<AActor*> NewRocks = CreateRockFormations(FeatureLocations);
    CreatedTerrainActors.Append(NewRocks);

    // Generate biome-specific features
    switch (BiomeType)
    {
        case EBiomeType::Mountain:
            CreateCaveEntrances(BiomeCenter, 5);
            GenerateCliffFaces(BiomeCenter + FVector(-10000, -10000, 0), BiomeCenter + FVector(10000, 10000, 0), 8);
            break;
        case EBiomeType::Desert:
            // Create sand dunes (using scaled rock meshes)
            for (int32 i = 0; i < 20; i++)
            {
                FVector DunePos = GetRandomBiomePosition(BiomeCenter, 12000.0f);
                DunePos.Z = 80.0f; // Lower height for dunes
                CreateRockFormations({DunePos}, 5.0f, 15.0f);
            }
            break;
        case EBiomeType::Swamp:
            // Create elevated land patches
            for (int32 i = 0; i < 15; i++)
            {
                FVector PatchPos = GetRandomBiomePosition(BiomeCenter, 10000.0f);
                PatchPos.Z = 60.0f; // Slightly elevated
                CreateRockFormations({PatchPos}, 8.0f, 12.0f);
            }
            break;
        default:
            break;
    }
}

TArray<AActor*> AWorld_TerrainGenerator::CreateRockFormations(const TArray<FVector>& Locations, float MinScale, float MaxScale)
{
    TArray<AActor*> CreatedRocks;
    
    if (!RockMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("World_TerrainGenerator: Cannot create rocks - RockMesh is null"));
        return CreatedRocks;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World_TerrainGenerator: World is null"));
        return CreatedRocks;
    }

    for (const FVector& Location : Locations)
    {
        // Spawn static mesh actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
        
        if (RockActor)
        {
            // Set the mesh
            UStaticMeshComponent* MeshComp = RockActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                MeshComp->SetStaticMesh(RockMesh);
                
                // Random scale
                float Scale = FMath::RandRange(MinScale, MaxScale);
                RockActor->SetActorScale3D(FVector(Scale, Scale, Scale));
                
                // Random rotation
                FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
                RockActor->SetActorRotation(RandomRotation);
            }
            
            // Set label for identification
            FString RockLabel = FString::Printf(TEXT("TerrainRock_%d"), CreatedRocks.Num() + 1);
            RockActor->SetActorLabel(RockLabel);
            
            CreatedRocks.Add(RockActor);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Created %d rock formations"), CreatedRocks.Num());
    return CreatedRocks;
}

void AWorld_TerrainGenerator::GenerateCliffFaces(FVector StartPoint, FVector EndPoint, int32 CliffCount)
{
    if (!CliffMesh || CliffCount <= 0)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FVector Direction = (EndPoint - StartPoint).GetSafeNormal();
    float TotalDistance = FVector::Dist(StartPoint, EndPoint);
    float CliffSpacing = TotalDistance / CliffCount;

    for (int32 i = 0; i < CliffCount; i++)
    {
        FVector CliffPosition = StartPoint + (Direction * CliffSpacing * i);
        CliffPosition.Z += FMath::RandRange(100.0f, 300.0f); // Elevate cliffs
        
        FActorSpawnParameters SpawnParams;
        AStaticMeshActor* CliffActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), CliffPosition, FRotator::ZeroRotator, SpawnParams);
        
        if (CliffActor)
        {
            UStaticMeshComponent* MeshComp = CliffActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                MeshComp->SetStaticMesh(CliffMesh);
                
                // Scale for cliff appearance
                FVector CliffScale = FVector(3.0f, 1.0f, 8.0f);
                CliffActor->SetActorScale3D(CliffScale);
            }
            
            CliffActor->SetActorLabel(FString::Printf(TEXT("Cliff_%d"), i + 1));
            CreatedTerrainActors.Add(CliffActor);
        }
    }
}

TArray<AActor*> AWorld_TerrainGenerator::CreateCaveEntrances(FVector MountainCenter, int32 CaveCount)
{
    TArray<AActor*> CreatedCaves;
    
    if (!CaveMesh || CaveCount <= 0)
    {
        return CreatedCaves;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return CreatedCaves;
    }

    for (int32 i = 0; i < CaveCount; i++)
    {
        FVector CavePosition = GetRandomBiomePosition(MountainCenter, 8000.0f);
        CavePosition.Z += FMath::RandRange(200.0f, 500.0f); // Place caves on mountain sides
        
        FActorSpawnParameters SpawnParams;
        AStaticMeshActor* CaveActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), CavePosition, FRotator::ZeroRotator, SpawnParams);
        
        if (CaveActor)
        {
            UStaticMeshComponent* MeshComp = CaveActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                MeshComp->SetStaticMesh(CaveMesh);
                
                // Scale for cave entrance
                FVector CaveScale = FVector(4.0f, 2.0f, 3.0f);
                CaveActor->SetActorScale3D(CaveScale);
            }
            
            CaveActor->SetActorLabel(FString::Printf(TEXT("CaveEntrance_%d"), i + 1));
            CreatedCaves.Add(CaveActor);
            CreatedTerrainActors.Add(CaveActor);
        }
    }

    return CreatedCaves;
}

void AWorld_TerrainGenerator::ClearBiomeTerrainFeatures(EBiomeType BiomeType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Remove all created terrain actors
    for (AActor* Actor : CreatedTerrainActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    CreatedTerrainActors.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Cleared terrain features for biome %d"), (int32)BiomeType);
}

FVector AWorld_TerrainGenerator::GetRandomBiomePosition(FVector BiomeCenter, float Radius)
{
    float RandomX = FMath::RandRange(-Radius, Radius);
    float RandomY = FMath::RandRange(-Radius, Radius);
    float RandomZ = FMath::RandRange(HeightRange.X, HeightRange.Y);
    
    return BiomeCenter + FVector(RandomX, RandomY, RandomZ);
}

bool AWorld_TerrainGenerator::IsValidTerrainPosition(FVector Position)
{
    // Check minimum distance from existing features
    for (AActor* ExistingActor : CreatedTerrainActors)
    {
        if (IsValid(ExistingActor))
        {
            float Distance = FVector::Dist(Position, ExistingActor->GetActorLocation());
            if (Distance < MinFeatureDistance)
            {
                return false;
            }
        }
    }
    
    return true;
}

float AWorld_TerrainGenerator::GetBiomeSpecificScale(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Mountain:
            return FMath::RandRange(5.0f, 12.0f); // Large mountain rocks
        case EBiomeType::Desert:
            return FMath::RandRange(2.0f, 6.0f);  // Medium desert rocks
        case EBiomeType::Swamp:
            return FMath::RandRange(1.0f, 4.0f);  // Small swamp features
        case EBiomeType::Forest:
            return FMath::RandRange(2.0f, 8.0f);  // Varied forest rocks
        case EBiomeType::Savanna:
        default:
            return FMath::RandRange(3.0f, 7.0f);  // Medium savanna rocks
    }
}