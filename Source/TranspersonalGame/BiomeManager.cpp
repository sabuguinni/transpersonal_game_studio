// BiomeManager.cpp — P1 World Generation: Biome System
// Engine Architect #02 — Transpersonal Game Studio
// Implements ABiomeManager: zone registration, biome queries, blending, debug visuals.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// ============================================================
// Constructor
// ============================================================

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false; // Only ticks when debug is enabled
    PrimaryActorTick.bStartWithTickEnabled = false;

    // Default: no zones — call InitializeDefaultBiomes() in editor or BeginPlay
}

// ============================================================
// Lifecycle
// ============================================================

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    if (BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }

    if (bDrawDebugVisuals)
    {
        PrimaryActorTick.bCanEverTick = true;
        SetActorTickEnabled(true);
    }

    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Initialized with %d biome zones."), BiomeZones.Num());
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDrawDebugVisuals)
    {
        DrawBiomeDebugSpheres();
    }
}

// ============================================================
// Static Accessor
// ============================================================

ABiomeManager* ABiomeManager::GetInstance(UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return nullptr;
    }

    // Find first BiomeManager in the world
    for (TActorIterator<ABiomeManager> It(World); It; ++It)
    {
        return *It;
    }

    UE_LOG(LogTemp, Warning, TEXT("[BiomeManager] No instance found in world. Spawning one."));

    // Auto-spawn if missing
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ABiomeManager* NewManager = World->SpawnActor<ABiomeManager>(ABiomeManager::StaticClass(),
        FVector::ZeroVector, FRotator::ZeroRotator, Params);

    if (NewManager)
    {
        NewManager->InitializeDefaultBiomes();
    }

    return NewManager;
}

// ============================================================
// Biome Query API
// ============================================================

EBiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    const FBiomeZone* Nearest = FindNearestZone(WorldLocation);
    if (Nearest)
    {
        return Nearest->BiomeType;
    }
    return EBiomeType::Savanna; // Default fallback
}

FBiomeZone ABiomeManager::GetBiomeZoneAtLocation(const FVector& WorldLocation) const
{
    const FBiomeZone* Nearest = FindNearestZone(WorldLocation);
    if (Nearest)
    {
        return *Nearest;
    }
    return FBiomeZone(); // Default empty zone
}

FBiomeTransition ABiomeManager::GetTransitionAtLocation(const FVector& WorldLocation) const
{
    FBiomeTransition Transition;

    if (BiomeZones.Num() < 2)
    {
        return Transition;
    }

    // Find two nearest zones and compute blend
    float Dist1 = FLT_MAX;
    float Dist2 = FLT_MAX;
    const FBiomeZone* Zone1 = nullptr;
    const FBiomeZone* Zone2 = nullptr;

    for (const FBiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist(WorldLocation, Zone.CenterLocation);
        if (Dist < Dist1)
        {
            Dist2 = Dist1;
            Zone2 = Zone1;
            Dist1 = Dist;
            Zone1 = &Zone;
        }
        else if (Dist < Dist2)
        {
            Dist2 = Dist;
            Zone2 = &Zone;
        }
    }

    if (Zone1 && Zone2)
    {
        Transition.BiomeA = Zone1->BiomeType;
        Transition.BiomeB = Zone2->BiomeType;

        // Blend alpha: 0 = fully Zone1, 1 = fully Zone2
        float TotalDist = Dist1 + Dist2;
        Transition.BlendAlpha = (TotalDist > 0.0f) ? (Dist1 / TotalDist) : 0.0f;
    }

    return Transition;
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    const FBiomeZone* Nearest = FindNearestZone(WorldLocation);
    if (Nearest)
    {
        // Add altitude modifier: -2°C per 1000 units of height
        float AltitudeMod = -(WorldLocation.Z / 1000.0f) * 2.0f;
        return Nearest->TemperatureBase + AltitudeMod;
    }
    return 25.0f;
}

float ABiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    const FBiomeZone* Nearest = FindNearestZone(WorldLocation);
    return Nearest ? Nearest->HumidityBase : 0.5f;
}

float ABiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    const FBiomeZone* Nearest = FindNearestZone(WorldLocation);
    return Nearest ? Nearest->DangerLevel : 0.5f;
}

// ============================================================
// Zone Management
// ============================================================

void ABiomeManager::RegisterBiomeZone(const FBiomeZone& Zone)
{
    BiomeZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Registered biome zone. Total: %d"), BiomeZones.Num());
}

void ABiomeManager::ClearAllZones()
{
    BiomeZones.Empty();
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] All biome zones cleared."));
}

int32 ABiomeManager::GetZoneCount() const
{
    return BiomeZones.Num();
}

// ============================================================
// Default World Biomes — Cretaceous Prehistoric World
// ============================================================

