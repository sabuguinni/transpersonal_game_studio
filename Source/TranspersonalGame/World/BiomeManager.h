#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// Biome system — P1 World Generation
// Defines biome classification, transition blending, and
// per-biome survival modifiers for the prehistoric world.
// ============================================================

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    None            UMETA(DisplayName = "None"),
    Jungle          UMETA(DisplayName = "Jungle"),
    Savanna         UMETA(DisplayName = "Savanna"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    Coastal         UMETA(DisplayName = "Coastal"),
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert"),
    Tundra          UMETA(DisplayName = "Tundra"),
    River           UMETA(DisplayName = "River"),
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeSurvivalModifiers
{
    GENERATED_BODY()

    // Temperature offset in Celsius relative to world baseline
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float TemperatureOffset = 0.0f;

    // Multiplier on player thirst drain (1.0 = normal)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float ThirstDrainMultiplier = 1.0f;

    // Multiplier on player hunger drain
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float HungerDrainMultiplier = 1.0f;

    // Multiplier on stamina recovery speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float StaminaRecoveryMultiplier = 1.0f;

    // Visibility range modifier (fog/canopy density)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float VisibilityRange = 5000.0f;

    // Ambient danger level 0-1 (affects NPC spawn rates)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float DangerLevel = 0.5f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FName BiomeName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor DebugColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    FEng_BiomeSurvivalModifiers SurvivalModifiers;

    // Minimum/maximum altitude for this biome (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float MinAltitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float MaxAltitude = 50000.0f;

    // Rainfall 0-1 (drives vegetation density)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float Rainfall = 0.5f;

    // Primary dinosaur species that spawn in this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Fauna")
    TArray<FName> DinosaurSpecies;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeSample
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType PrimaryBiome = EEng_BiomeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType SecondaryBiome = EEng_BiomeType::None;

    // 0-1 blend weight between primary and secondary
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendWeight = 0.0f;

    // Blended survival modifiers
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FEng_BiomeSurvivalModifiers BlendedModifiers;
};

/**
 * ABiomeManager
 *
 * World-space biome classification and query system.
 * Placed once in the level. Other systems query it via
 * GetBiomeAtLocation() to get blended biome data.
 *
 * Architecture: Singleton actor pattern — one per world.
 * Biome boundaries driven by noise + altitude + moisture.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|World")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ── Configuration ──────────────────────────────────────────

    /** DataTable of FEng_BiomeData rows, keyed by biome name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    UDataTable* BiomeDataTable;

    /** World scale for the noise-based biome map (larger = bigger biomes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeNoiseScale = 0.00015f;

    /** Blend radius at biome edges (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeBlendRadius = 50000.0f;

    /** Random seed for biome placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    int32 BiomeSeed = 42;

    // ── Runtime State ──────────────────────────────────────────

    /** Cached biome data map, populated from DataTable at BeginPlay */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Runtime")
    TMap<uint8, FEng_BiomeData> BiomeDataMap;

    // ── Public API ─────────────────────────────────────────────

    /**
     * Query the biome at a world-space location.
     * Returns blended sample if on a biome boundary.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeSample GetBiomeAtLocation(FVector WorldLocation) const;

    /**
     * Get the primary biome type at a location (fast path, no blending).
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetPrimaryBiomeType(FVector WorldLocation) const;

    /**
     * Get blended survival modifiers for a world location.
     * Used by the character survival system every tick.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeSurvivalModifiers GetSurvivalModifiers(FVector WorldLocation) const;

    /**
     * Debug: draw biome boundaries in the viewport.
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DrawBiomeBoundaries();

    /**
     * Get the singleton BiomeManager from the world.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome", meta = (WorldContext = "WorldContextObject"))
    static ABiomeManager* GetInstance(UObject* WorldContextObject);

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    /** Evaluate raw biome noise at a 2D position, returns 0-1 */
    float EvaluateBiomeNoise(float X, float Y, int32 OctaveOffset) const;

    /** Map noise value + altitude to a biome type */
    EEng_BiomeType ClassifyBiome(float NoiseValue, float Altitude, float Moisture) const;

    /** Blend two BiomeData structs by weight */
    FEng_BiomeSurvivalModifiers BlendModifiers(
        const FEng_BiomeSurvivalModifiers& A,
        const FEng_BiomeSurvivalModifiers& B,
        float BlendWeight) const;

    /** Load BiomeDataTable rows into BiomeDataMap */
    void InitializeBiomeDataMap();

    /** Cached singleton reference */
    static TWeakObjectPtr<ABiomeManager> InstanceCache;
};
