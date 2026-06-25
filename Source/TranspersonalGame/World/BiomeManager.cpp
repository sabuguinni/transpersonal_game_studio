// BiomeManager.cpp
// Core Systems Programmer — Agent #03
// Transpersonal Game Studio — Prehistoric Survival Game

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================
UBiomeManager::UBiomeManager()
{
    CurrentBiome = ECore_BiomeType::Jungle;
    TransitionBlend = 0.0f;
    bBiomeTransitionActive = false;
    BiomeTransitionDuration = 5.0f;
    BiomeTransitionElapsed = 0.0f;
    TemperatureBase = 28.0f;
    HumidityBase = 0.75f;
    DangerLevel = 0.5f;
}

// ============================================================
// Initialization
// ============================================================
void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Initialized — current biome: %d"), (int32)CurrentBiome);
    ApplyBiomeEnvironment(CurrentBiome);
}

void UBiomeManager::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Deinitialized"));
}

// ============================================================
// Tick
// ============================================================
void UBiomeManager::Tick(float DeltaTime)
{
    if (bBiomeTransitionActive)
    {
        BiomeTransitionElapsed += DeltaTime;
        TransitionBlend = FMath::Clamp(BiomeTransitionElapsed / BiomeTransitionDuration, 0.0f, 1.0f);

        if (TransitionBlend >= 1.0f)
        {
            bBiomeTransitionActive = false;
            CurrentBiome = TargetBiome;
            TransitionBlend = 0.0f;
            BiomeTransitionElapsed = 0.0f;
            OnBiomeChanged.Broadcast(CurrentBiome);
            UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Transition complete — new biome: %d"), (int32)CurrentBiome);
        }
    }
}

bool UBiomeManager::IsTickable() const
{
    return true;
}

TStatId UBiomeManager::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UBiomeManager, STATGROUP_Tickables);
}

// ============================================================
// Biome Query
// ============================================================
ECore_BiomeType UBiomeManager::GetCurrentBiome() const
{
    return CurrentBiome;
}

ECore_BiomeType UBiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    // Simple height-based biome determination for prototype
    // Z < 0     → Swamp
    // Z 0-500   → Jungle (default)
    // Z 500-1500 → Forest
    // Z > 1500  → Mountain
    float Z = WorldLocation.Z;
    if (Z < 0.0f)        return ECore_BiomeType::Swamp;
    if (Z < 500.0f)      return ECore_BiomeType::Jungle;
    if (Z < 1500.0f)     return ECore_BiomeType::Forest;
    return ECore_BiomeType::Mountain;
}

FCore_BiomeData UBiomeManager::GetBiomeData(ECore_BiomeType BiomeType) const
{
    // Return cached data if available
    if (const FCore_BiomeData* Found = BiomeDataMap.Find(BiomeType))
    {
        return *Found;
    }

    // Build default data per biome type
    FCore_BiomeData Data;
    Data.BiomeType = BiomeType;

    switch (BiomeType)
    {
    case ECore_BiomeType::Jungle:
        Data.DisplayName = FText::FromString(TEXT("Cretaceous Jungle"));
        Data.BaseTemperature = 32.0f;
        Data.Humidity = 0.85f;
        Data.DangerRating = 0.7f;
        Data.FoliageDensity = 1.0f;
        Data.bAllowsLargeHerbivores = true;
        Data.bAllowsCarnivores = true;
        Data.PrimaryFogColor = FLinearColor(0.05f, 0.12f, 0.05f, 1.0f);
        break;

    case ECore_BiomeType::Savanna:
        Data.DisplayName = FText::FromString(TEXT("Open Savanna"));
        Data.BaseTemperature = 38.0f;
        Data.Humidity = 0.25f;
        Data.DangerRating = 0.6f;
        Data.FoliageDensity = 0.3f;
        Data.bAllowsLargeHerbivores = true;
        Data.bAllowsCarnivores = true;
        Data.PrimaryFogColor = FLinearColor(0.18f, 0.14f, 0.06f, 1.0f);
        break;

    case ECore_BiomeType::Forest:
        Data.DisplayName = FText::FromString(TEXT("Conifer Forest"));
        Data.BaseTemperature = 18.0f;
        Data.Humidity = 0.6f;
        Data.DangerRating = 0.5f;
        Data.FoliageDensity = 0.8f;
        Data.bAllowsLargeHerbivores = true;
        Data.bAllowsCarnivores = false;
        Data.PrimaryFogColor = FLinearColor(0.08f, 0.1f, 0.08f, 1.0f);
        break;

    case ECore_BiomeType::Swamp:
        Data.DisplayName = FText::FromString(TEXT("Primordial Swamp"));
        Data.BaseTemperature = 26.0f;
        Data.Humidity = 0.95f;
        Data.DangerRating = 0.8f;
        Data.FoliageDensity = 0.6f;
        Data.bAllowsLargeHerbivores = false;
        Data.bAllowsCarnivores = true;
        Data.PrimaryFogColor = FLinearColor(0.04f, 0.08f, 0.04f, 1.0f);
        break;

    case ECore_BiomeType::Mountain:
        Data.DisplayName = FText::FromString(TEXT("Rocky Highlands"));
        Data.BaseTemperature = 8.0f;
        Data.Humidity = 0.3f;
        Data.DangerRating = 0.4f;
        Data.FoliageDensity = 0.15f;
        Data.bAllowsLargeHerbivores = false;
        Data.bAllowsCarnivores = false;
        Data.PrimaryFogColor = FLinearColor(0.12f, 0.12f, 0.15f, 1.0f);
        break;

    case ECore_BiomeType::Coastal:
        Data.DisplayName = FText::FromString(TEXT("Coastal Shore"));
        Data.BaseTemperature = 24.0f;
        Data.Humidity = 0.7f;
        Data.DangerRating = 0.45f;
        Data.FoliageDensity = 0.4f;
        Data.bAllowsLargeHerbivores = true;
        Data.bAllowsCarnivores = false;
        Data.PrimaryFogColor = FLinearColor(0.1f, 0.15f, 0.18f, 1.0f);
        break;

    default:
        Data.DisplayName = FText::FromString(TEXT("Unknown Biome"));
        Data.BaseTemperature = 20.0f;
        Data.Humidity = 0.5f;
        Data.DangerRating = 0.5f;
        Data.FoliageDensity = 0.5f;
        break;
    }

    return Data;
}

