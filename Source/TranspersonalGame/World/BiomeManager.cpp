// BiomeManager.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Biome classification, transition, and environmental parameter system
// Priority: P1 — World Generation

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 2.0f; // Update every 2 seconds — not every frame

    // Default biome parameters
    CurrentBiomeType = EBiomeType::Grassland;
    TransitionBlendAlpha = 0.0f;
    bBiomeTransitionActive = false;
    WorldSizeKm = 4.0f;
    BiomeSeed = 42;

    // Default environmental parameters
    CurrentTemperatureCelsius = 28.0f;
    CurrentHumidityPercent = 65.0f;
    CurrentWindSpeedKmh = 15.0f;
    CurrentVisibilityMeters = 1000.0f;
    bIsRaining = false;
    RainIntensity = 0.0f;

    // Survival impact defaults
    TemperatureHeatStressThreshold = 38.0f;
    TemperatureColdStressThreshold = 10.0f;
    HumidityDehydrationModifier = 1.0f;
}

// ============================================================
// Lifecycle
// ============================================================

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeBiomeMap();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with seed %d, world size %.1f km²"), BiomeSeed, WorldSizeKm * WorldSizeKm);
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update biome transitions
    if (bBiomeTransitionActive)
    {
        TransitionBlendAlpha = FMath::Clamp(TransitionBlendAlpha + DeltaTime * 0.1f, 0.0f, 1.0f);
        if (TransitionBlendAlpha >= 1.0f)
        {
            bBiomeTransitionActive = false;
            CurrentBiomeType = TargetBiomeType;
            OnBiomeTransitionComplete(CurrentBiomeType);
        }
    }

    // Update environmental simulation
    UpdateEnvironmentalParameters(DeltaTime);
}

// ============================================================
// Biome Initialization
// ============================================================

