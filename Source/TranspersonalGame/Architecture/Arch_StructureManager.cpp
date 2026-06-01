#include "Arch_StructureManager.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AArch_StructureManager::AArch_StructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    MaxStructuresPerBiome = 50;
    StructureSpawnRadius = 15000.0f;
    bAutoGenerateStructures = false;

    // Initialize managed structures array
    ManagedStructures.Reserve(250); // 50 per biome * 5 biomes
}

void AArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoGenerateStructures)
    {
        // Generate structures for all biomes
        GenerateStructuresForBiome(EBiomeType::Savanna, 10);
        GenerateStructuresForBiome(EBiomeType::Swamp, 8);
        GenerateStructuresForBiome(EBiomeType::Forest, 12);
        GenerateStructuresForBiome(EBiomeType::Desert, 6);
        GenerateStructuresForBiome(EBiomeType::Mountain, 15);

        UE_LOG(LogTemp, Warning, TEXT("Architecture Manager: Auto-generated structures for all biomes"));
    }
}

void AArch_StructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Periodic validation of structure health and state
    static float ValidationTimer = 0.0f;
    ValidationTimer += DeltaTime;

    if (ValidationTimer >= 30.0f) // Validate every 30 seconds
    {
        ValidationTimer = 0.0f;

        // Check for destroyed structures and update array
        for (int32 i = ManagedStructures.Num() - 1; i >= 0; i--)
        {
            if (ManagedStructures[i].StructureHealth <= 0.0f)
            {
                ManagedStructures.RemoveAt(i);
            }
        }
    }
}

void AArch_StructureManager::SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Architecture Manager: No valid world for structure spawning"));
        return;
    }

    // Validate location
    EBiomeType BiomeType = EBiomeType::Savanna; // Default, should be determined by location
    ValidateStructurePlacement(Location, BiomeType);

    // Spawn static mesh actor
    AStaticMeshActor* StructureActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (!StructureActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Architecture Manager: Failed to spawn structure actor"));
        return;
    }

    // Set mesh based on structure type
    UStaticMesh* StructureMesh = GetMeshForStructureType(StructureType);
    if (StructureMesh && StructureActor->GetStaticMeshComponent())
    {
        StructureActor->GetStaticMeshComponent()->SetStaticMesh(StructureMesh);
    }

    // Set actor label
    FString StructureName = FString::Printf(TEXT("Arch_%s_%d"), 
        *UEnum::GetValueAsString(StructureType), ManagedStructures.Num());
    StructureActor->SetActorLabel(StructureName);

    // Create structure data
    FArch_StructureData NewStructure;
    NewStructure.StructureName = StructureName;
    NewStructure.BiomeType = BiomeType;
    NewStructure.Location = Location;
    NewStructure.Rotation = Rotation;
    NewStructure.StructureHealth = 100.0f;
    NewStructure.bIsRuin = (StructureType == EArch_StructureType::AncientRuin);

    ManagedStructures.Add(NewStructure);

    UE_LOG(LogTemp, Warning, TEXT("Architecture Manager: Spawned %s at %s"), 
        *StructureName, *Location.ToString());
}

void AArch_StructureManager::GenerateStructuresForBiome(EBiomeType BiomeType, int32 Count)
{
    if (Count <= 0 || Count > MaxStructuresPerBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("Architecture Manager: Invalid structure count %d for biome"), Count);
        return;
    }

    FVector BiomeCenter = GetBiomeCenterLocation(BiomeType);
    
    for (int32 i = 0; i < Count; i++)
    {
        // Random location within biome radius
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(1000.0f, StructureSpawnRadius);
        
        FVector SpawnLocation = BiomeCenter + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            100.0f
        );

        // Random rotation
        FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);

        // Choose structure type based on biome
        EArch_StructureType StructureType = EArch_StructureType::StonePillar;
        switch (BiomeType)
        {
        case EBiomeType::Mountain:
            StructureType = (i % 3 == 0) ? EArch_StructureType::CaveEntrance : EArch_StructureType::RockFormation;
            break;
        case EBiomeType::Forest:
            StructureType = (i % 2 == 0) ? EArch_StructureType::AncientRuin : EArch_StructureType::StoneArch;
            break;
        case EBiomeType::Desert:
            StructureType = EArch_StructureType::StonePillar;
            break;
        case EBiomeType::Swamp:
            StructureType = EArch_StructureType::AncientRuin;
            break;
        default:
            StructureType = EArch_StructureType::StoneWall;
            break;
        }

        SpawnStructureAtLocation(StructureType, SpawnLocation, SpawnRotation);
    }

    UE_LOG(LogTemp, Warning, TEXT("Architecture Manager: Generated %d structures for %s biome"), 
        Count, *UEnum::GetValueAsString(BiomeType));
}

