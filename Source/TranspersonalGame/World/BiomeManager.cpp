// BiomeManager.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Biome system: classifies world locations into prehistoric biomes,
// drives survival stat modifiers (temperature, hydration drain, visibility)

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

// ─────────────────────────────────────────────────────────────────────────────
// FCore_BiomeData defaults
// ─────────────────────────────────────────────────────────────────────────────

FCore_BiomeData::FCore_BiomeData()
    : BiomeType(ECore_BiomeType::Jungle)
    , DisplayName(TEXT("Jungle"))
    , BaseTemperature(32.0f)
    , HydrationDrainMultiplier(1.5f)
    , StaminaDrainMultiplier(1.2f)
    , FoodAbundance(0.8f)
    , DangerLevel(0.6f)
    , VisibilityRange(1500.0f)
    , BiomeColor(FLinearColor(0.1f, 0.5f, 0.1f, 1.0f))
{
}

// ─────────────────────────────────────────────────────────────────────────────
// ABiomeManager
// ─────────────────────────────────────────────────────────────────────────────

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 2.0f; // Update every 2 seconds — not every frame

    CurrentPlayerBiome = ECore_BiomeType::Jungle;
    bBiomesInitialized = false;
    BiomeTransitionBlend = 0.0f;
    BiomeCheckRadius = 5000.0f;
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeBiomes();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: BeginPlay — %d biomes registered"), BiomeRegistry.Num());
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bBiomesInitialized)
        return;

    // Update player's current biome
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLoc = PlayerPawn->GetActorLocation();
        ECore_BiomeType NewBiome = GetBiomeAtLocation(PlayerLoc);

        if (NewBiome != CurrentPlayerBiome)
        {
            ECore_BiomeType OldBiome = CurrentPlayerBiome;
            CurrentPlayerBiome = NewBiome;
            OnBiomeTransition(OldBiome, NewBiome);
            UE_LOG(LogTemp, Verbose, TEXT("BiomeManager: Player entered biome %d"), (int32)NewBiome);
        }
    }

    UpdateActiveBiomes();
}