void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeZones.Empty();

    // --- Forest Biome (North) ---
    {
        FBiomeZone Forest;
        Forest.BiomeType = EBiomeType::Forest;
        Forest.CenterLocation = FVector(0.0f, 3000.0f, 0.0f);
        Forest.Radius = 4000.0f;
        Forest.TemperatureBase = 22.0f;
        Forest.HumidityBase = 0.8f;
        Forest.DangerLevel = 0.6f;
        Forest.InhabitingSpecies.Add(EDinosaurSpecies::Velociraptor);
        Forest.InhabitingSpecies.Add(EDinosaurSpecies::Compsognathus);
        BiomeZones.Add(Forest);
    }

    // --- Savanna Biome (East) ---
    {
        FBiomeZone Savanna;
        Savanna.BiomeType = EBiomeType::Savanna;
        Savanna.CenterLocation = FVector(3000.0f, 0.0f, 0.0f);
        Savanna.Radius = 5000.0f;
        Savanna.TemperatureBase = 32.0f;
        Savanna.HumidityBase = 0.3f;
        Savanna.DangerLevel = 0.8f;
        Savanna.InhabitingSpecies.Add(EDinosaurSpecies::TyrannosaurusRex);
        Savanna.InhabitingSpecies.Add(EDinosaurSpecies::Triceratops);
        Savanna.InhabitingSpecies.Add(EDinosaurSpecies::Parasaurolophus);
        BiomeZones.Add(Savanna);
    }

    // --- Swampland Biome (South) ---
    {
        FBiomeZone Swamp;
        Swamp.BiomeType = EBiomeType::Swampland;
        Swamp.CenterLocation = FVector(0.0f, -3000.0f, -100.0f);
        Swamp.Radius = 3500.0f;
        Swamp.TemperatureBase = 28.0f;
        Swamp.HumidityBase = 0.95f;
        Swamp.DangerLevel = 0.7f;
        Swamp.InhabitingSpecies.Add(EDinosaurSpecies::Ankylosaurus);
        BiomeZones.Add(Swamp);
    }

    // --- Mountain Biome (West) ---
    {
        FBiomeZone Mountain;
        Mountain.BiomeType = EBiomeType::Mountains;
        Mountain.CenterLocation = FVector(-3000.0f, 0.0f, 500.0f);
        Mountain.Radius = 4500.0f;
        Mountain.TemperatureBase = 12.0f;
        Mountain.HumidityBase = 0.4f;
        Mountain.DangerLevel = 0.5f;
        Mountain.InhabitingSpecies.Add(EDinosaurSpecies::Pteranodon);
        BiomeZones.Add(Mountain);
    }

    // --- River Valley Biome (Center) ---
    {
        FBiomeZone River;
        River.BiomeType = EBiomeType::RiverValley;
        River.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
        River.Radius = 2000.0f;
        River.TemperatureBase = 24.0f;
        River.HumidityBase = 0.9f;
        River.DangerLevel = 0.4f;
        River.InhabitingSpecies.Add(EDinosaurSpecies::Brachiosaurus);
        River.InhabitingSpecies.Add(EDinosaurSpecies::Parasaurolophus);
        BiomeZones.Add(River);
    }

    // --- Coastline Biome (Far East) ---
    {
        FBiomeZone Coast;
        Coast.BiomeType = EBiomeType::Coastline;
        Coast.CenterLocation = FVector(6000.0f, 0.0f, 0.0f);
        Coast.Radius = 3000.0f;
        Coast.TemperatureBase = 26.0f;
        Coast.HumidityBase = 0.85f;
        Coast.DangerLevel = 0.3f;
        Coast.InhabitingSpecies.Add(EDinosaurSpecies::Pteranodon);
        BiomeZones.Add(Coast);
    }

    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Default Cretaceous biomes initialized: %d zones."), BiomeZones.Num());
}

// ============================================================
// Debug Visualization
// ============================================================

void ABiomeManager::DrawBiomeDebugSpheres()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World) return;

    static const TMap<EBiomeType, FColor> BiomeColors = {
        { EBiomeType::Forest,      FColor::Green },
        { EBiomeType::Savanna,     FColor::Yellow },
        { EBiomeType::Swampland,   FColor(0, 100, 50) },
        { EBiomeType::Mountains,   FColor::Silver },
        { EBiomeType::RiverValley, FColor::Blue },
        { EBiomeType::Coastline,   FColor::Cyan },
        { EBiomeType::Desert,      FColor::Orange },
    };

    for (const FBiomeZone& Zone : BiomeZones)
    {
        const FColor* Color = BiomeColors.Find(Zone.BiomeType);
        FColor DrawColor = Color ? *Color : FColor::White;

        DrawDebugSphere(World, Zone.CenterLocation, Zone.Radius, 32, DrawColor, false, -1.0f, 0, 5.0f);
        DrawDebugString(World, Zone.CenterLocation + FVector(0, 0, 200),
            FString::Printf(TEXT("Biome: %d\nDanger: %.1f\nTemp: %.1f°C"),
                (int32)Zone.BiomeType, Zone.DangerLevel, Zone.TemperatureBase),
            nullptr, DrawColor, -1.0f);
    }
#endif
}

// ============================================================
// Internal Helpers
// ============================================================

const FBiomeZone* ABiomeManager::FindNearestZone(const FVector& Location) const
{
    const FBiomeZone* Nearest = nullptr;
    float MinDist = FLT_MAX;

    for (const FBiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist(Location, Zone.CenterLocation);
        if (Dist < MinDist)
        {
            MinDist = Dist;
            Nearest = &Zone;
        }
    }

    return Nearest;
}

float ABiomeManager::GetBlendWeight(const FVector& Location, const FBiomeZone& Zone) const
{
    float Dist = FVector::Dist(Location, Zone.CenterLocation);
    if (Dist >= Zone.Radius + TransitionBlendRadius)
    {
        return 0.0f;
    }
    if (Dist <= Zone.Radius)
    {
        return 1.0f;
    }
    // Smooth blend in transition zone
    float T = 1.0f - ((Dist - Zone.Radius) / TransitionBlendRadius);
    return FMath::SmoothStep(0.0f, 1.0f, T);
}
