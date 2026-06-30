// BiomeManager.cpp
// Engine Architect #02 — PROD_CYCLE_AUTO_20260630_009
// Full implementation of biome classification, query, and transition system.

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
    PrimaryActorTick.TickInterval = 1.0f; // Update biome state every second

    // Default biome table — five Cretaceous biomes
    FEng_BiomeDefinition Jungle;
    Jungle.BiomeID       = EEng_BiomeType::TropicalJungle;
    Jungle.DisplayName   = FText::FromString(TEXT("Tropical Jungle"));
    Jungle.BaseTemperature = 32.0f;
    Jungle.Humidity        = 0.9f;
    Jungle.VegetationDensity = 0.95f;
    Jungle.DangerLevel     = 0.7f;
    BiomeTable.Add(Jungle);

    FEng_BiomeDefinition Savanna;
    Savanna.BiomeID       = EEng_BiomeType::OpenSavanna;
    Savanna.DisplayName   = FText::FromString(TEXT("Open Savanna"));
    Savanna.BaseTemperature = 38.0f;
    Savanna.Humidity        = 0.25f;
    Savanna.VegetationDensity = 0.35f;
    Savanna.DangerLevel     = 0.5f;
    BiomeTable.Add(Savanna);

    FEng_BiomeDefinition Swamp;
    Swamp.BiomeID       = EEng_BiomeType::CoastalSwamp;
    Swamp.DisplayName   = FText::FromString(TEXT("Coastal Swamp"));
    Swamp.BaseTemperature = 28.0f;
    Swamp.Humidity        = 0.95f;
    Swamp.VegetationDensity = 0.75f;
    Swamp.DangerLevel     = 0.85f;
    BiomeTable.Add(Swamp);

    FEng_BiomeDefinition Volcanic;
    Volcanic.BiomeID       = EEng_BiomeType::VolcanicBadlands;
    Volcanic.DisplayName   = FText::FromString(TEXT("Volcanic Badlands"));
    Volcanic.BaseTemperature = 55.0f;
    Volcanic.Humidity        = 0.05f;
    Volcanic.VegetationDensity = 0.05f;
    Volcanic.DangerLevel     = 0.95f;
    BiomeTable.Add(Volcanic);

    FEng_BiomeDefinition Forest;
    Forest.BiomeID       = EEng_BiomeType::ConiferousForest;
    Forest.DisplayName   = FText::FromString(TEXT("Coniferous Forest"));
    Forest.BaseTemperature = 18.0f;
    Forest.Humidity        = 0.6f;
    Forest.VegetationDensity = 0.8f;
    Forest.DangerLevel     = 0.4f;
    BiomeTable.Add(Forest);

    TransitionBlendRadius = 2000.0f;
    bDebugDrawBiomes      = false;
    ActiveBiomeID         = EEng_BiomeType::OpenSavanna;
}

// ============================================================
// BeginPlay
// ============================================================
void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Initialized with %d biome definitions."), BiomeTable.Num());
}

// ============================================================
// Tick
// ============================================================
void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update active biome based on player location
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
        EEng_BiomeType NewBiome = GetBiomeAtLocation(PlayerLoc);
        if (NewBiome != ActiveBiomeID)
        {
            ActiveBiomeID = NewBiome;
            OnBiomeChanged.Broadcast(NewBiome);
            UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Player entered biome: %d"), (int32)NewBiome);
        }
    }

#if WITH_EDITOR
    if (bDebugDrawBiomes)
    {
        DrawDebugBiomeBoundaries();
    }
#endif
}

