#include "Arch_StructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMesh.h"

AArch_StructureManager::AArch_StructureManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    bAutoDistributeOnBeginPlay = true;
    StructuresPerBiome = 5;
    MinDistanceBetweenStructures = 2000.0f;
}

void AArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeStructureMeshes();
    
    if (bAutoDistributeOnBeginPlay)
    {
        DistributeStructuresAcrossBiomes();
    }
}

void AArch_StructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AArch_StructureManager::SpawnStructureAtBiome(EArch_StructureType StructureType, EBiomeType BiomeType, int32 Count)
{
    for (int32 i = 0; i < Count; i++)
    {
        FArch_StructureSpawnData SpawnData;
        SpawnData.StructureType = StructureType;
        SpawnData.BiomeType = BiomeType;
        SpawnData.SpawnLocation = GetRandomLocationInBiome(BiomeType);
        SpawnData.SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        SpawnData.Scale = FVector(FMath::RandRange(0.8f, 1.2f));
        
        SpawnStructureAtLocation(SpawnData);
    }
}

void AArch_StructureManager::SpawnStructureAtLocation(const FArch_StructureSpawnData& SpawnData)
{
    if (!IsLocationValid(SpawnData.SpawnLocation, MinDistanceBetweenStructures))
    {
        return;
    }
    
    UStaticMesh* Mesh = GetStructureMesh(SpawnData.StructureType);
    if (!Mesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("No mesh found for structure type"));
        return;
    }
    
    AActor* SpawnedActor = SpawnStructureMesh(Mesh, SpawnData.SpawnLocation, SpawnData.SpawnRotation, SpawnData.Scale);
    if (SpawnedActor)
    {
        SpawnedStructures.Add(SpawnedActor);
        
        // Set appropriate label
        FString BiomeName = UEnum::GetValueAsString(SpawnData.BiomeType);
        FString StructureName = UEnum::GetValueAsString(SpawnData.StructureType);
        FString Label = FString::Printf(TEXT("%s_%s_%d"), *StructureName, *BiomeName, SpawnedStructures.Num());
        SpawnedActor->SetActorLabel(Label);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned structure: %s at %s"), *Label, *SpawnData.SpawnLocation.ToString());
    }
}

