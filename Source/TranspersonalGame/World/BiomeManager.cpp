#include "BiomeManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// ABiomeManager — Implementation
// Biome classification via layered noise + altitude + moisture
// ============================================================

TWeakObjectPtr<ABiomeManager> ABiomeManager::InstanceCache = nullptr;

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    BiomeDataTable = nullptr;
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InstanceCache = this;
    InitializeBiomeDataMap();
}

void ABiomeManager::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    // Re-init in editor when properties change
    InitializeBiomeDataMap();
}

// ── Singleton ────────────────────────────────────────────────

ABiomeManager* ABiomeManager::GetInstance(UObject* WorldContextObject)
{
    if (InstanceCache.IsValid())
    {
        return InstanceCache.Get();
    }

    if (!WorldContextObject)
    {
        return nullptr;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(World, ABiomeManager::StaticClass(), Found);
    if (Found.Num() > 0)
    {
        ABiomeManager* Instance = Cast<ABiomeManager>(Found[0]);
        InstanceCache = Instance;
        return Instance;
    }

    return nullptr;
}

// ── Public API ───────────────────────────────────────────────

FEng_BiomeSample ABiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    FEng_BiomeSample Sample;

    // Primary biome noise
    float PrimaryNoise = EvaluateBiomeNoise(WorldLocation.X, WorldLocation.Y, 0);
    float MoistureNoise = EvaluateBiomeNoise(WorldLocation.X, WorldLocation.Y, 1000);
    float Altitude = FMath::Max(0.0f, WorldLocation.Z);

    Sample.PrimaryBiome = ClassifyBiome(PrimaryNoise, Altitude, MoistureNoise);

    // Secondary biome for blending (slightly offset sample)
    float SecondaryNoise = EvaluateBiomeNoise(WorldLocation.X + BiomeBlendRadius * 0.1f, WorldLocation.Y, 0);
    Sample.SecondaryBiome = ClassifyBiome(SecondaryNoise, Altitude, MoistureNoise);

    // Blend weight based on proximity to boundary
    float NoiseDelta = FMath::Abs(PrimaryNoise - SecondaryNoise);
    Sample.BlendWeight = FMath::Clamp(NoiseDelta * 5.0f, 0.0f, 1.0f);

    // Blend survival modifiers
    const FEng_BiomeData* PrimaryData = BiomeDataMap.Find(static_cast<uint8>(Sample.PrimaryBiome));
    const FEng_BiomeData* SecondaryData = BiomeDataMap.Find(static_cast<uint8>(Sample.SecondaryBiome));

    if (PrimaryData && SecondaryData && Sample.BlendWeight > 0.01f)
    {
        Sample.BlendedModifiers = BlendModifiers(
            PrimaryData->SurvivalModifiers,
            SecondaryData->SurvivalModifiers,
            Sample.BlendWeight);
    }
    else if (PrimaryData)
    {
        Sample.BlendedModifiers = PrimaryData->SurvivalModifiers;
    }

    return Sample;
}

EEng_BiomeType ABiomeManager::GetPrimaryBiomeType(FVector WorldLocation) const
{
    float Noise = EvaluateBiomeNoise(WorldLocation.X, WorldLocation.Y, 0);
    float Moisture = EvaluateBiomeNoise(WorldLocation.X, WorldLocation.Y, 1000);
    float Altitude = FMath::Max(0.0f, WorldLocation.Z);
    return ClassifyBiome(Noise, Altitude, Moisture);
}

FEng_BiomeSurvivalModifiers ABiomeManager::GetSurvivalModifiers(FVector WorldLocation) const
{
    FEng_BiomeSample Sample = GetBiomeAtLocation(WorldLocation);
    return Sample.BlendedModifiers;
}

