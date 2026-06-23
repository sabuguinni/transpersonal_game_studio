// BiomeManager.cpp
// Engine Architect #02 — P1 World Generation
// Implements biome detection, transitions, and runtime data for the prehistoric survival world.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// -------------------------------------------------------
// Constructor
// -------------------------------------------------------
ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Check biome every 1 second (not every frame)

    // Default biome definitions
    BiomeDefinitions.Empty();

    // Jungle biome
    FEng_BiomeDefinition Jungle;
    Jungle.BiomeType = EBiomeType::Jungle;
    Jungle.DisplayName = FText::FromString(TEXT("Cretaceous Jungle"));
    Jungle.BaseTemperature = 28.0f;
    Jungle.BaseHumidity = 0.85f;
    Jungle.FogDensity = 0.04f;
    Jungle.FogColor = FLinearColor(0.1f, 0.15f, 0.08f, 1.0f);
    Jungle.AmbientColor = FLinearColor(0.05f, 0.12f, 0.04f, 1.0f);
    Jungle.bAllowsDinosaurSpawn = true;
    Jungle.DinosaurDensityMultiplier = 1.5f;
    Jungle.VegetationDensityMultiplier = 2.0f;
    BiomeDefinitions.Add(Jungle);

    // Savanna biome
    FEng_BiomeDefinition Savanna;
    Savanna.BiomeType = EBiomeType::Savanna;
    Savanna.DisplayName = FText::FromString(TEXT("Open Savanna"));
    Savanna.BaseTemperature = 35.0f;
    Savanna.BaseHumidity = 0.25f;
    Savanna.FogDensity = 0.01f;
    Savanna.FogColor = FLinearColor(0.3f, 0.25f, 0.1f, 1.0f);
    Savanna.AmbientColor = FLinearColor(0.25f, 0.2f, 0.05f, 1.0f);
    Savanna.bAllowsDinosaurSpawn = true;
    Savanna.DinosaurDensityMultiplier = 1.2f;
    Savanna.VegetationDensityMultiplier = 0.4f;
    BiomeDefinitions.Add(Savanna);

    // Swamp biome
    FEng_BiomeDefinition Swamp;
    Swamp.BiomeType = EBiomeType::Swamp;
    Swamp.DisplayName = FText::FromString(TEXT("Primordial Swamp"));
    Swamp.BaseTemperature = 24.0f;
    Swamp.BaseHumidity = 0.95f;
    Swamp.FogDensity = 0.08f;
    Swamp.FogColor = FLinearColor(0.05f, 0.1f, 0.05f, 1.0f);
    Swamp.AmbientColor = FLinearColor(0.02f, 0.08f, 0.03f, 1.0f);
    Swamp.bAllowsDinosaurSpawn = true;
    Swamp.DinosaurDensityMultiplier = 0.8f;
    Swamp.VegetationDensityMultiplier = 1.8f;
    BiomeDefinitions.Add(Swamp);

    // Volcanic biome
    FEng_BiomeDefinition Volcanic;
    Volcanic.BiomeType = EBiomeType::Volcanic;
    Volcanic.DisplayName = FText::FromString(TEXT("Volcanic Badlands"));
    Volcanic.BaseTemperature = 55.0f;
    Volcanic.BaseHumidity = 0.05f;
    Volcanic.FogDensity = 0.06f;
    Volcanic.FogColor = FLinearColor(0.2f, 0.08f, 0.02f, 1.0f);
    Volcanic.AmbientColor = FLinearColor(0.15f, 0.05f, 0.01f, 1.0f);
    Volcanic.bAllowsDinosaurSpawn = false;
    Volcanic.DinosaurDensityMultiplier = 0.1f;
    Volcanic.VegetationDensityMultiplier = 0.05f;
    BiomeDefinitions.Add(Volcanic);

    // Coastal biome
    FEng_BiomeDefinition Coastal;
    Coastal.BiomeType = EBiomeType::Coastal;
    Coastal.DisplayName = FText::FromString(TEXT("Coastal Shore"));
    Coastal.BaseTemperature = 22.0f;
    Coastal.BaseHumidity = 0.7f;
    Coastal.FogDensity = 0.03f;
    Coastal.FogColor = FLinearColor(0.15f, 0.2f, 0.25f, 1.0f);
    Coastal.AmbientColor = FLinearColor(0.1f, 0.15f, 0.2f, 1.0f);
    Coastal.bAllowsDinosaurSpawn = true;
    Coastal.DinosaurDensityMultiplier = 0.6f;
    Coastal.VegetationDensityMultiplier = 0.7f;
    BiomeDefinitions.Add(Coastal);

    // Mountain biome
    FEng_BiomeDefinition Mountain;
    Mountain.BiomeType = EBiomeType::Mountain;
    Mountain.DisplayName = FText::FromString(TEXT("Highland Ridge"));
    Mountain.BaseTemperature = 8.0f;
    Mountain.BaseHumidity = 0.4f;
    Mountain.FogDensity = 0.05f;
    Mountain.FogColor = FLinearColor(0.2f, 0.22f, 0.25f, 1.0f);
    Mountain.AmbientColor = FLinearColor(0.15f, 0.17f, 0.2f, 1.0f);
    Mountain.bAllowsDinosaurSpawn = true;
    Mountain.DinosaurDensityMultiplier = 0.5f;
    Mountain.VegetationDensityMultiplier = 0.3f;
    BiomeDefinitions.Add(Mountain);

    // River biome
    FEng_BiomeDefinition River;
    River.BiomeType = EBiomeType::River;
    River.DisplayName = FText::FromString(TEXT("River Delta"));
    River.BaseTemperature = 20.0f;
    River.BaseHumidity = 0.9f;
    River.FogDensity = 0.035f;
    River.FogColor = FLinearColor(0.08f, 0.12f, 0.15f, 1.0f);
    River.AmbientColor = FLinearColor(0.05f, 0.1f, 0.12f, 1.0f);
    River.bAllowsDinosaurSpawn = true;
    River.DinosaurDensityMultiplier = 1.0f;
    River.VegetationDensityMultiplier = 1.4f;
    BiomeDefinitions.Add(River);

    // Plains biome
    FEng_BiomeDefinition Plains;
    Plains.BiomeType = EBiomeType::Plains;
    Plains.DisplayName = FText::FromString(TEXT("Open Plains"));
    Plains.BaseTemperature = 25.0f;
    Plains.BaseHumidity = 0.35f;
    Plains.FogDensity = 0.008f;
    Plains.FogColor = FLinearColor(0.25f, 0.22f, 0.15f, 1.0f);
    Plains.AmbientColor = FLinearColor(0.2f, 0.18f, 0.1f, 1.0f);
    Plains.bAllowsDinosaurSpawn = true;
    Plains.DinosaurDensityMultiplier = 1.3f;
    Plains.VegetationDensityMultiplier = 0.6f;
    BiomeDefinitions.Add(Plains);

    // Initial state
    CurrentBiomeType = EBiomeType::Plains;
    PreviousBiomeType = EBiomeType::Plains;
    TransitionProgress = 1.0f;
    TransitionSpeed = 2.0f;
    bIsTransitioning = false;
    BiomeTransitionBlendRadius = 500.0f;
}

