#include "EnvArt_RockFormationSpawner.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

UEnvArt_RockFormationSpawner::UEnvArt_RockFormationSpawner()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default rock formation data
    RockFormationData.ScaleRange = FVector2D(0.8f, 2.5f);
    RockFormationData.ClusterRadius = 500.0f;
    RockFormationData.MinRocksPerCluster = 3;
    RockFormationData.MaxRocksPerCluster = 8;
    RockFormationData.SurfaceAlignmentStrength = 0.7f;
    
    SpawnRadius = 2000.0f;
    MaxFormations = 20;
    MinDistanceBetweenFormations = 800.0f;
    
    SpawnedRockActors.Empty();
}

void UEnvArt_RockFormationSpawner::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeRockData();
    
    // Auto-spawn formations on begin play
    if (GetWorld())
    {
        SpawnRockFormations();
    }
}

void UEnvArt_RockFormationSpawner::SpawnRockFormations()
{
    if (!GetWorld())
    {
        return;
    }
    
    ClearAllRockFormations();
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    TArray<FVector> FormationLocations;
    
    // Generate formation locations
    for (int32 i = 0; i < MaxFormations; i++)
    {
        FVector RandomLocation = OwnerLocation + FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            0.0f
        );
        
        // Check minimum distance from other formations
        bool bValidLocation = true;
        for (const FVector& ExistingLocation : FormationLocations)
        {
            if (FVector::Dist2D(RandomLocation, ExistingLocation) < MinDistanceBetweenFormations)
            {
                bValidLocation = false;
                break;
            }
        }
        
        if (bValidLocation && IsValidSpawnLocation(RandomLocation))
        {
            FVector SurfaceLocation = GetSurfaceLocation(RandomLocation);
            FormationLocations.Add(SurfaceLocation);
            
            // Determine biome type (simplified - using height for now)
            EBiomeType BiomeType = EBiomeType::TemperateForest;
            if (SurfaceLocation.Z > 500.0f)
            {
                BiomeType = EBiomeType::Mountain;
            }
            else if (SurfaceLocation.Z < 100.0f)
            {
                BiomeType = EBiomeType::Swampland;
            }
            
            SpawnSingleFormation(SurfaceLocation, BiomeType);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("RockFormationSpawner: Spawned %d formations with %d total rocks"), 
           FormationLocations.Num(), SpawnedRockActors.Num());
}

void UEnvArt_RockFormationSpawner::SpawnSingleFormation(const FVector& Location, EBiomeType BiomeType)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get biome-specific rock data
    const FEnvArt_RockFormationData* FormationData = &RockFormationData;
    if (BiomeSpecificRocks.Contains(BiomeType))
    {
        FormationData = &BiomeSpecificRocks[BiomeType];
    }
    
    int32 NumRocks = FMath::RandRange(FormationData->MinRocksPerCluster, FormationData->MaxRocksPerCluster);
    
    for (int32 i = 0; i < NumRocks; i++)
    {
        // Random position within cluster radius
        FVector2D RandomCircle = FMath::RandPointInCircle(FormationData->ClusterRadius);
        FVector RockLocation = Location + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);
        
        // Trace to surface
        FVector SurfaceLocation = GetSurfaceLocation(RockLocation);
        
        // Select random rock mesh
        UStaticMesh* RockMesh = SelectRandomRockMesh(BiomeType);
        if (!RockMesh)
        {
            continue;
        }
        
        // Spawn rock actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwner();
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AStaticMeshActor* RockActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SurfaceLocation, FRotator::ZeroRotator, SpawnParams);
        
        if (RockActor)
        {
            // Set mesh
            RockActor->GetStaticMeshComponent()->SetStaticMesh(RockMesh);
            
            // Random scale
            float RandomScale = FMath::RandRange(FormationData->ScaleRange.X, FormationData->ScaleRange.Y);
            RockActor->SetActorScale3D(FVector(RandomScale));
            
            // Calculate surface-aligned rotation
            FVector SurfaceNormal = FVector::UpVector; // Default up
            FHitResult HitResult;
            FVector TraceStart = SurfaceLocation + FVector(0, 0, 100);
            FVector TraceEnd = SurfaceLocation - FVector(0, 0, 200);
            
            if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
            {
                SurfaceNormal = HitResult.Normal;
            }
            
            FRotator SurfaceRotation = CalculateRockRotation(SurfaceNormal);
            RockActor->SetActorRotation(SurfaceRotation);
            
            // Set label for identification
            RockActor->SetActorLabel(FString::Printf(TEXT("EnvArt_Rock_%s_%d"), 
                                                   *UEnum::GetValueAsString(BiomeType), SpawnedRockActors.Num()));
            
            SpawnedRockActors.Add(RockActor);
        }
    }
}

void UEnvArt_RockFormationSpawner::ClearAllRockFormations()
{
    for (AActor* RockActor : SpawnedRockActors)
    {
        if (IsValid(RockActor))
        {
            RockActor->Destroy();
        }
    }
    SpawnedRockActors.Empty();
}