// ============================================================
// Biome Transition
// ============================================================
void UBiomeManager::TransitionToBiome(ECore_BiomeType NewBiome, float Duration)
{
    if (NewBiome == CurrentBiome && !bBiomeTransitionActive)
    {
        return;
    }

    TargetBiome = NewBiome;
    BiomeTransitionDuration = FMath::Max(Duration, 0.1f);
    BiomeTransitionElapsed = 0.0f;
    TransitionBlend = 0.0f;
    bBiomeTransitionActive = true;

    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Transition started: %d → %d over %.1fs"),
        (int32)CurrentBiome, (int32)NewBiome, Duration);
}

void UBiomeManager::SetBiomeImmediate(ECore_BiomeType NewBiome)
{
    bBiomeTransitionActive = false;
    TransitionBlend = 0.0f;
    BiomeTransitionElapsed = 0.0f;
    CurrentBiome = NewBiome;
    ApplyBiomeEnvironment(NewBiome);
    OnBiomeChanged.Broadcast(NewBiome);
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Biome set immediately: %d"), (int32)NewBiome);
}

// ============================================================
// Environment Application
// ============================================================
void UBiomeManager::ApplyBiomeEnvironment(ECore_BiomeType BiomeType)
{
    FCore_BiomeData Data = GetBiomeData(BiomeType);
    TemperatureBase = Data.BaseTemperature;
    HumidityBase = Data.Humidity;
    DangerLevel = Data.DangerRating;

    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Applied biome env — Temp:%.1f Humidity:%.2f Danger:%.2f"),
        TemperatureBase, HumidityBase, DangerLevel);
}

// ============================================================
// Survival Queries
// ============================================================
float UBiomeManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    ECore_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FCore_BiomeData Data = GetBiomeData(Biome);
    // Add altitude cooling: -2°C per 100 units above 0
    float AltitudeFactor = FMath::Max(0.0f, WorldLocation.Z / 100.0f) * -2.0f;
    return Data.BaseTemperature + AltitudeFactor;
}

float UBiomeManager::GetHumidityAtLocation(FVector WorldLocation) const
{
    ECore_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FCore_BiomeData Data = GetBiomeData(Biome);
    return Data.Humidity;
}

float UBiomeManager::GetDangerLevelAtLocation(FVector WorldLocation) const
{
    ECore_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FCore_BiomeData Data = GetBiomeData(Biome);
    return Data.DangerRating;
}

bool UBiomeManager::IsLocationSafeForPlayer(FVector WorldLocation) const
{
    return GetDangerLevelAtLocation(WorldLocation) < 0.5f;
}

// ============================================================
// Dinosaur Spawning Rules
// ============================================================
bool UBiomeManager::CanSpawnDinosaurInBiome(ECore_BiomeType BiomeType, bool bIsLargeHerbivore, bool bIsCarnivore) const
{
    FCore_BiomeData Data = GetBiomeData(BiomeType);
    if (bIsLargeHerbivore && !Data.bAllowsLargeHerbivores) return false;
    if (bIsCarnivore && !Data.bAllowsCarnivores) return false;
    return true;
}

TArray<FVector> UBiomeManager::GetValidSpawnLocationsInBiome(ECore_BiomeType BiomeType, int32 Count, float SearchRadius) const
{
    TArray<FVector> Locations;
    UWorld* World = GetWorld();
    if (!World) return Locations;

    // Simple grid-based spawn location search
    FVector Center = FVector::ZeroVector;
    float Step = SearchRadius / FMath::Sqrt((float)FMath::Max(Count, 1));

    for (int32 i = 0; i < Count * 4 && Locations.Num() < Count; ++i)
    {
        float Angle = (float)i * 137.5f * (PI / 180.0f); // Golden angle distribution
        float Radius = FMath::Sqrt((float)i / (float)(Count * 4)) * SearchRadius;
        FVector Candidate = Center + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            500.0f // Start high for line trace
        );

        // Line trace to find ground
        FHitResult Hit;
        FVector TraceEnd = Candidate - FVector(0, 0, 2000.0f);
        FCollisionQueryParams Params;
        if (World->LineTraceSingleByChannel(Hit, Candidate, TraceEnd, ECC_WorldStatic, Params))
        {
            ECore_BiomeType HitBiome = GetBiomeAtLocation(Hit.Location);
            if (HitBiome == BiomeType)
            {
                Locations.Add(Hit.Location);
            }
        }
    }

    return Locations;
}