void ABiomeManager::InitializeBiomeMap()
{
    // Define the 6 core biomes for the Cretaceous world
    // Each biome has distinct survival parameters

    FBiomeData GrasslandBiome;
    GrasslandBiome.BiomeType = EBiomeType::Grassland;
    GrasslandBiome.DisplayName = FText::FromString(TEXT("Cretaceous Grassland"));
    GrasslandBiome.BaseTemperature = 28.0f;
    GrasslandBiome.BaseHumidity = 60.0f;
    GrasslandBiome.BaseWindSpeed = 20.0f;
    GrasslandBiome.FoliageDensity = 0.4f;
    GrasslandBiome.DinosaurDensityMultiplier = 1.0f;
    GrasslandBiome.PlayerMovementSpeedModifier = 1.0f;
    GrasslandBiome.bAllowsFireCrafting = true;
    GrasslandBiome.ResourceTypes = { TEXT("Flint"), TEXT("Grass"), TEXT("Berries") };
    BiomeDataMap.Add(EBiomeType::Grassland, GrasslandBiome);

    FBiomeData ForestBiome;
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.DisplayName = FText::FromString(TEXT("Cretaceous Forest"));
    ForestBiome.BaseTemperature = 24.0f;
    ForestBiome.BaseHumidity = 80.0f;
    ForestBiome.BaseWindSpeed = 5.0f;
    ForestBiome.FoliageDensity = 0.9f;
    ForestBiome.DinosaurDensityMultiplier = 1.5f;
    ForestBiome.PlayerMovementSpeedModifier = 0.75f; // Dense foliage slows movement
    ForestBiome.bAllowsFireCrafting = true;
    ForestBiome.ResourceTypes = { TEXT("Wood"), TEXT("Mushrooms"), TEXT("Vines"), TEXT("Insects") };
    BiomeDataMap.Add(EBiomeType::Forest, ForestBiome);

    FBiomeData SwampBiome;
    SwampBiome.BiomeType = EBiomeType::Swamp;
    SwampBiome.DisplayName = FText::FromString(TEXT("Cretaceous Swamp"));
    SwampBiome.BaseTemperature = 32.0f;
    SwampBiome.BaseHumidity = 95.0f;
    SwampBiome.BaseWindSpeed = 3.0f;
    SwampBiome.FoliageDensity = 0.7f;
    SwampBiome.DinosaurDensityMultiplier = 2.0f; // Dangerous zone
    SwampBiome.PlayerMovementSpeedModifier = 0.5f; // Mud severely slows movement
    SwampBiome.bAllowsFireCrafting = false; // Too wet
    SwampBiome.ResourceTypes = { TEXT("Clay"), TEXT("Reeds"), TEXT("Fish"), TEXT("Mud") };
    BiomeDataMap.Add(EBiomeType::Swamp, SwampBiome);

    FBiomeData DesertBiome;
    DesertBiome.BiomeType = EBiomeType::Desert;
    DesertBiome.DisplayName = FText::FromString(TEXT("Cretaceous Desert"));
    DesertBiome.BaseTemperature = 42.0f;
    DesertBiome.BaseHumidity = 15.0f;
    DesertBiome.BaseWindSpeed = 35.0f;
    DesertBiome.FoliageDensity = 0.05f;
    DesertBiome.DinosaurDensityMultiplier = 0.3f; // Sparse life
    DesertBiome.PlayerMovementSpeedModifier = 0.9f;
    DesertBiome.bAllowsFireCrafting = true;
    DesertBiome.ResourceTypes = { TEXT("Sandstone"), TEXT("Bones"), TEXT("Cactus") };
    BiomeDataMap.Add(EBiomeType::Desert, DesertBiome);

    FBiomeData VolcanicBiome;
    VolcanicBiome.BiomeType = EBiomeType::Volcanic;
    VolcanicBiome.DisplayName = FText::FromString(TEXT("Volcanic Badlands"));
    VolcanicBiome.BaseTemperature = 55.0f;
    VolcanicBiome.BaseHumidity = 20.0f;
    VolcanicBiome.BaseWindSpeed = 40.0f;
    VolcanicBiome.FoliageDensity = 0.02f;
    VolcanicBiome.DinosaurDensityMultiplier = 0.1f; // Almost no life
    VolcanicBiome.PlayerMovementSpeedModifier = 0.8f;
    VolcanicBiome.bAllowsFireCrafting = true;
    VolcanicBiome.ResourceTypes = { TEXT("Obsidian"), TEXT("Sulfur"), TEXT("Basalt") };
    BiomeDataMap.Add(EBiomeType::Volcanic, VolcanicBiome);

    FBiomeData RiverbankBiome;
    RiverbankBiome.BiomeType = EBiomeType::Riverbank;
    RiverbankBiome.DisplayName = FText::FromString(TEXT("Cretaceous Riverbank"));
    RiverbankBiome.BaseTemperature = 26.0f;
    RiverbankBiome.BaseHumidity = 75.0f;
    RiverbankBiome.BaseWindSpeed = 10.0f;
    RiverbankBiome.FoliageDensity = 0.6f;
    RiverbankBiome.DinosaurDensityMultiplier = 1.8f; // Water attracts dinosaurs
    RiverbankBiome.PlayerMovementSpeedModifier = 0.85f;
    RiverbankBiome.bAllowsFireCrafting = true;
    RiverbankBiome.ResourceTypes = { TEXT("Fish"), TEXT("Water"), TEXT("Clay"), TEXT("Reeds"), TEXT("Flint") };
    BiomeDataMap.Add(EBiomeType::Riverbank, RiverbankBiome);

    // Set initial biome
    if (BiomeDataMap.Contains(CurrentBiomeType))
    {
        ApplyBiomeParameters(BiomeDataMap[CurrentBiomeType]);
    }
}

// ============================================================
// Biome Query
// ============================================================

EBiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Simplified biome determination based on world position
    // In full implementation this would sample a biome noise map
    float X = WorldLocation.X / 100.0f; // Convert cm to meters
    float Y = WorldLocation.Y / 100.0f;

    // Use distance from origin and angle to determine biome zones
    float Distance = FMath::Sqrt(X * X + Y * Y);
    float Angle = FMath::Atan2(Y, X) * (180.0f / PI);

    // Zone assignment based on distance rings and angular sectors
    if (Distance < 500.0f) return EBiomeType::Grassland;       // Center: grassland
    if (Distance < 1000.0f && Angle > 0.0f) return EBiomeType::Forest;     // NE: forest
    if (Distance < 1000.0f && Angle <= 0.0f) return EBiomeType::Riverbank; // SE: riverbank
    if (Distance < 1500.0f && Angle > 90.0f) return EBiomeType::Swamp;     // NW: swamp
    if (Distance < 1500.0f && Angle < -90.0f) return EBiomeType::Desert;   // SW: desert
    return EBiomeType::Volcanic; // Outer ring: volcanic badlands

}