// ─────────────────────────────────────────────────────────────────────────────
// InitializeBiomes — register all prehistoric biomes with their stat modifiers
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::InitializeBiomes()
{
    BiomeRegistry.Empty();

    // ── Jungle ──────────────────────────────────────────────────────────────
    {
        FCore_BiomeData Jungle;
        Jungle.BiomeType = ECore_BiomeType::Jungle;
        Jungle.DisplayName = TEXT("Dense Jungle");
        Jungle.BaseTemperature = 34.0f;
        Jungle.HydrationDrainMultiplier = 1.6f;   // Hot + humid = high sweat
        Jungle.StaminaDrainMultiplier = 1.3f;      // Dense vegetation = harder movement
        Jungle.FoodAbundance = 0.9f;               // Fruit, small prey abundant
        Jungle.DangerLevel = 0.7f;                 // Raptors, ambush predators
        Jungle.VisibilityRange = 800.0f;           // Dense canopy limits sight
        Jungle.BiomeColor = FLinearColor(0.05f, 0.45f, 0.05f, 1.0f);
        BiomeRegistry.Add(ECore_BiomeType::Jungle, Jungle);
    }

    // ── Savanna ─────────────────────────────────────────────────────────────
    {
        FCore_BiomeData Savanna;
        Savanna.BiomeType = ECore_BiomeType::Savanna;
        Savanna.DisplayName = TEXT("Open Savanna");
        Savanna.BaseTemperature = 38.0f;
        Savanna.HydrationDrainMultiplier = 2.0f;   // Hot + exposed = rapid dehydration
        Savanna.StaminaDrainMultiplier = 0.9f;     // Flat terrain = easier movement
        Savanna.FoodAbundance = 0.5f;              // Sparse vegetation
        Savanna.DangerLevel = 0.8f;                // T-Rex territory, open = no cover
        Savanna.VisibilityRange = 8000.0f;         // Clear sightlines
        Savanna.BiomeColor = FLinearColor(0.7f, 0.6f, 0.2f, 1.0f);
        BiomeRegistry.Add(ECore_BiomeType::Savanna, Savanna);
    }

    // ── Swamp ───────────────────────────────────────────────────────────────
    {
        FCore_BiomeData Swamp;
        Swamp.BiomeType = ECore_BiomeType::Swamp;
        Swamp.DisplayName = TEXT("Primordial Swamp");
        Swamp.BaseTemperature = 28.0f;
        Swamp.HydrationDrainMultiplier = 0.7f;    // Water everywhere = low thirst
        Swamp.StaminaDrainMultiplier = 1.8f;      // Mud/water = very slow movement
        Swamp.FoodAbundance = 0.7f;               // Fish, amphibians
        Swamp.DangerLevel = 0.9f;                 // Spinosaurus, crocodilians
        Swamp.VisibilityRange = 600.0f;           // Fog + vegetation
        Swamp.BiomeColor = FLinearColor(0.2f, 0.35f, 0.15f, 1.0f);
        BiomeRegistry.Add(ECore_BiomeType::Swamp, Swamp);
    }

    // ── Volcanic ────────────────────────────────────────────────────────────
    {
        FCore_BiomeData Volcanic;
        Volcanic.BiomeType = ECore_BiomeType::Volcanic;
        Volcanic.DisplayName = TEXT("Volcanic Badlands");
        Volcanic.BaseTemperature = 55.0f;
        Volcanic.HydrationDrainMultiplier = 3.0f;  // Extreme heat = rapid dehydration
        Volcanic.StaminaDrainMultiplier = 2.0f;    // Ash, unstable ground
        Volcanic.FoodAbundance = 0.1f;             // Almost nothing survives here
        Volcanic.DangerLevel = 1.0f;               // Lava flows, toxic gas, apex predators
        Volcanic.VisibilityRange = 1200.0f;        // Ash clouds reduce visibility
        Volcanic.BiomeColor = FLinearColor(0.6f, 0.15f, 0.05f, 1.0f);
        BiomeRegistry.Add(ECore_BiomeType::Volcanic, Volcanic);
    }

    // ── River ───────────────────────────────────────────────────────────────
    {
        FCore_BiomeData River;
        River.BiomeType = ECore_BiomeType::River;
        River.DisplayName = TEXT("River Crossing");
        River.BaseTemperature = 25.0f;
        River.HydrationDrainMultiplier = 0.3f;    // Drinking water available
        River.StaminaDrainMultiplier = 1.4f;      // Swimming/wading costs stamina
        River.FoodAbundance = 0.85f;              // Fish, riverbank prey
        River.DangerLevel = 0.6f;                 // Spinosaurus near water
        River.VisibilityRange = 3000.0f;
        River.BiomeColor = FLinearColor(0.1f, 0.3f, 0.7f, 1.0f);
        BiomeRegistry.Add(ECore_BiomeType::River, River);
    }

    // ── Forest ──────────────────────────────────────────────────────────────
    {
        FCore_BiomeData Forest;
        Forest.BiomeType = ECore_BiomeType::Forest;
        Forest.DisplayName = TEXT("Ancient Forest");
        Forest.BaseTemperature = 22.0f;
        Forest.HydrationDrainMultiplier = 1.0f;   // Baseline
        Forest.StaminaDrainMultiplier = 1.1f;
        Forest.FoodAbundance = 0.75f;
        Forest.DangerLevel = 0.5f;                // Moderate — raptors patrol
        Forest.VisibilityRange = 1500.0f;
        Forest.BiomeColor = FLinearColor(0.1f, 0.4f, 0.1f, 1.0f);
        BiomeRegistry.Add(ECore_BiomeType::Forest, Forest);
    }

    bBiomesInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: InitializeBiomes complete — %d biomes"), BiomeRegistry.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// GetBiomeAtLocation — classify a world position into a biome
// Uses elevation + distance heuristics (PCG biome data will replace this later)
// ─────────────────────────────────────────────────────────────────────────────

ECore_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    if (!bBiomesInitialized)
        return ECore_BiomeType::Jungle;

    float Z = WorldLocation.Z;
    float X = WorldLocation.X;
    float Y = WorldLocation.Y;

    // Elevation-based classification (terrain height drives biome)
    if (Z > 8000.0f)
        return ECore_BiomeType::Volcanic;   // High altitude = volcanic peaks

    if (Z < -200.0f)
        return ECore_BiomeType::Swamp;      // Below sea level = swamp/wetlands

    // Distance from world center drives biome ring
    float DistFromCenter = FVector2D(X, Y).Size();

    if (DistFromCenter < 3000.0f)
        return ECore_BiomeType::Jungle;     // Central jungle

    if (DistFromCenter < 6000.0f)
        return ECore_BiomeType::Forest;     // Mid-ring forest

    if (DistFromCenter < 10000.0f)
        return ECore_BiomeType::Savanna;    // Outer savanna

    // River corridors (Y axis approximation — PCG will refine)
    if (FMath::Abs(Y) < 500.0f && DistFromCenter > 2000.0f)
        return ECore_BiomeType::River;

    return ECore_BiomeType::Savanna;
}

