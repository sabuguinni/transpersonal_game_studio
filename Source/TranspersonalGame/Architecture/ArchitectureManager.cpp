#include "ArchitectureManager.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AArch_ArchitectureManager::AArch_ArchitectureManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AArch_ArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeCretaceousStructures();
}

void AArch_ArchitectureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AArch_ArchitectureManager::RegisterStructure(FArch_StructureData StructureData)
{
    if (RegisteredStructures.Num() < MaxStructureCount)
    {
        RegisteredStructures.Add(StructureData);
    }
}

TArray<FArch_StructureData> AArch_ArchitectureManager::GetStructuresInRadius(FVector Center, float Radius) const
{
    TArray<FArch_StructureData> Result;
    for (const FArch_StructureData& Structure : RegisteredStructures)
    {
        float Distance = FVector::Dist(Center, Structure.WorldLocation);
        if (Distance <= Radius)
        {
            Result.Add(Structure);
        }
    }
    return Result;
}

void AArch_ArchitectureManager::SpawnRuinPillarAtBiomeCoords()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FVector SpawnLocation(BiomeX, BiomeY, BiomeZ);
    FRotator SpawnRotation(0.0f, 0.0f, 0.0f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AStaticMeshActor* PillarActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (PillarActor)
    {
        PillarActor->SetActorLabel(TEXT("Arch_CretaceousRuinPillar_Biome"));
        PillarActor->SetActorScale3D(FVector(0.5f, 0.5f, 3.0f));

        // Register this structure
        FArch_StructureData NewStructure;
        NewStructure.StructureType = EArch_StructureType::RuinPillar;
        NewStructure.DecayState = EArch_DecayState::Weathered;
        NewStructure.WorldLocation = SpawnLocation;
        NewStructure.HeightMeters = 3.0f;
        NewStructure.bHasMossGrowth = true;
        NewStructure.bHasPrehistoricFerns = true;
        NewStructure.MeshyAssetURL = CretaceousRuinPillarURL;
        RegisteredStructures.Add(NewStructure);
    }
}

void AArch_ArchitectureManager::InitializeCretaceousStructures()
{
    // Seed default Cretaceous structures around the biome center
    const TArray<FVector> DefaultLocations = {
        FVector(50000.0f, 50000.0f, 100.0f),
        FVector(50500.0f, 50200.0f, 100.0f),
        FVector(49800.0f, 50700.0f, 100.0f),
        FVector(50300.0f, 49500.0f, 100.0f),
        FVector(51000.0f, 50800.0f, 100.0f)
    };

    const TArray<EArch_StructureType> DefaultTypes = {
        EArch_StructureType::RuinPillar,
        EArch_StructureType::BoulderCluster,
        EArch_StructureType::StoneWall,
        EArch_StructureType::RockyOutcrop,
        EArch_StructureType::RuinPillar
    };

    for (int32 i = 0; i < DefaultLocations.Num(); ++i)
    {
        FArch_StructureData Structure;
        Structure.StructureType = DefaultTypes[i];
        Structure.DecayState = EArch_DecayState::Weathered;
        Structure.WorldLocation = DefaultLocations[i];
        Structure.HeightMeters = 3.0f;
        Structure.bHasMossGrowth = true;
        Structure.bHasPrehistoricFerns = true;
        Structure.MeshyAssetURL = CretaceousRuinPillarURL;
        RegisteredStructures.Add(Structure);
    }
}
