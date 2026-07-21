#include "EnvArt_VegetationSpawner.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UEnvArt_VegetationSpawner::UEnvArt_VegetationSpawner()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize forest vegetation defaults
    ForestTrees.Density = 0.8f;
    ForestTrees.ClusterRadius = 800.0f;
    ForestTrees.MaxInstancesPerCluster = 30;
    ForestTrees.ScaleRange = FVector2D(0.9f, 1.3f);

    ForestUndergrowth.Density = 1.5f;
    ForestUndergrowth.ClusterRadius = 300.0f;
    ForestUndergrowth.MaxInstancesPerCluster = 80;
    ForestUndergrowth.ScaleRange = FVector2D(0.6f, 1.0f);

    // Initialize swampland vegetation defaults
    SwampCypress.Density = 0.6f;
    SwampCypress.ClusterRadius = 1000.0f;
    SwampCypress.MaxInstancesPerCluster = 20;
    SwampCypress.ScaleRange = FVector2D(1.0f, 1.5f);

    SwampMoss.Density = 2.0f;
    SwampMoss.ClusterRadius = 200.0f;
    SwampMoss.MaxInstancesPerCluster = 100;
    SwampMoss.ScaleRange = FVector2D(0.8f, 1.2f);

    // Initialize canyon vegetation defaults
    DesertCacti.Density = 0.3f;
    DesertCacti.ClusterRadius = 600.0f;
    DesertCacti.MaxInstancesPerCluster = 15;
    DesertCacti.ScaleRange = FVector2D(0.7f, 1.4f);

    RockFormations.Density = 0.4f;
    RockFormations.ClusterRadius = 1200.0f;
    RockFormations.MaxInstancesPerCluster = 25;
    RockFormations.ScaleRange = FVector2D(0.8f, 2.0f);

    // Initialize spawning parameters
    SpawnRadius = 2000.0f;
    MaxVegetationInstances = 1000;
    bUseRandomSeed = true;
    RandomSeed = 12345;
    CurrentBiome = EBiomeType::Forest;
}

void UEnvArt_VegetationSpawner::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize random stream
    if (bUseRandomSeed)
    {
        RandomStream.Initialize(FMath::Rand());
    }
    else
    {
        RandomStream.Initialize(RandomSeed);
    }
}

void UEnvArt_VegetationSpawner::SpawnVegetationForBiome(EBiomeType BiomeType, const FVector& CenterLocation)
{
    CurrentBiome = BiomeType;
    
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            SpawnForestVegetation(CenterLocation);
            break;
        case EBiomeType::Swampland:
            SpawnSwamplandVegetation(CenterLocation);
            break;
        case EBiomeType::Canyon:
            SpawnCanyonVegetation(CenterLocation);
            break;
        case EBiomeType::Plains:
            // Spawn grass and scattered trees
            SpawnVegetationCluster(ForestUndergrowth, CenterLocation);
            break;
        case EBiomeType::Mountains:
            // Spawn hardy vegetation and rocks
            SpawnVegetationCluster(RockFormations, CenterLocation);
            break;
        case EBiomeType::Desert:
            // Spawn cacti and desert plants
            SpawnVegetationCluster(DesertCacti, CenterLocation);
            break;
        default:
            SpawnForestVegetation(CenterLocation);
            break;
    }
}

void UEnvArt_VegetationSpawner::SpawnVegetationCluster(const FEnvArt_VegetationCluster& ClusterConfig, const FVector& CenterLocation)
{
    if (!GetWorld())
    {
        return;
    }

    int32 InstancesToSpawn = FMath::RoundToInt(ClusterConfig.MaxInstancesPerCluster * ClusterConfig.Density);
    InstancesToSpawn = FMath::Min(InstancesToSpawn, MaxVegetationInstances - SpawnedVegetationActors.Num());

    for (int32 i = 0; i < InstancesToSpawn; i++)
    {
        // Get random location within cluster radius
        FVector SpawnLocation = GetRandomLocationInRadius(CenterLocation, ClusterConfig.ClusterRadius);
        
        if (!IsValidSpawnLocation(SpawnLocation))
        {
            continue;
        }

        // Select random mesh variant
        if (ClusterConfig.MeshVariants.Num() > 0)
        {
            int32 MeshIndex = RandomStream.RandRange(0, ClusterConfig.MeshVariants.Num() - 1);
            TSoftObjectPtr<UStaticMesh> MeshPtr = ClusterConfig.MeshVariants[MeshIndex];
            
            if (UStaticMesh* Mesh = MeshPtr.LoadSynchronous())
            {
                // Random rotation (only Y-axis for most vegetation)
                FRotator SpawnRotation = FRotator(0.0f, RandomStream.FRandRange(0.0f, 360.0f), 0.0f);
                
                // Random scale within range
                float ScaleValue = RandomStream.FRandRange(ClusterConfig.ScaleRange.X, ClusterConfig.ScaleRange.Y);
                FVector SpawnScale = FVector(ScaleValue);

                SpawnMeshInstance(Mesh, SpawnLocation, SpawnRotation, SpawnScale);
            }
        }
    }
}

