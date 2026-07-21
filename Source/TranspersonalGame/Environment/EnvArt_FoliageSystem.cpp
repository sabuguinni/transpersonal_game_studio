#include "EnvArt_FoliageSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "Components/StaticMeshComponent.h"

AEnvArt_FoliageSystem::AEnvArt_FoliageSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    UpdateRadius = 5000.0f;
    LODDistance1 = 1000.0f;
    LODDistance2 = 2500.0f;
    bEnableWind = true;
    WindStrength = 1.0f;
    WindDirection = FVector(1.0f, 0.0f, 0.0f);
}

void AEnvArt_FoliageSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeFoliageComponents();
    
    PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    
    // Generate initial foliage around spawn point
    if (PlayerPawn)
    {
        GenerateFoliageInRadius(PlayerPawn->GetActorLocation(), UpdateRadius);
    }
}

void AEnvArt_FoliageSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (PlayerPawn)
    {
        UpdateFoliageLOD();
        
        // Update wind animation
        if (bEnableWind)
        {
            SetWindParameters(WindStrength, WindDirection);
        }
    }
}

void AEnvArt_FoliageSystem::InitializeFoliageComponents()
{
    // Create instanced mesh components for each foliage type
    for (int32 i = 0; i < (int32)EEnvArt_FoliageType::Debris + 1; ++i)
    {
        EEnvArt_FoliageType FoliageType = (EEnvArt_FoliageType)i;
        if (FoliageType == EEnvArt_FoliageType::None) continue;
        
        FString ComponentName = FString::Printf(TEXT("InstancedMesh_%d"), i);
        UInstancedStaticMeshComponent* InstancedComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(*ComponentName);
        InstancedComp->SetupAttachment(RootComponent);
        InstancedComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        InstancedComp->SetCastShadow(true);
        InstancedComp->bReceivesDecals = false;
        
        InstancedMeshComponents.Add(FoliageType, InstancedComp);
    }
}

void AEnvArt_FoliageSystem::GenerateFoliageInRadius(const FVector& Center, float Radius)
{
    if (!GetWorld()) return;
    
    for (const FEnvArt_FoliageCluster& Cluster : FoliageClusters)
    {
        if (Cluster.FoliageType == EEnvArt_FoliageType::None) continue;
        
        // Generate multiple spawn points within radius
        int32 NumClusters = FMath::RandRange(3, 8);
        for (int32 i = 0; i < NumClusters; ++i)
        {
            FVector ClusterCenter = GetRandomLocationInRadius(Center, Radius * 0.8f);
            SpawnFoliageCluster(Cluster, ClusterCenter);
        }
    }
}

void AEnvArt_FoliageSystem::SpawnFoliageCluster(const FEnvArt_FoliageCluster& Cluster, const FVector& Center)
{
    UInstancedStaticMeshComponent* InstancedComp = InstancedMeshComponents.FindRef(Cluster.FoliageType);
    if (!InstancedComp || !Cluster.MeshAsset.LoadSynchronous()) return;
    
    // Set the mesh if not already set
    if (!InstancedComp->GetStaticMesh())
    {
        InstancedComp->SetStaticMesh(Cluster.MeshAsset.LoadSynchronous());
        
        if (Cluster.MaterialOverride.LoadSynchronous())
        {
            InstancedComp->SetMaterial(0, Cluster.MaterialOverride.LoadSynchronous());
        }
    }
    
    // Spawn instances in cluster
    int32 NumInstances = FMath::RandRange(Cluster.MaxInstancesPerCluster / 2, Cluster.MaxInstancesPerCluster);
    for (int32 i = 0; i < NumInstances; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInRadius(Center, Cluster.ClusterRadius);
        
        if (!IsValidSpawnLocation(SpawnLocation, Cluster.FoliageType)) continue;
        
        // Adjust to terrain height
        float TerrainHeight = GetTerrainHeightAtLocation(SpawnLocation);
        SpawnLocation.Z = TerrainHeight;
        
        // Random rotation
        FRotator SpawnRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        
        // Random scale
        float ScaleVariation = FMath::RandRange(Cluster.ScaleRange.X, Cluster.ScaleRange.Y);
        FVector SpawnScale = FVector(ScaleVariation);
        
        // Create transform
        FTransform SpawnTransform(SpawnRotation, SpawnLocation, SpawnScale);
        
        // Add instance
        InstancedComp->AddInstance(SpawnTransform);
    }
}

