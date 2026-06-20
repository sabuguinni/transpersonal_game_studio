// BiomeManager.cpp
// Engine Architect #02 — PROD_CYCLE_AUTO_20260620_001
// P1 Priority: World Generation — Biome system concrete implementation

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

UBiomeManager::UBiomeManager()
{
    // Default biome configuration — safe for CDO construction
    CurrentBiome = EEng_BiomeType::Grassland;
    TransitionBlendAlpha = 0.0f;
    bBiomeTransitionActive = false;
    WorldTemperature = 25.0f;
    WorldHumidity = 0.5f;
    WorldAltitude = 0.0f;

    // Populate default biome data table
    InitializeDefaultBiomeData();
}

// ============================================================
// UGameInstanceSubsystem interface
// ============================================================

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeDefaultBiomeData();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome definitions"), BiomeDataTable.Num());
}

void UBiomeManager::Deinitialize()
{
    BiomeDataTable.Empty();
    Super::Deinitialize();
}

// ============================================================
// Core biome query
// ============================================================

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Deterministic biome assignment based on world coordinates
    // Uses noise-like hash to create varied biome regions without PCG dependency
    const float NormalizedX = WorldLocation.X / 10000.0f;
    const float NormalizedY = WorldLocation.Y / 10000.0f;
    const float Altitude = WorldLocation.Z;

    // Altitude overrides
    if (Altitude > 3000.0f)
    {
        return EEng_BiomeType::Alpine;
    }
    if (Altitude < -50.0f)
    {
        return EEng_BiomeType::Swamp;
    }

    // Simple hash-based biome grid (8 biome types across world)
    const int32 GridX = FMath::FloorToInt(NormalizedX);
    const int32 GridY = FMath::FloorToInt(NormalizedY);
    const int32 Hash = FMath::Abs((GridX * 73856093) ^ (GridY * 19349663)) % 6;

    switch (Hash)
    {
        case 0: return EEng_BiomeType::Grassland;
        case 1: return EEng_BiomeType::Forest;
        case 2: return EEng_BiomeType::Desert;
        case 3: return EEng_BiomeType::Jungle;
        case 4: return EEng_BiomeType::Volcanic;
        case 5: return EEng_BiomeType::Coastal;
        default: return EEng_BiomeType::Grassland;
    }
}

FEng_BiomeData UBiomeManager::GetBiomeData(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeData* Found = BiomeDataTable.Find(BiomeType);
    if (Found)
    {
        return *Found;
    }

    // Return safe default if biome not registered
    FEng_BiomeData Default;
    Default.BiomeType = BiomeType;
    Default.DisplayName = FText::FromString(TEXT("Unknown Biome"));
    Default.BaseTemperature = 20.0f;
    Default.BaseHumidity = 0.5f;
    Default.DinosaurDensity = 1.0f;
    Default.VegetationDensity = 1.0f;
    Default.DangerLevel = 1;
    return Default;
}

// ============================================================
// Biome transition
// ============================================================

void UBiomeManager::SetCurrentBiome(EEng_BiomeType NewBiome, float TransitionDuration)
{
    if (NewBiome == CurrentBiome)
    {
        return;
    }

    PreviousBiome = CurrentBiome;
    CurrentBiome = NewBiome;
    TransitionBlendAlpha = 0.0f;
    bBiomeTransitionActive = (TransitionDuration > 0.0f);

    const FEng_BiomeData& NewData = GetBiomeData(NewBiome);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transitioning to biome '%s' (duration=%.1fs)"),
        *NewData.DisplayName.ToString(), TransitionDuration);

    OnBiomeChanged.Broadcast(PreviousBiome, CurrentBiome);
}

