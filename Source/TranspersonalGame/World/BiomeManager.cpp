// BiomeManager.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Biome system implementation: classification, transitions, dinosaur spawning rules

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY_STATIC(LogBiomeManager, Log, All);

// ============================================================
// Constructor
// ============================================================

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 2.0f; // Biome checks every 2 seconds for performance

    CurrentPlayerBiome = EBiomeType::Forest;
    bDebugDrawBiomes = false;
    BiomeTransitionBlendRadius = 200.0f;
    MaxActiveBiomes = 8;
}

// ============================================================
// Lifecycle
// ============================================================

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize default biome definitions if none configured
    if (BiomeDefinitions.Num() == 0)
    {
        InitializeDefaultBiomes();
    }

    UE_LOG(LogBiomeManager, Log, TEXT("BiomeManager initialized with %d biome definitions"), BiomeDefinitions.Num());
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update player biome classification
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        EBiomeType NewBiome = ClassifyLocationBiome(PlayerLocation);

        if (NewBiome != CurrentPlayerBiome)
        {
            OnBiomeTransition(CurrentPlayerBiome, NewBiome);
            CurrentPlayerBiome = NewBiome;
        }
    }

#if WITH_EDITOR
    if (bDebugDrawBiomes)
    {
        DebugDrawBiomeBoundaries();
    }
#endif
}

// ============================================================
// Biome Classification
// ============================================================

EBiomeType ABiomeManager::ClassifyLocationBiome(const FVector& WorldLocation) const
{
    if (BiomeDefinitions.Num() == 0)
    {
        return EBiomeType::Forest;
    }

    float ClosestDistSq = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Forest;

    for (const FEng_BiomeDefinition& Def : BiomeDefinitions)
    {
        FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);
        float DistSq = FVector2D::DistSquared(Loc2D, Def.CenterLocation);

        if (DistSq < ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            ClosestBiome = Def.BiomeType;
        }
    }

    return ClosestBiome;
}

float ABiomeManager::GetBiomeBlendWeight(const FVector& WorldLocation, EBiomeType BiomeType) const
{
    for (const FEng_BiomeDefinition& Def : BiomeDefinitions)
    {
        if (Def.BiomeType != BiomeType) continue;

        FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);
        float Dist = FVector2D::Distance(Loc2D, Def.CenterLocation);

        if (Dist >= Def.Radius + BiomeTransitionBlendRadius)
        {
            return 0.0f;
        }
        if (Dist <= Def.Radius - BiomeTransitionBlendRadius)
        {
            return 1.0f;
        }

        // Smooth blend in transition zone
        float Alpha = (Dist - (Def.Radius - BiomeTransitionBlendRadius)) / (2.0f * BiomeTransitionBlendRadius);
        return FMath::SmoothStep(0.0f, 1.0f, 1.0f - Alpha);
    }

    return 0.0f;
}

const FEng_BiomeDefinition* ABiomeManager::GetBiomeDefinition(EBiomeType BiomeType) const
{
    for (const FEng_BiomeDefinition& Def : BiomeDefinitions)
    {
        if (Def.BiomeType == BiomeType)
        {
            return &Def;
        }
    }
    return nullptr;
}

TArray<EBiomeType> ABiomeManager::GetBiomesAtLocation(const FVector& WorldLocation, float SearchRadius) const
{
    TArray<EBiomeType> FoundBiomes;

    for (const FEng_BiomeDefinition& Def : BiomeDefinitions)
    {
        FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);
        float Dist = FVector2D::Distance(Loc2D, Def.CenterLocation);

        if (Dist <= Def.Radius + SearchRadius)
        {
            FoundBiomes.AddUnique(Def.BiomeType);
        }
    }

    return FoundBiomes;
}

// ============================================================
// Dinosaur Spawning Rules
// ============================================================

TArray<EDinosaurSpecies> ABiomeManager::GetValidDinosaursForBiome(EBiomeType BiomeType) const
{
    const FEng_BiomeDefinition* Def = GetBiomeDefinition(BiomeType);
    if (Def)
    {
        return Def->NativeDinosaurs;
    }

    // Fallback: return common species for unknown biomes
    return TArray<EDinosaurSpecies>{ EDinosaurSpecies::Compsognathus };
}

bool ABiomeManager::CanSpawnDinosaurInBiome(EDinosaurSpecies Species, EBiomeType BiomeType) const
{
    TArray<EDinosaurSpecies> ValidSpecies = GetValidDinosaursForBiome(BiomeType);
    return ValidSpecies.Contains(Species);
}

float ABiomeManager::GetDinosaurAggressionModifier(EDinosaurSpecies Species, EBiomeType BiomeType) const
{
    // Territorial biomes increase aggression
    switch (BiomeType)
    {
        case EBiomeType::Canyon:    return 1.4f;  // Confined spaces = more territorial
        case EBiomeType::Swamp:     return 1.2f;  // Ambush predator territory
        case EBiomeType::Forest:    return 1.1f;  // Cover-based hunting
        case EBiomeType::Savanna:   return 0.9f;  // Open ground = less ambush
        case EBiomeType::Mountain:  return 1.3f;  // Defensive high ground
        case EBiomeType::River:     return 0.8f;  // Water = escape routes
        case EBiomeType::Lake:      return 0.7f;  // Open water = visibility
        case EBiomeType::Desert:    return 1.0f;  // Baseline
        default:                    return 1.0f;
    }
}

