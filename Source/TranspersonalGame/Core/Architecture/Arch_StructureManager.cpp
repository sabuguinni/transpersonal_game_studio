#include "Arch_StructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"

void UArch_StructureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture Structure Manager Initialized"));
    
    SpawnedStructures.Empty();
    StructureActors.Empty();
}

void UArch_StructureManager::Deinitialize()
{
    for (AActor* Actor : StructureActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    SpawnedStructures.Empty();
    StructureActors.Empty();
    
    Super::Deinitialize();
}

void UArch_StructureManager::SpawnStructureAtBiome(EBiomeType BiomeType, EArch_StructureType StructureType, int32 Count)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found for structure spawning"));
        return;
    }

    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeType, 3000.0f);
        FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        switch (StructureType)
        {
            case EArch_StructureType::Pillar:
                CreatePrimitivePillar(SpawnLocation, SpawnRotation);
                break;
            case EArch_StructureType::Shelter:
                CreatePrimitiveShelter(SpawnLocation, SpawnRotation);
                break;
            case EArch_StructureType::Ruin:
                CreateStoneRuin(SpawnLocation, SpawnRotation);
                break;
            default:
                CreatePrimitivePillar(SpawnLocation, SpawnRotation);
                break;
        }
        
        // Record structure data
        FArch_StructureData NewStructure;
        NewStructure.StructureName = FString::Printf(TEXT("%s_%s_%d"), 
            *UEnum::GetValueAsString(StructureType),
            *UEnum::GetValueAsString(BiomeType), 
            i);
        NewStructure.Location = SpawnLocation;
        NewStructure.Rotation = SpawnRotation;
        NewStructure.BiomeType = BiomeType;
        NewStructure.StructureHealth = FMath::RandRange(50.0f, 100.0f);
        NewStructure.bIsRuin = (StructureType == EArch_StructureType::Ruin);
        
        SpawnedStructures.Add(NewStructure);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d structures of type %s in biome %s"), 
        Count, 
        *UEnum::GetValueAsString(StructureType),
        *UEnum::GetValueAsString(BiomeType));
}

void UArch_StructureManager::SpawnPillarCluster(FVector CenterLocation, int32 PillarCount)
{
    float ClusterRadius = 500.0f;
    
    for (int32 i = 0; i < PillarCount; i++)
    {
        float Angle = (2.0f * PI * i) / PillarCount;
        float Distance = FMath::RandRange(100.0f, ClusterRadius);
        
        FVector PillarLocation = CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FRotator PillarRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        CreatePrimitivePillar(PillarLocation, PillarRotation);
    }
}

void UArch_StructureManager::SpawnShelterInterior(FVector Location, bool bIncludeFurniture)
{
    CreatePrimitiveShelter(Location, FRotator::ZeroRotator);
    
    if (bIncludeFurniture)
    {
        // Spawn interior elements around the shelter
        FVector InteriorCenter = Location + FVector(0.0f, 0.0f, 50.0f);
        
        // Fire pit in center
        CreatePrimitivePillar(InteriorCenter, FRotator::ZeroRotator);
        
        // Sleeping area
        FVector SleepingArea = InteriorCenter + FVector(200.0f, 0.0f, 0.0f);
        CreatePrimitivePillar(SleepingArea, FRotator::ZeroRotator);
        
        // Storage area
        FVector StorageArea = InteriorCenter + FVector(-200.0f, 150.0f, 0.0f);
        CreatePrimitivePillar(StorageArea, FRotator::ZeroRotator);
    }
}

TArray<AActor*> UArch_StructureManager::GetStructuresInRadius(FVector Center, float Radius)
{
    TArray<AActor*> NearbyStructures;
    
    for (AActor* Actor : StructureActors)
    {
        if (IsValid(Actor))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                NearbyStructures.Add(Actor);
            }
        }
    }
    
    return NearbyStructures;
}

