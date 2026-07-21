#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ExponentialHeightFogComponent.h"

// ============================================================
// Constructor
// ============================================================
ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Tick every 0.5s for performance

    // Initialize default biomes in constructor so CDO has valid data
    InitializeDefaultBiomes();
}

// ============================================================
// BeginPlay
// ============================================================
void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    if (BiomeConfigs.Num() == 0)
    {
        InitializeDefaultBiomes();
    }

    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Initialized with %d biome zones"), BiomeConfigs.Num());
}

// ============================================================
// Tick
// ============================================================
void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate < UpdateInterval)
    {
        return;
    }
    TimeSinceLastUpdate = 0.0f;

    // Get player location and update biome state
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();
    EEng_BiomeType NewBiome = GetBiomeAtLocation(PlayerLoc);

    if (NewBiome != CurrentPlayerBiome)
    {
        CurrentPlayerBiome = NewBiome;
        UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Player entered biome: %d"), (int32)NewBiome);

        if (bEnableVisualTransitions)
        {
            const FEng_BiomeConfig* Config = FindNearestBiome(PlayerLoc);
            if (Config)
            {
                ApplyBiomeVisuals(*Config);
            }
        }
    }
}

// ============================================================
// InitializeDefaultBiomes — 5 prehistoric biome zones
// ============================================================
void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeConfigs.Empty();

    // --- JUNGLE ---
    FEng_BiomeConfig Jungle;
    Jungle.BiomeType = EEng_BiomeType::Jungle;
    Jungle.DisplayName = TEXT("Dense Jungle");
    Jungle.Center = FVector(200000.0f, 200000.0f, 0.0f);
    Jungle.Radius = 250000.0f;
    Jungle.AmbientTemperature = 32.0f;
    Jungle.WaterAvailability = 0.9f;
    Jungle.FoodAvailability = 0.8f;
    Jungle.DangerLevel = 0.6f;
    Jungle.FogDensity = 0.04f;
    Jungle.SkyTint = FLinearColor(0.3f, 0.6f, 0.2f, 1.0f);
    BiomeConfigs.Add(Jungle);

    // --- SAVANNA ---
    FEng_BiomeConfig Savanna;
    Savanna.BiomeType = EEng_BiomeType::Savanna;
    Savanna.DisplayName = TEXT("Open Savanna");
    Savanna.Center = FVector(-200000.0f, 200000.0f, 0.0f);
    Savanna.Radius = 300000.0f;
    Savanna.AmbientTemperature = 38.0f;
    Savanna.WaterAvailability = 0.2f;
    Savanna.FoodAvailability = 0.4f;
    Savanna.DangerLevel = 0.7f; // High — open ground, T-Rex territory
    Savanna.FogDensity = 0.01f;
    Savanna.SkyTint = FLinearColor(0.9f, 0.7f, 0.3f, 1.0f);
    BiomeConfigs.Add(Savanna);

    // --- VOLCANIC ---
    FEng_BiomeConfig Volcanic;
    Volcanic.BiomeType = EEng_BiomeType::Volcanic;
    Volcanic.DisplayName = TEXT("Volcanic Highlands");
    Volcanic.Center = FVector(200000.0f, -200000.0f, 0.0f);
    Volcanic.Radius = 200000.0f;
    Volcanic.AmbientTemperature = 55.0f; // Extreme heat
    Volcanic.WaterAvailability = 0.05f;
    Volcanic.FoodAvailability = 0.1f;
    Volcanic.DangerLevel = 0.9f; // Extremely dangerous
    Volcanic.FogDensity = 0.06f; // Ash/smoke
    Volcanic.SkyTint = FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);
    BiomeConfigs.Add(Volcanic);

    // --- RIVER DELTA ---
    FEng_BiomeConfig River;
    River.BiomeType = EEng_BiomeType::River;
    River.DisplayName = TEXT("River Delta");
    River.Center = FVector(-200000.0f, -200000.0f, 0.0f);
    River.Radius = 180000.0f;
    River.AmbientTemperature = 26.0f;
    River.WaterAvailability = 1.0f; // Maximum water
    River.FoodAvailability = 0.7f; // Fish, plants
    River.DangerLevel = 0.4f;
    River.FogDensity = 0.03f;
    River.SkyTint = FLinearColor(0.4f, 0.5f, 0.9f, 1.0f);
    BiomeConfigs.Add(River);

    // --- OPEN PLAINS (starting area) ---
    FEng_BiomeConfig Plains;
    Plains.BiomeType = EEng_BiomeType::Plains;
    Plains.DisplayName = TEXT("Open Plains");
    Plains.Center = FVector(0.0f, 0.0f, 0.0f); // Player start area
    Plains.Radius = 150000.0f;
    Plains.AmbientTemperature = 24.0f;
    Plains.WaterAvailability = 0.4f;
    Plains.FoodAvailability = 0.5f;
    Plains.DangerLevel = 0.3f; // Safer starting zone
    Plains.FogDensity = 0.02f;
    Plains.SkyTint = FLinearColor(0.6f, 0.7f, 1.0f, 1.0f);
    BiomeConfigs.Add(Plains);

    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Default biomes initialized: %d zones"), BiomeConfigs.Num());
}