// -------------------------------------------------------
// BeginPlay
// -------------------------------------------------------
void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize runtime data for all biomes
    for (const FEng_BiomeDefinition& Def : BiomeDefinitions)
    {
        FEng_BiomeRuntimeData RuntimeData;
        RuntimeData.BiomeType = Def.BiomeType;
        RuntimeData.CurrentTemperature = Def.BaseTemperature;
        RuntimeData.CurrentHumidity = Def.BaseHumidity;
        RuntimeData.ActiveDinosaurCount = 0;
        RuntimeData.bIsActive = (Def.BiomeType == CurrentBiomeType);
        ActiveBiomeData.Add(RuntimeData);
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome definitions"), BiomeDefinitions.Num());
}

// -------------------------------------------------------
// Tick
// -------------------------------------------------------
void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Advance transition if active
    if (bIsTransitioning)
    {
        TransitionProgress = FMath::Clamp(TransitionProgress + DeltaTime * TransitionSpeed, 0.0f, 1.0f);

        if (TransitionProgress >= 1.0f)
        {
            bIsTransitioning = false;
            PreviousBiomeType = CurrentBiomeType;
            UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transition complete -> %d"), (int32)CurrentBiomeType);
        }
    }
}

// -------------------------------------------------------
// GetBiomeAtLocation
// -------------------------------------------------------
EBiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Simple height + distance-based biome assignment for MinPlayableMap
    // In production this would sample a biome map texture or use PCG data

    float Height = WorldLocation.Z;
    float DistFromCenter = FVector2D(WorldLocation.X, WorldLocation.Y).Size();

    // Volcanic zone: high altitude or very far from center
    if (Height > 800.0f)
    {
        return EBiomeType::Volcanic;
    }

    // Mountain: elevated terrain
    if (Height > 400.0f)
    {
        return EBiomeType::Mountain;
    }

    // Coastal: near the edge of the map
    if (DistFromCenter > 3000.0f)
    {
        return EBiomeType::Coastal;
    }

    // Swamp: low-lying areas with negative Z
    if (Height < -50.0f)
    {
        return EBiomeType::Swamp;
    }

    // River: near water features (placeholder: negative X quadrant)
    if (WorldLocation.X < -500.0f && Height < 100.0f)
    {
        return EBiomeType::River;
    }

    // Jungle: mid-range distance, moderate height
    if (DistFromCenter < 800.0f && Height > 50.0f)
    {
        return EBiomeType::Jungle;
    }

    // Savanna: far from center but not coastal
    if (DistFromCenter > 1500.0f)
    {
        return EBiomeType::Savanna;
    }

    // Default: Plains
    return EBiomeType::Plains;
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
    FEng_BiomeDefinition Default;
    Default.BiomeType = EBiomeType::Plains;
    Default.DisplayName = FText::FromString(TEXT("Unknown"));
    Default.BaseTemperature = 20.0f;
    Default.BaseHumidity = 0.5f;
    Default.FogDensity = 0.01f;
    return Default;
}