void ABiomeManager::DrawBiomeBoundaries()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World) return;

    const float GridSize = 5000.0f;   // 50m grid
    const float WorldExtent = 200000.0f; // 2km radius
    const float DrawZ = 200.0f;

    for (float X = -WorldExtent; X <= WorldExtent; X += GridSize)
    {
        for (float Y = -WorldExtent; Y <= WorldExtent; Y += GridSize)
        {
            FVector Loc(X, Y, DrawZ);
            EEng_BiomeType Biome = GetPrimaryBiomeType(Loc);

            FColor DebugColor = FColor::White;
            const FEng_BiomeData* Data = BiomeDataMap.Find(static_cast<uint8>(Biome));
            if (Data)
            {
                DebugColor = Data->DebugColor.ToFColor(true);
            }

            DrawDebugPoint(World, Loc, 8.0f, DebugColor, false, 10.0f);
        }
    }
#endif
}

// ── Private ──────────────────────────────────────────────────

float ABiomeManager::EvaluateBiomeNoise(float X, float Y, int32 OctaveOffset) const
{
    // Layered FMath::PerlinNoise2D approximation using sin/cos harmonics
    // (UE5 doesn't expose Perlin directly in C++ without PCG module)
    float Freq1 = BiomeNoiseScale;
    float Freq2 = BiomeNoiseScale * 2.3f;
    float Freq3 = BiomeNoiseScale * 5.1f;

    float SeedOffset = static_cast<float>(BiomeSeed + OctaveOffset) * 0.1f;

    float N1 = FMath::Sin((X * Freq1 + SeedOffset) * PI) * FMath::Cos((Y * Freq1 + SeedOffset) * PI);
    float N2 = FMath::Sin((X * Freq2 + SeedOffset * 1.3f) * PI) * FMath::Cos((Y * Freq2 + SeedOffset * 1.7f) * PI);
    float N3 = FMath::Sin((X * Freq3 + SeedOffset * 2.1f) * PI) * FMath::Cos((Y * Freq3 + SeedOffset * 0.9f) * PI);

    float Combined = (N1 * 0.5f + N2 * 0.3f + N3 * 0.2f);
    return FMath::Clamp((Combined + 1.0f) * 0.5f, 0.0f, 1.0f); // Normalize to [0,1]
}

EEng_BiomeType ABiomeManager::ClassifyBiome(float NoiseValue, float Altitude, float Moisture) const
{
    // High altitude → Volcanic or Tundra
    if (Altitude > 30000.0f)
    {
        return (NoiseValue > 0.6f) ? EEng_BiomeType::Volcanic : EEng_BiomeType::Tundra;
    }

    // Low altitude coastal zone
    if (Altitude < 500.0f)
    {
        if (Moisture > 0.7f) return EEng_BiomeType::Swamp;
        return EEng_BiomeType::Coastal;
    }

    // Mid altitude — noise + moisture classification
    if (Moisture > 0.65f)
    {
        if (NoiseValue > 0.55f) return EEng_BiomeType::Jungle;
        return EEng_BiomeType::Forest;
    }

    if (Moisture < 0.3f)
    {
        if (NoiseValue > 0.5f) return EEng_BiomeType::Desert;
        return EEng_BiomeType::Savanna;
    }

    // River corridors — low noise value in mid moisture
    if (NoiseValue < 0.2f && Moisture > 0.4f)
    {
        return EEng_BiomeType::River;
    }

    // Default mid-range
    if (NoiseValue > 0.5f) return EEng_BiomeType::Savanna;
    return EEng_BiomeType::Forest;
}

FEng_BiomeSurvivalModifiers ABiomeManager::BlendModifiers(
    const FEng_BiomeSurvivalModifiers& A,
    const FEng_BiomeSurvivalModifiers& B,
    float W) const
{
    FEng_BiomeSurvivalModifiers Out;
    float InvW = 1.0f - W;

    Out.TemperatureOffset       = A.TemperatureOffset       * InvW + B.TemperatureOffset       * W;
    Out.ThirstDrainMultiplier   = A.ThirstDrainMultiplier   * InvW + B.ThirstDrainMultiplier   * W;
    Out.HungerDrainMultiplier   = A.HungerDrainMultiplier   * InvW + B.HungerDrainMultiplier   * W;
    Out.StaminaRecoveryMultiplier = A.StaminaRecoveryMultiplier * InvW + B.StaminaRecoveryMultiplier * W;
    Out.VisibilityRange         = A.VisibilityRange         * InvW + B.VisibilityRange         * W;
    Out.DangerLevel             = A.DangerLevel             * InvW + B.DangerLevel             * W;

    return Out;
}

