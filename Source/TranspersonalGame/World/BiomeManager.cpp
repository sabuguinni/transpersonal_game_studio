// BiomeManager.cpp
// Engine Architect #02 — P1 World Generation
// Full implementation of ABiomeManager — biome detection, transitions, runtime state.

#include "BiomeManager.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

// -------------------------------------------------------
// Constructor
// -------------------------------------------------------
ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    CurrentBiomeType    = EBiomeType::Plains;
    PreviousBiomeType   = EBiomeType::Plains;
    TransitionProgress  = 0.0f;
    TransitionSpeed     = 1.0f;
    bIsTransitioning    = false;
    BiomeTransitionBlendRadius = 500.0f;

    // Populate default biome definitions so the CDO is never empty
    // Plains
    {
        FEng_BiomeDefinition Plains;
        Plains.BiomeType                    = EBiomeType::Plains;
        Plains.BaseTemperature              = 22.0f;
        Plains.BaseHumidity                 = 0.45f;
        Plains.FogDensity                   = 0.005f;
        Plains.FogColor                     = FLinearColor(0.55f, 0.65f, 0.75f, 1.0f);
        Plains.AmbientColor                 = FLinearColor(0.08f, 0.09f, 0.10f, 1.0f);
        Plains.bAllowsDinosaurSpawn         = true;
        Plains.DinosaurDensityMultiplier    = 1.0f;
        Plains.VegetationDensityMultiplier  = 1.0f;
        BiomeDefinitions.Add(Plains);
    }

    // Forest
    {
        FEng_BiomeDefinition Forest;
        Forest.BiomeType                    = EBiomeType::Forest;
        Forest.BaseTemperature              = 18.0f;
        Forest.BaseHumidity                 = 0.70f;
        Forest.FogDensity                   = 0.02f;
        Forest.FogColor                     = FLinearColor(0.40f, 0.55f, 0.40f, 1.0f);
        Forest.AmbientColor                 = FLinearColor(0.05f, 0.08f, 0.05f, 1.0f);
        Forest.bAllowsDinosaurSpawn         = true;
        Forest.DinosaurDensityMultiplier    = 1.4f;
        Forest.VegetationDensityMultiplier  = 2.5f;
        BiomeDefinitions.Add(Forest);
    }

    // Desert
    {
        FEng_BiomeDefinition Desert;
        Desert.BiomeType                    = EBiomeType::Desert;
        Desert.BaseTemperature              = 38.0f;
        Desert.BaseHumidity                 = 0.10f;
        Desert.FogDensity                   = 0.002f;
        Desert.FogColor                     = FLinearColor(0.80f, 0.70f, 0.50f, 1.0f);
        Desert.AmbientColor                 = FLinearColor(0.15f, 0.12f, 0.06f, 1.0f);
        Desert.bAllowsDinosaurSpawn         = true;
        Desert.DinosaurDensityMultiplier    = 0.6f;
        Desert.VegetationDensityMultiplier  = 0.2f;
        BiomeDefinitions.Add(Desert);
    }

    // Swamp
    {
        FEng_BiomeDefinition Swamp;
        Swamp.BiomeType                    = EBiomeType::Swamp;
        Swamp.BaseTemperature              = 25.0f;
        Swamp.BaseHumidity                 = 0.90f;
        Swamp.FogDensity                   = 0.05f;
        Swamp.FogColor                     = FLinearColor(0.30f, 0.40f, 0.30f, 1.0f);
        Swamp.AmbientColor                 = FLinearColor(0.04f, 0.06f, 0.04f, 1.0f);
        Swamp.bAllowsDinosaurSpawn         = true;
        Swamp.DinosaurDensityMultiplier    = 1.8f;
        Swamp.VegetationDensityMultiplier  = 3.0f;
        BiomeDefinitions.Add(Swamp);
    }

    // Mountain
    {
        FEng_BiomeDefinition Mountain;
        Mountain.BiomeType                    = EBiomeType::Mountain;
        Mountain.BaseTemperature              = 5.0f;
        Mountain.BaseHumidity                 = 0.35f;
        Mountain.FogDensity                   = 0.03f;
        Mountain.FogColor                     = FLinearColor(0.70f, 0.75f, 0.85f, 1.0f);
        Mountain.AmbientColor                 = FLinearColor(0.10f, 0.10f, 0.12f, 1.0f);
        Mountain.bAllowsDinosaurSpawn         = true;
        Mountain.DinosaurDensityMultiplier    = 0.8f;
        Mountain.VegetationDensityMultiplier  = 0.5f;
        BiomeDefinitions.Add(Mountain);
    }

    // Volcano
    {
        FEng_BiomeDefinition Volcano;
        Volcano.BiomeType                    = EBiomeType::Volcano;
        Volcano.BaseTemperature              = 60.0f;
        Volcano.BaseHumidity                 = 0.05f;
        Volcano.FogDensity                   = 0.08f;
        Volcano.FogColor                     = FLinearColor(0.60f, 0.30f, 0.10f, 1.0f);
        Volcano.AmbientColor                 = FLinearColor(0.20f, 0.08f, 0.02f, 1.0f);
        Volcano.bAllowsDinosaurSpawn         = false;
        Volcano.DinosaurDensityMultiplier    = 0.0f;
        Volcano.VegetationDensityMultiplier  = 0.0f;
        BiomeDefinitions.Add(Volcano);
    }

    // Initialise runtime data array to match definitions count
    ActiveBiomeData.SetNum(BiomeDefinitions.Num());
    for (int32 i = 0; i < BiomeDefinitions.Num(); ++i)
    {
        ActiveBiomeData[i].BiomeType           = BiomeDefinitions[i].BiomeType;
        ActiveBiomeData[i].CurrentTemperature  = BiomeDefinitions[i].BaseTemperature;
        ActiveBiomeData[i].CurrentHumidity     = BiomeDefinitions[i].BaseHumidity;
        ActiveBiomeData[i].ActiveDinosaurCount = 0;
        ActiveBiomeData[i].bIsActive           = false;
    }
}

