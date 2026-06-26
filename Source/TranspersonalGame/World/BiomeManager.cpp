// BiomeManager.cpp — Engine Architect #02 — Cycle AUTO_009
// Manages biome regions, assigns dinosaur species to biomes,
// and provides query functions for world generation systems.
// P1 priority: World Generation backbone.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;  // Biome data is static — no per-frame update needed

    // Default biome configuration for the MinPlayableMap
    // Centered around the dino zone (2000, 2000)
    InitializeDefaultBiomes();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    if (Biomes.Num() == 0)
    {
        InitializeDefaultBiomes();
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biomes"), Biomes.Num());
}

void ABiomeManager::InitializeDefaultBiomes()
{
    Biomes.Empty();

    // Biome 1: Dense Jungle (center — main play area)
    FBiomeData Jungle;
    Jungle.BiomeType = EBiomeType::Forest;
    Jungle.BiomeCenter = FVector2D(2000.0f, 2000.0f);
    Jungle.BiomeRadius = 3000.0f;
    Jungle.NativeDinosaurs.Add(EDinosaurSpecies::Velociraptor);
    Jungle.NativeDinosaurs.Add(EDinosaurSpecies::Compsognathus);
    Jungle.NativeDinosaurs.Add(EDinosaurSpecies::Parasaurolophus);
    Biomes.Add(Jungle);

    // Biome 2: Open Savanna (north — large herbivores)
    FBiomeData Savanna;
    Savanna.BiomeType = EBiomeType::Savanna;
    Savanna.BiomeCenter = FVector2D(2000.0f, -1000.0f);
    Savanna.BiomeRadius = 4000.0f;
    Savanna.NativeDinosaurs.Add(EDinosaurSpecies::Triceratops);
    Savanna.NativeDinosaurs.Add(EDinosaurSpecies::Brachiosaurus);
    Savanna.NativeDinosaurs.Add(EDinosaurSpecies::Stegosaurus);
    Biomes.Add(Savanna);

    // Biome 3: Swamp (south — dangerous)
    FBiomeData Swamp;
    Swamp.BiomeType = EBiomeType::Swamp;
    Swamp.BiomeCenter = FVector2D(2000.0f, 5000.0f);
    Swamp.BiomeRadius = 2500.0f;
    Swamp.NativeDinosaurs.Add(EDinosaurSpecies::TRex);
    Swamp.NativeDinosaurs.Add(EDinosaurSpecies::Ankylosaurus);
    Biomes.Add(Swamp);

    // Biome 4: Mountain Canyon (west — exploration)
    FBiomeData Canyon;
    Canyon.BiomeType = EBiomeType::Canyon;
    Canyon.BiomeCenter = FVector2D(-2000.0f, 2000.0f);
    Canyon.BiomeRadius = 3500.0f;
    Canyon.NativeDinosaurs.Add(EDinosaurSpecies::Velociraptor);
    Canyon.NativeDinosaurs.Add(EDinosaurSpecies::TRex);
    Biomes.Add(Canyon);
}

EBiomeType ABiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);
    float ClosestDist = TNumericLimits<float>::Max();
    EBiomeType ClosestBiome = EBiomeType::Forest;

    for (const FBiomeData& Biome : Biomes)
    {
        float Dist = FVector2D::Distance(Loc2D, Biome.BiomeCenter);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Biome.BiomeType;
        }
    }

    return ClosestBiome;
}

TArray<EDinosaurSpecies> ABiomeManager::GetDinosaursForBiome(EBiomeType BiomeType) const
{
    for (const FBiomeData& Biome : Biomes)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome.NativeDinosaurs;
        }
    }
    return TArray<EDinosaurSpecies>();
}

FBiomeData ABiomeManager::GetBiomeDataAtLocation(FVector WorldLocation) const
{
    FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);
    float ClosestDist = TNumericLimits<float>::Max();
    int32 ClosestIdx = 0;

    for (int32 i = 0; i < Biomes.Num(); ++i)
    {
        float Dist = FVector2D::Distance(Loc2D, Biomes[i].BiomeCenter);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestIdx = i;
        }
    }

    return Biomes.IsValidIndex(ClosestIdx) ? Biomes[ClosestIdx] : FBiomeData();
}

bool ABiomeManager::IsLocationInBiome(FVector WorldLocation, EBiomeType BiomeType) const
{
    FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);

    for (const FBiomeData& Biome : Biomes)
    {
        if (Biome.BiomeType == BiomeType)
        {
            float Dist = FVector2D::Distance(Loc2D, Biome.BiomeCenter);
            if (Dist <= Biome.BiomeRadius)
            {
                return true;
            }
        }
    }
    return false;
}

int32 ABiomeManager::GetBiomeCount() const
{
    return Biomes.Num();
}

void ABiomeManager::AddBiome(FBiomeData NewBiome)
{
    Biomes.Add(NewBiome);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Added biome. Total: %d"), Biomes.Num());
}