void UBiomeManager::UpdateBiomeTransition(float DeltaTime, float TransitionDuration)
{
    if (!bBiomeTransitionActive || TransitionDuration <= 0.0f)
    {
        return;
    }

    TransitionBlendAlpha = FMath::Clamp(TransitionBlendAlpha + (DeltaTime / TransitionDuration), 0.0f, 1.0f);

    if (TransitionBlendAlpha >= 1.0f)
    {
        bBiomeTransitionActive = false;
        TransitionBlendAlpha = 1.0f;
    }
}

// ============================================================
// Environmental queries
// ============================================================

float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    const EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeData Data = GetBiomeData(Biome);

    // Altitude modifier: -6.5°C per 1000m (lapse rate)
    const float AltitudeModifier = (WorldLocation.Z / 100.0f) * -0.065f;
    return Data.BaseTemperature + AltitudeModifier + WorldTemperature - 25.0f;
}

float UBiomeManager::GetHazardLevelAtLocation(const FVector& WorldLocation) const
{
    const EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeData Data = GetBiomeData(Biome);
    return FMath::Clamp(static_cast<float>(Data.DangerLevel) / 5.0f, 0.0f, 1.0f);
}

TArray<FName> UBiomeManager::GetDinosaurSpeciesForBiome(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeData* Data = BiomeDataTable.Find(BiomeType);
    if (Data)
    {
        return Data->NativeDinosaurSpecies;
    }
    return TArray<FName>();
}

// ============================================================
// Debug
// ============================================================

void UBiomeManager::DrawBiomeDebug(UObject* WorldContext, const FVector& Center, float Radius)
{
    if (!WorldContext)
    {
        return;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return;
    }

    // Sample biome grid and draw colored spheres
    const int32 Steps = 10;
    const float Step = (Radius * 2.0f) / Steps;

    for (int32 X = 0; X <= Steps; ++X)
    {
        for (int32 Y = 0; Y <= Steps; ++Y)
        {
            const FVector SamplePos = Center + FVector(
                -Radius + X * Step,
                -Radius + Y * Step,
                0.0f
            );

            const EEng_BiomeType Biome = GetBiomeAtLocation(SamplePos);
            FColor DebugColor = GetBiomeDebugColor(Biome);

            DrawDebugSphere(World, SamplePos, 50.0f, 4, DebugColor, false, 10.0f);
        }
    }
}

// ============================================================
// Private helpers
// ============================================================