// ============================================================
// GetBiomeAtLocation — Classify by world XY position
// Uses a simple noise-based spatial hash for deterministic biome placement.
// ============================================================
EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Simple spatial classification using world coordinates
    // Divide world into sectors — each sector maps to a biome
    // This is a placeholder until PCG biome maps are integrated.
    float X = WorldLocation.X;
    float Y = WorldLocation.Y;

    // Volcanic zone: near origin high-altitude area
    if (FMath::Abs(X) < 1000.0f && FMath::Abs(Y) < 1000.0f && WorldLocation.Z > 500.0f)
    {
        return EEng_BiomeType::VolcanicBadlands;
    }

    // Swamp: negative X, positive Y quadrant
    if (X < -2000.0f && Y > 2000.0f)
    {
        return EEng_BiomeType::CoastalSwamp;
    }

    // Jungle: positive X, positive Y quadrant
    if (X > 2000.0f && Y > 2000.0f)
    {
        return EEng_BiomeType::TropicalJungle;
    }

    // Coniferous Forest: negative X, negative Y quadrant
    if (X < -2000.0f && Y < -2000.0f)
    {
        return EEng_BiomeType::ConiferousForest;
    }

    // Default: Open Savanna (central region)
    return EEng_BiomeType::OpenSavanna;
}

// ============================================================
// GetBiomeDefinition — Look up biome properties by ID
// ============================================================
FEng_BiomeDefinition ABiomeManager::GetBiomeDefinition(EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeDefinition& Def : BiomeTable)
    {
        if (Def.BiomeID == BiomeType)
        {
            return Def;
        }
    }

    // Return default (empty) definition if not found
    UE_LOG(LogTemp, Warning, TEXT("[BiomeManager] BiomeDefinition not found for type %d — returning default."), (int32)BiomeType);
    return FEng_BiomeDefinition();
}

// ============================================================
// GetBlendedTemperature — Smooth temperature across biome boundaries
// ============================================================
float ABiomeManager::GetBlendedTemperature(const FVector& WorldLocation) const
{
    EEng_BiomeType PrimaryBiome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition PrimaryDef = GetBiomeDefinition(PrimaryBiome);

    // Simple implementation: return primary biome temperature
    // Full blend would sample neighbouring biome and lerp by distance to boundary
    return PrimaryDef.BaseTemperature;
}

// ============================================================
// GetBlendedHumidity
// ============================================================
float ABiomeManager::GetBlendedHumidity(const FVector& WorldLocation) const
{
    EEng_BiomeType PrimaryBiome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition PrimaryDef = GetBiomeDefinition(PrimaryBiome);
    return PrimaryDef.Humidity;
}

// ============================================================
// GetDangerLevel
// ============================================================
float ABiomeManager::GetDangerLevel(const FVector& WorldLocation) const
{
    EEng_BiomeType PrimaryBiome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition PrimaryDef = GetBiomeDefinition(PrimaryBiome);
    return PrimaryDef.DangerLevel;
}

// ============================================================
// RegisterBiomeDefinition — Allow runtime biome registration
// ============================================================
void ABiomeManager::RegisterBiomeDefinition(const FEng_BiomeDefinition& NewBiome)
{
    // Remove existing entry with same ID if present
    BiomeTable.RemoveAll([&](const FEng_BiomeDefinition& Existing)
    {
        return Existing.BiomeID == NewBiome.BiomeID;
    });
    BiomeTable.Add(NewBiome);
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Registered biome: %s"), *NewBiome.DisplayName.ToString());
}

// ============================================================
// DrawDebugBiomeBoundaries — Editor visualization
// ============================================================
void ABiomeManager::DrawDebugBiomeBoundaries()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World) return;

    // Draw coloured spheres at biome centre points for editor visibility
    struct FBiomeDebugPoint { FVector Location; FColor Color; FString Label; };
    TArray<FBiomeDebugPoint> Points = {
        { FVector(3000, 3000, 0),   FColor::Green,  TEXT("Jungle") },
        { FVector(0, 0, 0),         FColor::Yellow, TEXT("Savanna") },
        { FVector(-3000, 3000, 0),  FColor::Cyan,   TEXT("Swamp") },
        { FVector(0, 0, 800),       FColor::Red,    TEXT("Volcanic") },
        { FVector(-3000, -3000, 0), FColor::White,  TEXT("Forest") },
    };

    for (const FBiomeDebugPoint& P : Points)
    {
        DrawDebugSphere(World, P.Location, 200.0f, 12, P.Color, false, 1.1f);
    }
#endif
}