void UEnvArt_VegetationSpawner::ClearAllVegetation()
{
    for (AActor* Actor : SpawnedVegetationActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedVegetationActors.Empty();
}

void UEnvArt_VegetationSpawner::RefreshVegetation()
{
    ClearAllVegetation();
    
    if (AActor* Owner = GetOwner())
    {
        SpawnVegetationForBiome(CurrentBiome, Owner->GetActorLocation());
    }
}

void UEnvArt_VegetationSpawner::SpawnTestVegetation()
{
    if (AActor* Owner = GetOwner())
    {
        SpawnVegetationForBiome(CurrentBiome, Owner->GetActorLocation());
    }
}

void UEnvArt_VegetationSpawner::ClearTestVegetation()
{
    ClearAllVegetation();
}

void UEnvArt_VegetationSpawner::SpawnForestVegetation(const FVector& CenterLocation)
{
    // Spawn trees first
    SpawnVegetationCluster(ForestTrees, CenterLocation);
    
    // Then spawn undergrowth around trees
    for (int32 i = 0; i < 3; i++)
    {
        FVector UndergrowthCenter = GetRandomLocationInRadius(CenterLocation, SpawnRadius * 0.7f);
        SpawnVegetationCluster(ForestUndergrowth, UndergrowthCenter);
    }
}

void UEnvArt_VegetationSpawner::SpawnSwamplandVegetation(const FVector& CenterLocation)
{
    // Spawn cypress trees
    SpawnVegetationCluster(SwampCypress, CenterLocation);
    
    // Spawn moss and swamp vegetation
    for (int32 i = 0; i < 4; i++)
    {
        FVector MossCenter = GetRandomLocationInRadius(CenterLocation, SpawnRadius * 0.8f);
        SpawnVegetationCluster(SwampMoss, MossCenter);
    }
}

void UEnvArt_VegetationSpawner::SpawnCanyonVegetation(const FVector& CenterLocation)
{
    // Spawn rock formations
    SpawnVegetationCluster(RockFormations, CenterLocation);
    
    // Spawn scattered desert vegetation
    for (int32 i = 0; i < 2; i++)
    {
        FVector CactiCenter = GetRandomLocationInRadius(CenterLocation, SpawnRadius * 0.6f);
        SpawnVegetationCluster(DesertCacti, CactiCenter);
    }
}

FVector UEnvArt_VegetationSpawner::GetRandomLocationInRadius(const FVector& Center, float Radius)
{
    float RandomAngle = RandomStream.FRandRange(0.0f, 2.0f * PI);
    float RandomDistance = RandomStream.FRandRange(0.0f, Radius);
    
    FVector Offset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return Center + Offset;
}

bool UEnvArt_VegetationSpawner::IsValidSpawnLocation(const FVector& Location)
{
    if (!GetWorld())
    {
        return false;
    }

    // Simple ground check - trace down to find ground
    FHitResult HitResult;
    FVector StartTrace = Location + FVector(0.0f, 0.0f, 1000.0f);
    FVector EndTrace = Location - FVector(0.0f, 0.0f, 1000.0f);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, QueryParams))
    {
        // Check if slope is reasonable for vegetation
        float SlopeDegrees = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector)));
        return SlopeDegrees < 45.0f; // Don't spawn on steep slopes
    }
    
    return false;
}

void UEnvArt_VegetationSpawner::SpawnMeshInstance(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    if (!GetWorld() || !Mesh)
    {
        return;
    }

    // Spawn static mesh actor
    AStaticMeshActor* MeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (MeshActor)
    {
        // Set the mesh
        MeshActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
        MeshActor->GetStaticMeshComponent()->SetWorldScale3D(Scale);
        
        // Set collision to block only world static (for performance)
        MeshActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        MeshActor->GetStaticMeshComponent()->SetCollisionObjectType(ECC_WorldStatic);
        
        // Add to tracking array
        SpawnedVegetationActors.Add(MeshActor);
        
        // Set appropriate tags for identification
        MeshActor->Tags.Add(FName("EnvArt_Vegetation"));
        MeshActor->Tags.Add(FName(*UEnum::GetValueAsString(CurrentBiome)));
    }
}