FBiomeData ABiomeManager::GetBiomeData(EBiomeType BiomeType) const
{
    if (BiomeDataMap.Contains(BiomeType))
    {
        return BiomeDataMap[BiomeType];
    }
    return FBiomeData(); // Return default empty struct
}

float ABiomeManager::GetPlayerSurvivalModifier(const FVector& PlayerLocation) const
{
    EBiomeType BiomeAtLocation = GetBiomeAtLocation(PlayerLocation);
    if (!BiomeDataMap.Contains(BiomeAtLocation)) return 1.0f;

    const FBiomeData& Data = BiomeDataMap[BiomeAtLocation];

    // Calculate composite survival modifier
    float ThermalStress = 1.0f;
    if (Data.BaseTemperature > TemperatureHeatStressThreshold)
    {
        ThermalStress = 1.0f + (Data.BaseTemperature - TemperatureHeatStressThreshold) * 0.05f;
    }
    else if (Data.BaseTemperature < TemperatureColdStressThreshold)
    {
        ThermalStress = 1.0f + (TemperatureColdStressThreshold - Data.BaseTemperature) * 0.03f;
    }

    float HumidityStress = 1.0f + FMath::Abs(Data.BaseHumidity - 50.0f) * 0.005f;

    return ThermalStress * HumidityStress;
}

// ============================================================
// Biome Transition
// ============================================================

void ABiomeManager::TriggerBiomeTransition(EBiomeType NewBiome)
{
    if (NewBiome == CurrentBiomeType) return;

    TargetBiomeType = NewBiome;
    bBiomeTransitionActive = true;
    TransitionBlendAlpha = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transitioning from %d to %d"),
        (int32)CurrentBiomeType, (int32)NewBiome);
}

void ABiomeManager::OnBiomeTransitionComplete(EBiomeType NewBiome)
{
    if (BiomeDataMap.Contains(NewBiome))
    {
        ApplyBiomeParameters(BiomeDataMap[NewBiome]);
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transition complete — now in biome %d"), (int32)NewBiome);
    OnBiomeChanged.Broadcast(NewBiome);
}

void ABiomeManager::ApplyBiomeParameters(const FBiomeData& BiomeData)
{
    CurrentTemperatureCelsius = BiomeData.BaseTemperature;
    CurrentHumidityPercent = BiomeData.BaseHumidity;
    CurrentWindSpeedKmh = BiomeData.BaseWindSpeed;
}

// ============================================================
// Environmental Simulation
// ============================================================

void ABiomeManager::UpdateEnvironmentalParameters(float DeltaTime)
{
    // Gentle drift simulation — parameters fluctuate slightly over time
    float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    // Temperature oscillation (simulates day/night cycle influence)
    float TempVariation = FMath::Sin(TimeSeconds * 0.01f) * 3.0f;
    CurrentTemperatureCelsius = FMath::Clamp(
        CurrentTemperatureCelsius + TempVariation * DeltaTime * 0.1f,
        -10.0f, 60.0f
    );

    // Wind variation
    float WindVariation = FMath::Sin(TimeSeconds * 0.05f + 1.5f) * 5.0f;
    CurrentWindSpeedKmh = FMath::Clamp(
        CurrentWindSpeedKmh + WindVariation * DeltaTime * 0.1f,
        0.0f, 80.0f
    );
}

// ============================================================
// Resource Query
// ============================================================

TArray<FString> ABiomeManager::GetAvailableResourcesAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeAtLocation = GetBiomeAtLocation(WorldLocation);
    if (BiomeDataMap.Contains(BiomeAtLocation))
    {
        return BiomeDataMap[BiomeAtLocation].ResourceTypes;
    }
    return TArray<FString>();
}

bool ABiomeManager::CanCraftFireAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeAtLocation = GetBiomeAtLocation(WorldLocation);
    if (BiomeDataMap.Contains(BiomeAtLocation))
    {
        return BiomeDataMap[BiomeAtLocation].bAllowsFireCrafting;
    }
    return false;
}