void AArch_StructureManager::DistributeStructuresAcrossBiomes()
{
    // Clear existing structures
    ClearAllStructures();
    
    // Spawn structures in each biome
    TArray<EBiomeType> Biomes = {
        EBiomeType::Savana,
        EBiomeType::Pantano,
        EBiomeType::Floresta,
        EBiomeType::Deserto,
        EBiomeType::Montanha
    };
    
    for (EBiomeType Biome : Biomes)
    {
        // Distribute different structure types per biome
        switch (Biome)
        {
            case EBiomeType::Savana:
                SpawnStructureAtBiome(EArch_StructureType::StonePillar, Biome, 2);
                SpawnStructureAtBiome(EArch_StructureType::TribalShelter, Biome, 3);
                break;
                
            case EBiomeType::Pantano:
                SpawnStructureAtBiome(EArch_StructureType::RockFormation, Biome, 3);
                SpawnStructureAtBiome(EArch_StructureType::AncientRuins, Biome, 2);
                break;
                
            case EBiomeType::Floresta:
                SpawnStructureAtBiome(EArch_StructureType::TribalShelter, Biome, 4);
                SpawnStructureAtBiome(EArch_StructureType::CaveEntrance, Biome, 1);
                break;
                
            case EBiomeType::Deserto:
                SpawnStructureAtBiome(EArch_StructureType::AncientRuins, Biome, 3);
                SpawnStructureAtBiome(EArch_StructureType::RockFormation, Biome, 2);
                break;
                
            case EBiomeType::Montanha:
                SpawnStructureAtBiome(EArch_StructureType::CaveEntrance, Biome, 2);
                SpawnStructureAtBiome(EArch_StructureType::StonePillar, Biome, 3);
                break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Distributed %d structures across all biomes"), SpawnedStructures.Num());
}

TArray<AActor*> AArch_StructureManager::GetStructuresInBiome(EBiomeType BiomeType)
{
    TArray<AActor*> BiomeStructures;
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    float BiomeRadius = 20000.0f; // 20km radius
    
    for (AActor* Structure : SpawnedStructures)
    {
        if (Structure && IsValid(Structure))
        {
            float Distance = FVector::Dist(Structure->GetActorLocation(), BiomeCenter);
            if (Distance <= BiomeRadius)
            {
                BiomeStructures.Add(Structure);
            }
        }
    }
    
    return BiomeStructures;
}

void AArch_StructureManager::ClearAllStructures()
{
    for (AActor* Structure : SpawnedStructures)
    {
        if (Structure && IsValid(Structure))
        {
            Structure->Destroy();
        }
    }
    SpawnedStructures.Empty();
}

FVector AArch_StructureManager::GetBiomeCenter(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savana:    return FVector(0, 0, 0);
        case EBiomeType::Pantano:   return FVector(-50000, -45000, 0);
        case EBiomeType::Floresta:  return FVector(-45000, 40000, 0);
        case EBiomeType::Deserto:   return FVector(55000, 0, 0);
        case EBiomeType::Montanha:  return FVector(40000, 50000, 0);
        default:                    return FVector::ZeroVector;
    }
}

FVector AArch_StructureManager::GetRandomLocationInBiome(EBiomeType BiomeType, float Radius)
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    
    float RandomX = FMath::RandRange(-Radius, Radius);
    float RandomY = FMath::RandRange(-Radius, Radius);
    float RandomZ = FMath::RandRange(50.0f, 200.0f); // Slight height variation
    
    return BiomeCenter + FVector(RandomX, RandomY, RandomZ);
}

AActor* AArch_StructureManager::SpawnStructureMesh(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    if (!Mesh || !GetWorld())
    {
        return nullptr;
    }
    
    // Spawn StaticMeshActor
    AStaticMeshActor* MeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation);
    if (MeshActor)
    {
        UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
        if (MeshComponent)
        {
            MeshComponent->SetStaticMesh(Mesh);
            MeshComponent->SetWorldScale3D(Scale);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        }
    }
    
    return MeshActor;
}

bool AArch_StructureManager::IsLocationValid(const FVector& Location, float MinDistance)
{
    for (AActor* ExistingStructure : SpawnedStructures)
    {
        if (ExistingStructure && IsValid(ExistingStructure))
        {
            float Distance = FVector::Dist(Location, ExistingStructure->GetActorLocation());
            if (Distance < MinDistance)
            {
                return false;
            }
        }
    }
    return true;
}

UStaticMesh* AArch_StructureManager::GetStructureMesh(EArch_StructureType StructureType)
{
    if (StructureMeshes.Contains(StructureType))
    {
        TSoftObjectPtr<UStaticMesh> MeshPtr = StructureMeshes[StructureType];
        if (MeshPtr.IsValid())
        {
            return MeshPtr.Get();
        }
        else if (!MeshPtr.IsNull())
        {
            return MeshPtr.LoadSynchronous();
        }
    }
    
    // Fallback to engine default cube
    return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
}

void AArch_StructureManager::InitializeStructureMeshes()
{
    // Initialize with default engine meshes as fallbacks
    // These can be replaced with proper assets when available
    StructureMeshes.Add(EArch_StructureType::StonePillar, 
        TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cylinder"))));
    
    StructureMeshes.Add(EArch_StructureType::RockFormation, 
        TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube"))));
    
    StructureMeshes.Add(EArch_StructureType::CaveEntrance, 
        TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Sphere"))));
    
    StructureMeshes.Add(EArch_StructureType::TribalShelter, 
        TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Wedge"))));
    
    StructureMeshes.Add(EArch_StructureType::AncientRuins, 
        TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube"))));
}