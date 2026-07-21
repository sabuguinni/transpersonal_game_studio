#include "TerrainPhysicsManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ATerrainPhysicsManager::ATerrainPhysicsManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize biome physics material paths
    BiomePhysicsMaterials.Add(TEXT("Savanna"), TEXT("/Game/Physics/Materials/PM_Savanna"));
    BiomePhysicsMaterials.Add(TEXT("Forest"), TEXT("/Game/Physics/Materials/PM_Forest"));
    BiomePhysicsMaterials.Add(TEXT("Swamp"), TEXT("/Game/Physics/Materials/PM_Swamp"));
    BiomePhysicsMaterials.Add(TEXT("Volcanic"), TEXT("/Game/Physics/Materials/PM_Volcanic"));
    BiomePhysicsMaterials.Add(TEXT("Coastal"), TEXT("/Game/Physics/Materials/PM_Coastal"));
}

void ATerrainPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Preload all physics materials at game start
    InitializeBiomePhysics();
}

void ATerrainPhysicsManager::InitializeBiomePhysics()
{
    LoadedPhysicsMaterials.Empty();

    for (const TPair<FString, FString>& BiomePair : BiomePhysicsMaterials)
    {
        UPhysicalMaterial* PhysMat = LoadObject<UPhysicalMaterial>(nullptr, *BiomePair.Value);
        if (PhysMat)
        {
            LoadedPhysicsMaterials.Add(BiomePair.Key, PhysMat);
            UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsManager: Loaded physics material for biome %s"), *BiomePair.Key);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsManager: Failed to load physics material for biome %s at path %s"), 
                *BiomePair.Key, *BiomePair.Value);
        }
    }
}

void ATerrainPhysicsManager::ApplyBiomePhysics(const FString& BiomeName, ALandscape* Landscape)
{
    if (!Landscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsManager: Invalid landscape provided"));
        return;
    }

    UPhysicalMaterial* PhysMat = GetBiomePhysicsMaterial(BiomeName);
    if (!PhysMat)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsManager: No physics material found for biome %s"), *BiomeName);
        return;
    }

    // Apply physics material to all landscape components
    TArray<ULandscapeComponent*> LandscapeComponents;
    Landscape->GetComponents<ULandscapeComponent>(LandscapeComponents);

    for (ULandscapeComponent* Component : LandscapeComponents)
    {
        if (Component)
        {
            Component->BodyInstance.SetPhysMaterialOverride(PhysMat);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsManager: Applied %s physics to landscape with %d components"), 
        *BiomeName, LandscapeComponents.Num());
}

UPhysicalMaterial* ATerrainPhysicsManager::GetBiomePhysicsMaterial(const FString& BiomeName) const
{
    if (const UPhysicalMaterial* const* FoundMat = LoadedPhysicsMaterials.Find(BiomeName))
    {
        return *FoundMat;
    }

    // Try to load on-demand if not cached
    if (const FString* PathPtr = BiomePhysicsMaterials.Find(BiomeName))
    {
        return LoadObject<UPhysicalMaterial>(nullptr, **PathPtr);
    }

    return nullptr;
}