// ============================================================
// Biome Transition Events
// ============================================================

void ABiomeManager::OnBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome)
{
    UE_LOG(LogBiomeManager, Log, TEXT("Player biome transition: %d -> %d"),
        (int32)FromBiome, (int32)ToBiome);

    OnPlayerBiomeChanged.Broadcast(FromBiome, ToBiome);
}

// ============================================================
// Default Biome Initialization
// ============================================================

void ABiomeManager::InitializeDefaultBiomes()
{
    // Forest biome — central, lush, medium danger
    {
        FEng_BiomeDefinition Forest;
        Forest.BiomeType = EBiomeType::Forest;
        Forest.BiomeName = TEXT("Cretaceous Forest");
        Forest.CenterLocation = FVector2D(0.0f, 0.0f);
        Forest.Radius = 3000.0f;
        Forest.BaseTemperature = 22.0f;
        Forest.Humidity = 75.0f;
        Forest.NativeDinosaurs = {
            EDinosaurSpecies::Velociraptor,
            EDinosaurSpecies::Compsognathus,
            EDinosaurSpecies::Stegosaurus,
            EDinosaurSpecies::Parasaurolophus
        };
        BiomeDefinitions.Add(Forest);
    }

    // Savanna biome — open plains, high visibility, T-Rex territory
    {
        FEng_BiomeDefinition Savanna;
        Savanna.BiomeType = EBiomeType::Savanna;
        Savanna.BiomeName = TEXT("Open Savanna");
        Savanna.CenterLocation = FVector2D(4000.0f, 0.0f);
        Savanna.Radius = 4000.0f;
        Savanna.BaseTemperature = 32.0f;
        Savanna.Humidity = 30.0f;
        Savanna.NativeDinosaurs = {
            EDinosaurSpecies::TRex,
            EDinosaurSpecies::Triceratops,
            EDinosaurSpecies::Ankylosaurus,
            EDinosaurSpecies::Brachiosaurus
        };
        BiomeDefinitions.Add(Savanna);
    }

    // Swamp biome — low visibility, ambush danger
    {
        FEng_BiomeDefinition Swamp;
        Swamp.BiomeType = EBiomeType::Swamp;
        Swamp.BiomeName = TEXT("Primordial Swamp");
        Swamp.CenterLocation = FVector2D(-3000.0f, 2000.0f);
        Swamp.Radius = 2500.0f;
        Swamp.BaseTemperature = 28.0f;
        Swamp.Humidity = 95.0f;
        Swamp.NativeDinosaurs = {
            EDinosaurSpecies::Compsognathus,
            EDinosaurSpecies::Velociraptor
        };
        BiomeDefinitions.Add(Swamp);
    }

    // Mountain biome — high altitude, sparse, dangerous terrain
    {
        FEng_BiomeDefinition Mountain;
        Mountain.BiomeType = EBiomeType::Mountain;
        Mountain.BiomeName = TEXT("Rocky Highlands");
        Mountain.CenterLocation = FVector2D(0.0f, -5000.0f);
        Mountain.Radius = 3500.0f;
        Mountain.BaseTemperature = 8.0f;
        Mountain.Humidity = 40.0f;
        Mountain.NativeDinosaurs = {
            EDinosaurSpecies::Ankylosaurus,
            EDinosaurSpecies::Stegosaurus
        };
        BiomeDefinitions.Add(Mountain);
    }

    // River biome — linear, resource-rich, migration corridor
    {
        FEng_BiomeDefinition River;
        River.BiomeType = EBiomeType::River;
        River.BiomeName = TEXT("Great River Corridor");
        River.CenterLocation = FVector2D(-1000.0f, 1000.0f);
        River.Radius = 800.0f;
        River.BaseTemperature = 20.0f;
        River.Humidity = 85.0f;
        River.NativeDinosaurs = {
            EDinosaurSpecies::Brachiosaurus,
            EDinosaurSpecies::Parasaurolophus,
            EDinosaurSpecies::Compsognathus
        };
        BiomeDefinitions.Add(River);
    }

    UE_LOG(LogBiomeManager, Log, TEXT("Initialized %d default biome definitions"), BiomeDefinitions.Num());
}

// ============================================================
// Debug
// ============================================================

void ABiomeManager::DebugDrawBiomeBoundaries() const
{
#if WITH_EDITOR
    for (const FEng_BiomeDefinition& Def : BiomeDefinitions)
    {
        FVector Center3D(Def.CenterLocation.X, Def.CenterLocation.Y, 100.0f);
        FColor DebugColor = FColor::Green;

        switch (Def.BiomeType)
        {
            case EBiomeType::Swamp:     DebugColor = FColor::Cyan;    break;
            case EBiomeType::Forest:    DebugColor = FColor::Green;   break;
            case EBiomeType::Savanna:   DebugColor = FColor::Yellow;  break;
            case EBiomeType::Desert:    DebugColor = FColor::Orange;  break;
            case EBiomeType::Mountain:  DebugColor = FColor::White;   break;
            case EBiomeType::River:     DebugColor = FColor::Blue;    break;
            case EBiomeType::Lake:      DebugColor = FColor::Purple;  break;
            case EBiomeType::Canyon:    DebugColor = FColor::Red;     break;
        }

        DrawDebugCircle(GetWorld(), Center3D, Def.Radius, 64,
            DebugColor, false, 2.0f, 0, 5.0f,
            FVector(1,0,0), FVector(0,1,0));
    }
#endif
}