FCore_BiomeData ABiomeManager::GetBiomeData(ECore_BiomeType BiomeType) const
{
    const FCore_BiomeData* Data = BiomeRegistry.Find(BiomeType);
    if (Data)
        return *Data;

    // Return default jungle data if not found
    return FCore_BiomeData();
}

FCore_BiomeData ABiomeManager::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    ECore_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    return GetBiomeData(Biome);
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateActiveBiomes — called every tick interval to refresh nearby biome state
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::UpdateActiveBiomes()
{
    // Lightweight update — just ensure player biome is current
    // Full biome streaming (PCG integration) handled by Agent #05
    BiomeTransitionBlend = FMath::Clamp(BiomeTransitionBlend + 0.05f, 0.0f, 1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// OnBiomeTransition — Blueprint event + C++ logic on biome change
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::OnBiomeTransition(ECore_BiomeType FromBiome, ECore_BiomeType ToBiome)
{
    BiomeTransitionBlend = 0.0f; // Reset blend on new transition

    // Log the transition
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transition %d → %d"), (int32)FromBiome, (int32)ToBiome);

    // Blueprint can override this to play ambient sound transitions, fog changes, etc.
    OnBiomeTransitionEvent(FromBiome, ToBiome);
}

void ABiomeManager::OnBiomeTransitionEvent_Implementation(ECore_BiomeType FromBiome, ECore_BiomeType ToBiome)
{
    // Default C++ implementation — Blueprint overrides for visual/audio effects
    UE_LOG(LogTemp, Verbose, TEXT("BiomeManager: OnBiomeTransitionEvent %d → %d"), (int32)FromBiome, (int32)ToBiome);
}

// ─────────────────────────────────────────────────────────────────────────────
// GetSurvivalModifiersForBiome — returns stat multipliers for survival system
// Called by SurvivalComponent every tick to apply biome penalties/bonuses
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::GetSurvivalModifiersForBiome(
    ECore_BiomeType BiomeType,
    float& OutHydrationDrain,
    float& OutStaminaDrain,
    float& OutTemperature) const
{
    FCore_BiomeData Data = GetBiomeData(BiomeType);
    OutHydrationDrain = Data.HydrationDrainMultiplier;
    OutStaminaDrain = Data.StaminaDrainMultiplier;
    OutTemperature = Data.BaseTemperature;
}

ECore_BiomeType ABiomeManager::GetCurrentPlayerBiome() const
{
    return CurrentPlayerBiome;
}