// -------------------------------------------------------
// BeginPlay
// -------------------------------------------------------
void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    // Mark the starting biome as active
    for (FEng_BiomeRuntimeData& Data : ActiveBiomeData)
    {
        if (Data.BiomeType == CurrentBiomeType)
        {
            Data.bIsActive = true;
            break;
        }
    }
}

// -------------------------------------------------------
// Tick — advance biome transition blend
// -------------------------------------------------------
void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsTransitioning)
    {
        return;
    }

    TransitionProgress += DeltaTime * TransitionSpeed;

    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress  = 1.0f;
        bIsTransitioning    = false;

        // Deactivate old biome, activate new
        for (FEng_BiomeRuntimeData& Data : ActiveBiomeData)
        {
            if (Data.BiomeType == PreviousBiomeType) { Data.bIsActive = false; }
            if (Data.BiomeType == CurrentBiomeType)  { Data.bIsActive = true;  }
        }

        OnBiomeChanged.Broadcast(PreviousBiomeType, CurrentBiomeType);
    }
}

// -------------------------------------------------------
// GetBiomeAtLocation
// Simple height-based heuristic for the MinPlayableMap.
// A full implementation would sample a biome weight map.
// -------------------------------------------------------
EBiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    const float Z = WorldLocation.Z;

    if (Z > 1500.0f) { return EBiomeType::Volcano;  }
    if (Z > 800.0f)  { return EBiomeType::Mountain; }

    // Distance from origin determines biome ring
    const float Dist = FVector2D(WorldLocation.X, WorldLocation.Y).Size();

    if (Dist < 1000.0f)  { return EBiomeType::Plains;  }
    if (Dist < 2500.0f)  { return EBiomeType::Forest;  }
    if (Dist < 4000.0f)  { return EBiomeType::Swamp;   }
    return EBiomeType::Desert;
}

// -------------------------------------------------------
// GetBiomeDefinition
// -------------------------------------------------------
FEng_BiomeDefinition ABiomeManager::GetBiomeDefinition(EBiomeType BiomeType) const
{
    for (const FEng_BiomeDefinition& Def : BiomeDefinitions)
    {
        if (Def.BiomeType == BiomeType)
        {
            return Def;
        }
    }

    // Return default (Plains) if not found
    return FEng_BiomeDefinition();
}

// -------------------------------------------------------
// RequestBiomeTransition
// -------------------------------------------------------
void ABiomeManager::RequestBiomeTransition(EBiomeType NewBiome)
{
    if (NewBiome == CurrentBiomeType)
    {
        return; // Already in this biome
    }

    PreviousBiomeType  = CurrentBiomeType;
    CurrentBiomeType   = NewBiome;
    TransitionProgress = 0.0f;
    bIsTransitioning   = true;
}

// -------------------------------------------------------
// UpdatePlayerBiome — call every frame with player location
// -------------------------------------------------------
void ABiomeManager::UpdatePlayerBiome(const FVector& PlayerLocation)
{
    const EBiomeType DetectedBiome = GetBiomeAtLocation(PlayerLocation);

    if (DetectedBiome != CurrentBiomeType)
    {
        RequestBiomeTransition(DetectedBiome);
    }
}

// -------------------------------------------------------
// GetCurrentBiomeTemperature
// -------------------------------------------------------
float ABiomeManager::GetCurrentBiomeTemperature() const
{
    for (const FEng_BiomeRuntimeData& Data : ActiveBiomeData)
    {
        if (Data.BiomeType == CurrentBiomeType)
        {
            return Data.CurrentTemperature;
        }
    }
    return 20.0f; // Fallback
}

// -------------------------------------------------------
// GetCurrentBiomeHumidity
// -------------------------------------------------------
float ABiomeManager::GetCurrentBiomeHumidity() const
{
    for (const FEng_BiomeRuntimeData& Data : ActiveBiomeData)
    {
        if (Data.BiomeType == CurrentBiomeType)
        {
            return Data.CurrentHumidity;
        }
    }
    return 0.5f; // Fallback
}

// -------------------------------------------------------
// GetActiveDinosaurCountForBiome
// -------------------------------------------------------
int32 ABiomeManager::GetActiveDinosaurCountForBiome(EBiomeType BiomeType) const
{
    for (const FEng_BiomeRuntimeData& Data : ActiveBiomeData)
    {
        if (Data.BiomeType == BiomeType)
        {
            return Data.ActiveDinosaurCount;
        }
    }
    return 0;
}

// -------------------------------------------------------
// RegisterDinosaurInBiome — called by DinosaurBase on spawn/death
// -------------------------------------------------------
void ABiomeManager::RegisterDinosaurInBiome(EBiomeType BiomeType, bool bAdding)
{
    for (FEng_BiomeRuntimeData& Data : ActiveBiomeData)
    {
        if (Data.BiomeType == BiomeType)
        {
            if (bAdding)
            {
                Data.ActiveDinosaurCount = FMath::Max(0, Data.ActiveDinosaurCount + 1);
            }
            else
            {
                Data.ActiveDinosaurCount = FMath::Max(0, Data.ActiveDinosaurCount - 1);
            }
            return;
        }
    }
}