void UEnvArt_RockFormationSpawner::UpdateFormationsForBiome(EBiomeType NewBiome)
{
    // Re-spawn formations with new biome data
    SpawnRockFormations();
}

bool UEnvArt_RockFormationSpawner::IsValidSpawnLocation(const FVector& Location) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Check if location is on valid terrain
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 1000);
    FVector TraceEnd = Location - FVector(0, 0, 1000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());
    
    return GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);
}

FVector UEnvArt_RockFormationSpawner::GetSurfaceLocation(const FVector& StartLocation) const
{
    if (!GetWorld())
    {
        return StartLocation;
    }
    
    FHitResult HitResult;
    FVector TraceStart = StartLocation + FVector(0, 0, 1000);
    FVector TraceEnd = StartLocation - FVector(0, 0, 1000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location;
    }
    
    return StartLocation;
}

FRotator UEnvArt_RockFormationSpawner::CalculateRockRotation(const FVector& SurfaceNormal) const
{
    // Blend between surface normal and random rotation
    FVector UpVector = FMath::Lerp(FVector::UpVector, SurfaceNormal, RockFormationData.SurfaceAlignmentStrength);
    
    // Add random yaw rotation
    float RandomYaw = FMath::RandRange(0.0f, 360.0f);
    
    // Calculate rotation from up vector
    FRotator SurfaceRotation = FRotationMatrix::MakeFromZ(UpVector).Rotator();
    SurfaceRotation.Yaw += RandomYaw;
    
    // Add slight random pitch and roll for natural variation
    SurfaceRotation.Pitch += FMath::RandRange(-15.0f, 15.0f);
    SurfaceRotation.Roll += FMath::RandRange(-15.0f, 15.0f);
    
    return SurfaceRotation;
}

UStaticMesh* UEnvArt_RockFormationSpawner::SelectRandomRockMesh(EBiomeType BiomeType) const
{
    const FEnvArt_RockFormationData* FormationData = &RockFormationData;
    if (BiomeSpecificRocks.Contains(BiomeType))
    {
        FormationData = &BiomeSpecificRocks[BiomeType];
    }
    
    if (FormationData->RockMeshes.Num() == 0)
    {
        // Fallback to engine default cube if no meshes are set
        return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    }
    
    int32 RandomIndex = FMath::RandRange(0, FormationData->RockMeshes.Num() - 1);
    TSoftObjectPtr<UStaticMesh> MeshPtr = FormationData->RockMeshes[RandomIndex];
    
    if (MeshPtr.IsValid())
    {
        return MeshPtr.Get();
    }
    
    // Load if not already loaded
    return MeshPtr.LoadSynchronous();
}

void UEnvArt_RockFormationSpawner::InitializeBiomeRockData()
{
    // Initialize biome-specific rock formations
    
    // Temperate Forest - medium sized rocks with moss
    FEnvArt_RockFormationData TemperateRocks;
    TemperateRocks.ScaleRange = FVector2D(1.0f, 2.0f);
    TemperateRocks.ClusterRadius = 400.0f;
    TemperateRocks.MinRocksPerCluster = 3;
    TemperateRocks.MaxRocksPerCluster = 6;
    TemperateRocks.SurfaceAlignmentStrength = 0.8f;
    BiomeSpecificRocks.Add(EBiomeType::TemperateForest, TemperateRocks);
    
    // Mountain - large angular rocks
    FEnvArt_RockFormationData MountainRocks;
    MountainRocks.ScaleRange = FVector2D(1.5f, 3.5f);
    MountainRocks.ClusterRadius = 600.0f;
    MountainRocks.MinRocksPerCluster = 2;
    MountainRocks.MaxRocksPerCluster = 5;
    MountainRocks.SurfaceAlignmentStrength = 0.6f;
    BiomeSpecificRocks.Add(EBiomeType::Mountain, MountainRocks);
    
    // Swampland - smaller weathered rocks
    FEnvArt_RockFormationData SwampRocks;
    SwampRocks.ScaleRange = FVector2D(0.5f, 1.5f);
    SwampRocks.ClusterRadius = 300.0f;
    SwampRocks.MinRocksPerCluster = 4;
    SwampRocks.MaxRocksPerCluster = 8;
    SwampRocks.SurfaceAlignmentStrength = 0.9f;
    BiomeSpecificRocks.Add(EBiomeType::Swampland, SwampRocks);
    
    // Desert - wind-carved rocks
    FEnvArt_RockFormationData DesertRocks;
    DesertRocks.ScaleRange = FVector2D(0.8f, 2.5f);
    DesertRocks.ClusterRadius = 500.0f;
    DesertRocks.MinRocksPerCluster = 2;
    DesertRocks.MaxRocksPerCluster = 4;
    DesertRocks.SurfaceAlignmentStrength = 0.5f;
    BiomeSpecificRocks.Add(EBiomeType::Desert, DesertRocks);
}