// ============================================================
// RegisterBiome
// ============================================================
void ABiomeManager::RegisterBiome(const FEng_BiomeConfig& Config)
{
    // Remove existing biome of same type if present
    BiomeConfigs.RemoveAll([&Config](const FEng_BiomeConfig& Existing)
    {
        return Existing.BiomeType == Config.BiomeType;
    });
    BiomeConfigs.Add(Config);
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Registered biome: %s"), *Config.DisplayName);
}

// ============================================================
// GetBiomeAtLocation
// ============================================================
EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    const FEng_BiomeConfig* Nearest = FindNearestBiome(WorldLocation);
    if (Nearest)
    {
        return Nearest->BiomeType;
    }
    return EEng_BiomeType::Plains; // Default fallback
}

// ============================================================
// GetBiomeConfigAtLocation
// ============================================================
FEng_BiomeConfig ABiomeManager::GetBiomeConfigAtLocation(const FVector& WorldLocation) const
{
    const FEng_BiomeConfig* Nearest = FindNearestBiome(WorldLocation);
    if (Nearest)
    {
        return *Nearest;
    }
    // Return default Plains config
    FEng_BiomeConfig Default;
    Default.BiomeType = EEng_BiomeType::Plains;
    Default.DisplayName = TEXT("Open Plains");
    return Default;
}

// ============================================================
// GetTemperatureAtLocation
// ============================================================
float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    if (BiomeConfigs.Num() == 0) return 25.0f;

    // Blend temperature from nearby biomes (weighted by inverse distance)
    float TotalWeight = 0.0f;
    float BlendedTemp = 0.0f;

    for (const FEng_BiomeConfig& Config : BiomeConfigs)
    {
        float Dist = FVector::Dist(WorldLocation, Config.Center);
        if (Dist < Config.Radius)
        {
            float Weight = 1.0f - (Dist / Config.Radius);
            Weight = Weight * Weight; // Quadratic falloff
            BlendedTemp += Config.AmbientTemperature * Weight;
            TotalWeight += Weight;
        }
    }

    if (TotalWeight > 0.0f)
    {
        return BlendedTemp / TotalWeight;
    }

    // Outside all biomes — return nearest biome temperature
    const FEng_BiomeConfig* Nearest = FindNearestBiome(WorldLocation);
    return Nearest ? Nearest->AmbientTemperature : 25.0f;
}

// ============================================================
// GetWaterAvailabilityAtLocation
// ============================================================
float ABiomeManager::GetWaterAvailabilityAtLocation(const FVector& WorldLocation) const
{
    const FEng_BiomeConfig* Nearest = FindNearestBiome(WorldLocation);
    return Nearest ? Nearest->WaterAvailability : 0.3f;
}

// ============================================================
// GetDangerLevelAtLocation
// ============================================================
float ABiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    const FEng_BiomeConfig* Nearest = FindNearestBiome(WorldLocation);
    return Nearest ? Nearest->DangerLevel : 0.3f;
}

// ============================================================
// FindNearestBiome (private)
// ============================================================
const FEng_BiomeConfig* ABiomeManager::FindNearestBiome(const FVector& WorldLocation) const
{
    const FEng_BiomeConfig* Best = nullptr;
    float BestDist = FLT_MAX;

    for (const FEng_BiomeConfig& Config : BiomeConfigs)
    {
        float Dist = FVector::Dist(WorldLocation, Config.Center);
        if (Dist < BestDist)
        {
            BestDist = Dist;
            Best = &Config;
        }
    }

    return Best;
}

// ============================================================
// ApplyBiomeVisuals (private)
// ============================================================
void ABiomeManager::ApplyBiomeVisuals(const FEng_BiomeConfig& Config)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find ExponentialHeightFog and update density
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);

    for (AActor* FogActor : FogActors)
    {
        AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(FogActor);
        if (Fog)
        {
            UExponentialHeightFogComponent* FogComp = Fog->GetComponent();
            if (FogComp)
            {
                FogComp->SetFogDensity(Config.FogDensity);
                FogComp->SetFogInscatteringColor(Config.SkyTint);
                UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Applied visuals for biome: %s (fog=%.3f)"),
                    *Config.DisplayName, Config.FogDensity);
            }
            break; // Only update first fog actor
        }
    }
}