void UArch_StructureManager::PopulateAllBiomesWithStructures()
{
    // Savana - Stone pillars and shelters
    SpawnStructureAtBiome(EBiomeType::Savana, EArch_StructureType::Pillar, 8);
    SpawnStructureAtBiome(EBiomeType::Savana, EArch_StructureType::Shelter, 3);
    
    // Floresta - Ruins and platforms
    SpawnStructureAtBiome(EBiomeType::Floresta, EArch_StructureType::Ruin, 5);
    SpawnStructureAtBiome(EBiomeType::Floresta, EArch_StructureType::Platform, 4);
    
    // Pantano - Elevated shelters
    SpawnStructureAtBiome(EBiomeType::Pantano, EArch_StructureType::Shelter, 6);
    SpawnStructureAtBiome(EBiomeType::Pantano, EArch_StructureType::Platform, 3);
    
    // Deserto - Ancient ruins
    SpawnStructureAtBiome(EBiomeType::Deserto, EArch_StructureType::Ruin, 7);
    SpawnStructureAtBiome(EBiomeType::Deserto, EArch_StructureType::Pillar, 5);
    
    // Montanha - Stone bridges and platforms
    SpawnStructureAtBiome(EBiomeType::Montanha, EArch_StructureType::Bridge, 4);
    SpawnStructureAtBiome(EBiomeType::Montanha, EArch_StructureType::Platform, 6);
    
    UE_LOG(LogTemp, Warning, TEXT("Populated all biomes with architectural structures"));
}

void UArch_StructureManager::CreatePrimitivePillar(FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    AStaticMeshActor* PillarActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (PillarActor)
    {
        UStaticMeshComponent* MeshComp = PillarActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Use engine primitive for now - will be replaced by Meshy assets
            UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            if (CylinderMesh)
            {
                MeshComp->SetStaticMesh(CylinderMesh);
                MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 3.0f)); // Tall pillar
            }
        }
        
        PillarActor->SetActorLabel(TEXT("Arch_StonePillar"));
        StructureActors.Add(PillarActor);
    }
}

void UArch_StructureManager::CreatePrimitiveShelter(FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    AStaticMeshActor* ShelterActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (ShelterActor)
    {
        UStaticMeshComponent* MeshComp = ShelterActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Use engine primitive for now - will be replaced by Meshy assets
            UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            if (CubeMesh)
            {
                MeshComp->SetStaticMesh(CubeMesh);
                MeshComp->SetWorldScale3D(FVector(4.0f, 4.0f, 2.0f)); // Shelter size
            }
        }
        
        ShelterActor->SetActorLabel(TEXT("Arch_PrimitiveShelter"));
        StructureActors.Add(ShelterActor);
    }
}

void UArch_StructureManager::CreateStoneRuin(FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    AStaticMeshActor* RuinActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (RuinActor)
    {
        UStaticMeshComponent* MeshComp = RuinActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Use engine primitive for now - will be replaced by Meshy assets
            UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            if (CubeMesh)
            {
                MeshComp->SetStaticMesh(CubeMesh);
                MeshComp->SetWorldScale3D(FVector(3.0f, 2.0f, 1.5f)); // Broken ruin size
            }
        }
        
        RuinActor->SetActorLabel(TEXT("Arch_AncientRuin"));
        StructureActors.Add(RuinActor);
    }
}

FVector UArch_StructureManager::GetBiomeCenter(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            return FVector(0.0f, 0.0f, 100.0f);
        case EBiomeType::Floresta:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case EBiomeType::Pantano:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        case EBiomeType::Deserto:
            return FVector(55000.0f, 0.0f, 100.0f);
        case EBiomeType::Montanha:
            return FVector(40000.0f, 50000.0f, 100.0f);
        default:
            return FVector::ZeroVector;
    }
}

FVector UArch_StructureManager::GetRandomLocationInBiome(EBiomeType BiomeType, float Radius)
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(100.0f, Radius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return BiomeCenter + RandomOffset;
}