bool AEnvArt_FoliageSystem::IsValidSpawnLocation(const FVector& Location, EEnvArt_FoliageType FoliageType)
{
    if (!GetWorld()) return false;
    
    // Line trace to check if location is on solid ground
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 1000);
    FVector TraceEnd = Location - FVector(0, 0, 1000);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_WorldStatic
    );
    
    if (!bHit) return false;
    
    // Check slope - foliage shouldn't spawn on steep surfaces
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector)));
    if (SlopeAngle > 45.0f) return false;
    
    // Type-specific validation
    switch (FoliageType)
    {
        case EEnvArt_FoliageType::Moss:
        case EEnvArt_FoliageType::Ferns:
            // Prefer shaded areas near water or rocks
            return true;
            
        case EEnvArt_FoliageType::Grass:
        case EEnvArt_FoliageType::Flowers:
            // Prefer open areas with good sunlight
            return SlopeAngle < 30.0f;
            
        case EEnvArt_FoliageType::Fallen_Logs:
        case EEnvArt_FoliageType::Rocks:
            // Can spawn anywhere with reasonable slope
            return SlopeAngle < 35.0f;
            
        default:
            return true;
    }
}

FVector AEnvArt_FoliageSystem::GetRandomLocationInRadius(const FVector& Center, float Radius)
{
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::RandRange(0.0f, Radius);
    
    FVector Offset = FVector(
        FMath::Cos(Angle) * Distance,
        FMath::Sin(Angle) * Distance,
        0.0f
    );
    
    return Center + Offset;
}

float AEnvArt_FoliageSystem::GetTerrainHeightAtLocation(const FVector& Location)
{
    if (!GetWorld()) return Location.Z;
    
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 10000);
    FVector TraceEnd = Location - FVector(0, 0, 10000);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_WorldStatic
    );
    
    return bHit ? HitResult.Location.Z : Location.Z;
}

void AEnvArt_FoliageSystem::ClearFoliageInRadius(const FVector& Center, float Radius)
{
    for (auto& Pair : InstancedMeshComponents)
    {
        UInstancedStaticMeshComponent* InstancedComp = Pair.Value;
        if (!InstancedComp) continue;
        
        TArray<int32> InstancesToRemove;
        
        for (int32 i = 0; i < InstancedComp->GetInstanceCount(); ++i)
        {
            FTransform InstanceTransform;
            if (InstancedComp->GetInstanceTransform(i, InstanceTransform, true))
            {
                float Distance = FVector::Dist(InstanceTransform.GetLocation(), Center);
                if (Distance <= Radius)
                {
                    InstancesToRemove.Add(i);
                }
            }
        }
        
        // Remove instances in reverse order to maintain indices
        for (int32 i = InstancesToRemove.Num() - 1; i >= 0; --i)
        {
            InstancedComp->RemoveInstance(InstancesToRemove[i]);
        }
    }
}

void AEnvArt_FoliageSystem::UpdateFoliageLOD()
{
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (auto& Pair : InstancedMeshComponents)
    {
        UInstancedStaticMeshComponent* InstancedComp = Pair.Value;
        if (!InstancedComp) continue;
        
        // Simple distance-based LOD
        float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerLocation);
        
        if (DistanceToPlayer > LODDistance2)
        {
            InstancedComp->SetVisibility(false);
        }
        else if (DistanceToPlayer > LODDistance1)
        {
            InstancedComp->SetVisibility(true);
            // Could set lower LOD mesh here
        }
        else
        {
            InstancedComp->SetVisibility(true);
            // Full detail
        }
    }
}

void AEnvArt_FoliageSystem::SetWindParameters(float Strength, const FVector& Direction)
{
    WindStrength = Strength;
    WindDirection = Direction.GetSafeNormal();
    
    // Apply wind to material parameters
    for (auto& Pair : InstancedMeshComponents)
    {
        UInstancedStaticMeshComponent* InstancedComp = Pair.Value;
        if (!InstancedComp) continue;
        
        // Set material parameters for wind animation
        InstancedComp->SetScalarParameterValueOnMaterials(TEXT("WindStrength"), WindStrength);
        InstancedComp->SetVectorParameterValueOnMaterials(TEXT("WindDirection"), WindDirection);
    }
}