void AArch_StructureManager::ClearStructuresInRadius(FVector Center, float Radius)
{
    if (!GetWorld())
    {
        return;
    }

    int32 ClearedCount = 0;
    for (int32 i = ManagedStructures.Num() - 1; i >= 0; i--)
    {
        float Distance = FVector::Dist(ManagedStructures[i].Location, Center);
        if (Distance <= Radius)
        {
            // Find and destroy the actual actor
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);
            
            for (AActor* Actor : FoundActors)
            {
                if (Actor->GetActorLabel().Contains(ManagedStructures[i].StructureName))
                {
                    Actor->Destroy();
                    ClearedCount++;
                    break;
                }
            }

            ManagedStructures.RemoveAt(i);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Architecture Manager: Cleared %d structures in radius %.1f"), 
        ClearedCount, Radius);
}

TArray<FArch_StructureData> AArch_StructureManager::GetStructuresInBiome(EBiomeType BiomeType)
{
    TArray<FArch_StructureData> BiomeStructures;
    
    for (const FArch_StructureData& Structure : ManagedStructures)
    {
        if (Structure.BiomeType == BiomeType)
        {
            BiomeStructures.Add(Structure);
        }
    }

    return BiomeStructures;
}

void AArch_StructureManager::UpdateStructureHealth(int32 StructureIndex, float NewHealth)
{
    if (ManagedStructures.IsValidIndex(StructureIndex))
    {
        ManagedStructures[StructureIndex].StructureHealth = FMath::Clamp(NewHealth, 0.0f, 100.0f);
        
        if (NewHealth <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Architecture Manager: Structure %s destroyed"), 
                *ManagedStructures[StructureIndex].StructureName);
        }
    }
}

void AArch_StructureManager::RegenerateAllStructures()
{
    // Clear existing structures
    ClearStructuresInRadius(FVector::ZeroVector, 100000.0f);

    // Regenerate for all biomes
    GenerateStructuresForBiome(EBiomeType::Savanna, 10);
    GenerateStructuresForBiome(EBiomeType::Swamp, 8);
    GenerateStructuresForBiome(EBiomeType::Forest, 12);
    GenerateStructuresForBiome(EBiomeType::Desert, 6);
    GenerateStructuresForBiome(EBiomeType::Mountain, 15);

    UE_LOG(LogTemp, Warning, TEXT("Architecture Manager: Regenerated all structures"));
}

void AArch_StructureManager::OnStructureDestroyed(AActor* DestroyedActor)
{
    if (!DestroyedActor)
    {
        return;
    }

    // Remove from managed structures array
    for (int32 i = ManagedStructures.Num() - 1; i >= 0; i--)
    {
        if (DestroyedActor->GetActorLabel().Contains(ManagedStructures[i].StructureName))
        {
            ManagedStructures.RemoveAt(i);
            break;
        }
    }
}

FVector AArch_StructureManager::GetBiomeCenterLocation(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
    case EBiomeType::Savanna:
        return FVector(0.0f, 0.0f, 0.0f);
    case EBiomeType::Swamp:
        return FVector(-50000.0f, -45000.0f, 0.0f);
    case EBiomeType::Forest:
        return FVector(-45000.0f, 40000.0f, 0.0f);
    case EBiomeType::Desert:
        return FVector(55000.0f, 0.0f, 0.0f);
    case EBiomeType::Mountain:
        return FVector(40000.0f, 50000.0f, 0.0f);
    default:
        return FVector::ZeroVector;
    }
}

UStaticMesh* AArch_StructureManager::GetMeshForStructureType(EArch_StructureType StructureType)
{
    // Return nullptr for now - meshes should be set via Blueprint or loaded from Content
    // This allows for easy asset swapping without recompilation
    return nullptr;
}

void AArch_StructureManager::ValidateStructurePlacement(FVector& Location, EBiomeType BiomeType)
{
    // Ensure minimum height above ground
    Location.Z = FMath::Max(Location.Z, 50.0f);

    // Clamp to biome boundaries
    FVector BiomeCenter = GetBiomeCenterLocation(BiomeType);
    float MaxDistance = StructureSpawnRadius;

    FVector Offset = Location - BiomeCenter;
    if (Offset.Size() > MaxDistance)
    {
        Offset = Offset.GetSafeNormal() * MaxDistance;
        Location = BiomeCenter + Offset;
    }
}