void ABiomeManager::InitializeBiomeDataMap()
{
    BiomeDataMap.Empty();

    if (BiomeDataTable)
    {
        // Load from DataTable if assigned
        TArray<FEng_BiomeData*> Rows;
        BiomeDataTable->GetAllRows<FEng_BiomeData>(TEXT("BiomeManager"), Rows);
        for (FEng_BiomeData* Row : Rows)
        {
            if (Row)
            {
                BiomeDataMap.Add(static_cast<uint8>(Row->BiomeType), *Row);
            }
        }
        return;
    }

    // ── Default biome data (no DataTable required) ──────────

    auto AddBiome = [&](EEng_BiomeType Type, FName Name, FLinearColor Color,
        float TempOffset, float ThirstMult, float HungerMult,
        float StaminaMult, float Visibility, float Danger)
    {
        FEng_BiomeData Data;
        Data.BiomeType = Type;
        Data.BiomeName = Name;
        Data.DebugColor = Color;
        Data.SurvivalModifiers.TemperatureOffset = TempOffset;
        Data.SurvivalModifiers.ThirstDrainMultiplier = ThirstMult;
        Data.SurvivalModifiers.HungerDrainMultiplier = HungerMult;
        Data.SurvivalModifiers.StaminaRecoveryMultiplier = StaminaMult;
        Data.SurvivalModifiers.VisibilityRange = Visibility;
        Data.SurvivalModifiers.DangerLevel = Danger;
        BiomeDataMap.Add(static_cast<uint8>(Type), Data);
    };

    //                Type                    Name          Color                         Temp  Thirst Hunger Stam   Vis      Danger
    AddBiome(EEng_BiomeType::Jungle,   "Jungle",   FLinearColor(0.0f, 0.6f, 0.1f, 1.0f),  +8.0f, 1.4f,  1.2f,  0.8f,  3000.0f, 0.75f);
    AddBiome(EEng_BiomeType::Savanna,  "Savanna",  FLinearColor(0.8f, 0.7f, 0.1f, 1.0f),  +12.0f,1.6f,  1.0f,  1.0f,  8000.0f, 0.60f);
    AddBiome(EEng_BiomeType::Swamp,    "Swamp",    FLinearColor(0.2f, 0.4f, 0.1f, 1.0f),  +5.0f, 1.0f,  1.3f,  0.7f,  2000.0f, 0.80f);
    AddBiome(EEng_BiomeType::Volcanic, "Volcanic", FLinearColor(0.9f, 0.2f, 0.0f, 1.0f),  +25.0f,1.8f,  1.1f,  0.6f,  4000.0f, 0.95f);
    AddBiome(EEng_BiomeType::Coastal,  "Coastal",  FLinearColor(0.2f, 0.6f, 0.9f, 1.0f),  +2.0f, 0.8f,  0.9f,  1.1f,  9000.0f, 0.40f);
    AddBiome(EEng_BiomeType::Forest,   "Forest",   FLinearColor(0.1f, 0.5f, 0.1f, 1.0f),  0.0f,  1.0f,  1.0f,  1.0f,  5000.0f, 0.55f);
    AddBiome(EEng_BiomeType::Desert,   "Desert",   FLinearColor(0.9f, 0.8f, 0.4f, 1.0f),  +18.0f,2.0f,  0.8f,  0.7f,  12000.0f,0.65f);
    AddBiome(EEng_BiomeType::Tundra,   "Tundra",   FLinearColor(0.8f, 0.9f, 1.0f, 1.0f),  -15.0f,0.7f,  1.4f,  0.9f,  7000.0f, 0.50f);
    AddBiome(EEng_BiomeType::River,    "River",    FLinearColor(0.1f, 0.4f, 0.8f, 1.0f),  0.0f,  0.5f,  0.9f,  1.2f,  6000.0f, 0.35f);
}