// -------------------------------------------------------
// RequestBiomeTransition
// -------------------------------------------------------
void ABiomeManager::RequestBiomeTransition(EBiomeType NewBiome)
{
    if (NewBiome == CurrentBiomeType)
    {
        return;
    }

    PreviousBiomeType = CurrentBiomeType;
    CurrentBiomeType = NewBiome;
    TransitionProgress = 0.0f;
    bIsTransitioning = true;

    // Update active flags in runtime data
    for (FEng_BiomeRuntimeData& Data : ActiveBiomeData)
    {
        Data.bIsActive = (Data.BiomeType == CurrentBiomeType);
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transition requested %d -> %d"), (int32)PreviousBiomeType, (int32)CurrentBiomeType);

    OnBiomeChanged.Broadcast(PreviousBiomeType, CurrentBiomeType);
}

// -------------------------------------------------------
// UpdatePlayerBiome
// -------------------------------------------------------
void ABiomeManager::UpdatePlayerBiome(const FVector& PlayerLocation)
{
    EBiomeType DetectedBiome = GetBiomeAtLocation(PlayerLocation);

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
    FEng_BiomeDefinition CurrentDef = GetBiomeDefinition(CurrentBiomeType);
    FEng_BiomeDefinition PreviousDef = GetBiomeDefinition(PreviousBiomeType);

    // Lerp temperature during transition
    return FMath::Lerp(PreviousDef.BaseTemperature, CurrentDef.BaseTemperature, TransitionProgress);
}

// -------------------------------------------------------
// GetCurrentBiomeHumidity
// -------------------------------------------------------
float ABiomeManager::GetCurrentBiomeHumidity() const
{
    FEng_BiomeDefinition CurrentDef = GetBiomeDefinition(CurrentBiomeType);
    FEng_BiomeDefinition PreviousDef = GetBiomeDefinition(PreviousBiomeType);

    return FMath::Lerp(PreviousDef.BaseHumidity, CurrentDef.BaseHumidity, TransitionProgress);
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
// RegisterDinosaurInBiome
// -------------------------------------------------------
void ABiomeManager::RegisterDinosaurInBiome(EBiomeType BiomeType, bool bAdding)
{
    for (FEng_BiomeRuntimeData& Data : ActiveBiomeData)
    {
        if (Data.BiomeType == BiomeType)
        {
            if (bAdding)
            {
                Data.ActiveDinosaurCount++;
            }
            else
            {
                Data.ActiveDinosaurCount = FMath::Max(0, Data.ActiveDinosaurCount - 1);
            }
            return;
        }
    }
}
