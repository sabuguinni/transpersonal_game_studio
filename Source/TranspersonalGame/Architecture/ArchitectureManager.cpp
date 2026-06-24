#include "ArchitectureManager.h"

AArchitectureManager::AArchitectureManager()
{
    PrimaryActorTick.bCanEverTick = false;
    TotalStructuresSpawned = 0;
}

void AArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
}

void AArchitectureManager::RegisterStructure(const FArch_StructureData& StructureData)
{
    RegisteredStructures.Add(StructureData);
    TotalStructuresSpawned++;
}

int32 AArchitectureManager::GetStructureCount() const
{
    return RegisteredStructures.Num();
}

void AArchitectureManager::ClearAllStructures()
{
    RegisteredStructures.Empty();
    TotalStructuresSpawned = 0;
}