void UBiomeManager::InitializeDefaultBiomeData()
{
    BiomeDataTable.Empty();

    // Grassland
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Grassland;
        Data.DisplayName = FText::FromString(TEXT("Grassland Plains"));
        Data.BaseTemperature = 22.0f;
        Data.BaseHumidity = 0.45f;
        Data.DinosaurDensity = 1.2f;
        Data.VegetationDensity = 0.8f;
        Data.DangerLevel = 2;
        Data.NativeDinosaurSpecies = { TEXT("Triceratops"), TEXT("Edmontosaurus"), TEXT("Pachycephalosaurus") };
        BiomeDataTable.Add(EEng_BiomeType::Grassland, Data);
    }

    // Forest
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Forest;
        Data.DisplayName = FText::FromString(TEXT("Dense Forest"));
        Data.BaseTemperature = 18.0f;
        Data.BaseHumidity = 0.70f;
        Data.DinosaurDensity = 0.9f;
        Data.VegetationDensity = 2.0f;
        Data.DangerLevel = 3;
        Data.NativeDinosaurSpecies = { TEXT("Velociraptor"), TEXT("Dryosaurus"), TEXT("Compsognathus") };
        BiomeDataTable.Add(EEng_BiomeType::Forest, Data);
    }

    // Desert
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Desert;
        Data.DisplayName = FText::FromString(TEXT("Arid Desert"));
        Data.BaseTemperature = 38.0f;
        Data.BaseHumidity = 0.10f;
        Data.DinosaurDensity = 0.5f;
        Data.VegetationDensity = 0.2f;
        Data.DangerLevel = 2;
        Data.NativeDinosaurSpecies = { TEXT("Oviraptor"), TEXT("Protoceratops") };
        BiomeDataTable.Add(EEng_BiomeType::Desert, Data);
    }

    // Jungle
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Jungle;
        Data.DisplayName = FText::FromString(TEXT("Tropical Jungle"));
        Data.BaseTemperature = 30.0f;
        Data.BaseHumidity = 0.90f;
        Data.DinosaurDensity = 1.5f;
        Data.VegetationDensity = 2.5f;
        Data.DangerLevel = 4;
        Data.NativeDinosaurSpecies = { TEXT("TyrannosaurusRex"), TEXT("Spinosaurus"), TEXT("Carnotaurus") };
        BiomeDataTable.Add(EEng_BiomeType::Jungle, Data);
    }

    // Swamp
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Swamp;
        Data.DisplayName = FText::FromString(TEXT("Prehistoric Swamp"));
        Data.BaseTemperature = 26.0f;
        Data.BaseHumidity = 0.95f;
        Data.DinosaurDensity = 1.0f;
        Data.VegetationDensity = 1.8f;
        Data.DangerLevel = 3;
        Data.NativeDinosaurSpecies = { TEXT("Spinosaurus"), TEXT("Sarcosuchus"), TEXT("Parasaurolophus") };
        BiomeDataTable.Add(EEng_BiomeType::Swamp, Data);
    }

    // Volcanic
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Volcanic;
        Data.DisplayName = FText::FromString(TEXT("Volcanic Badlands"));
        Data.BaseTemperature = 45.0f;
        Data.BaseHumidity = 0.15f;
        Data.DinosaurDensity = 0.3f;
        Data.VegetationDensity = 0.1f;
        Data.DangerLevel = 5;
        Data.NativeDinosaurSpecies = { TEXT("Abelisaurus") };
        BiomeDataTable.Add(EEng_BiomeType::Volcanic, Data);
    }

    // Coastal
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Coastal;
        Data.DisplayName = FText::FromString(TEXT("Coastal Shore"));
        Data.BaseTemperature = 24.0f;
        Data.BaseHumidity = 0.65f;
        Data.DinosaurDensity = 0.8f;
        Data.VegetationDensity = 0.6f;
        Data.DangerLevel = 2;
        Data.NativeDinosaurSpecies = { TEXT("Mosasaurus"), TEXT("Pteranodon"), TEXT("Ichthyosaurus") };
        BiomeDataTable.Add(EEng_BiomeType::Coastal, Data);
    }

    // Alpine
    {
        FEng_BiomeData Data;
        Data.BiomeType = EEng_BiomeType::Alpine;
        Data.DisplayName = FText::FromString(TEXT("Alpine Highland"));
        Data.BaseTemperature = 5.0f;
        Data.BaseHumidity = 0.40f;
        Data.DinosaurDensity = 0.4f;
        Data.VegetationDensity = 0.3f;
        Data.DangerLevel = 3;
        Data.NativeDinosaurSpecies = { TEXT("Pachycephalosaurus"), TEXT("Ankylosaurus") };
        BiomeDataTable.Add(EEng_BiomeType::Alpine, Data);
    }
}

FColor UBiomeManager::GetBiomeDebugColor(EEng_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EEng_BiomeType::Grassland:  return FColor(100, 200, 50);
        case EEng_BiomeType::Forest:     return FColor(0, 100, 0);
        case EEng_BiomeType::Desert:     return FColor(220, 180, 80);
        case EEng_BiomeType::Jungle:     return FColor(0, 150, 30);
        case EEng_BiomeType::Swamp:      return FColor(60, 100, 60);
        case EEng_BiomeType::Volcanic:   return FColor(200, 50, 0);
        case EEng_BiomeType::Coastal:    return FColor(50, 150, 200);
        case EEng_BiomeType::Alpine:     return FColor(200, 220, 240);
        default:                         return FColor::White;
    }
}
