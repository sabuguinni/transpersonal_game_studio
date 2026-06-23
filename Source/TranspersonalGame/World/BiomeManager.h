#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// BiomeManager.h — P1 World Generation
// Manages biome assignment, transitions, and per-biome
// environmental parameters for the prehistoric survival world.
// All types prefixed Eng_ per architecture rules.
// ============================================================

// Per-biome environmental parameters (data-driven via DataTable)
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeParameters
{
    GENERATED_BODY()

    // Display name shown in debug HUD
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FName BiomeName;

    // Base temperature in Celsius (Cretaceous: 20-35°C typical)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float BaseTemperatureCelsius = 28.0f;

    // Humidity 0-1 (affects fog density, plant density)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float Humidity = 0.6f;

    // Rainfall mm/year — drives vegetation density
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float AnnualRainfallMM = 1200.0f;

    // Fog density multiplier (1.0 = default)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensityMultiplier = 1.0f;

    // Sky tint colour for this biome's atmosphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor SkyTint = FLinearColor(0.53f, 0.81f, 0.98f, 1.0f);

    // Ground albedo colour
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor GroundColour = FLinearColor(0.35f, 0.28f, 0.15f, 1.0f);

    // Max foliage density (trees/10000m²)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Ecology")
    int32 MaxFoliageDensity = 80;

    // Dinosaur species that spawn in this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Ecology")
    TArray<EDinosaurSpecies> NativeDinosaurSpecies;

    // Water body coverage 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float WaterCoverage = 0.1f;

    // Terrain roughness 0-1 (0=flat plains, 1=jagged mountains)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float TerrainRoughness = 0.3f;
};

// Biome transition blend state (used during runtime blending)
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransitionState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    EBiomeType FromBiome = EBiomeType::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    EBiomeType ToBiome = EBiomeType::Grassland;

    // 0.0 = fully FromBiome, 1.0 = fully ToBiome
    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    float BlendAlpha = 0.0f;

    // Transition zone radius in cm
    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    float TransitionRadiusCM = 50000.0f;
};

// ============================================================
// ABiomeManager — world actor, one instance per level
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ---- Biome Query API ----

    // Returns the biome type at a given world XY position
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Returns full parameters for a biome type
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeParameters GetBiomeParameters(EBiomeType BiomeType) const;

    // Returns blended parameters when in a transition zone
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeParameters GetBlendedBiomeParameters(const FVector& WorldLocation) const;

    // Returns transition state for a location (alpha=0 if not in transition)
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeTransitionState GetTransitionState(const FVector& WorldLocation) const;

    // ---- Runtime State ----

    // Current biome the player is in (updated each tick)
    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    EBiomeType PlayerCurrentBiome = EBiomeType::Forest;

    // Active weather type (driven by biome + time of day)
    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    // Time of day 0-24 (drives lighting and dinosaur activity)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Time")
    float TimeOfDayHours = 12.0f;

    // Day length in real seconds (default 20 min = 1200s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Time")
    float DayLengthSeconds = 1200.0f;

    // ---- Configuration ----

    // World size in cm (square world, default 4km x 4km)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    float WorldSizeCM = 400000.0f;

    // Noise scale for biome boundary generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    float BiomeNoiseScale = 0.00005f;

    // Seed for deterministic biome layout
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    int32 WorldSeed = 42;

    // Enable biome transition blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    bool bEnableBiomeBlending = true;

    // ---- Debug ----

    // Draw biome debug overlay in viewport
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DrawBiomeDebugOverlay();

    // Print current biome stats to output log
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void PrintBiomeStats();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // Internal biome parameter table (populated in constructor)
    TMap<EBiomeType, FEng_BiomeParameters> BiomeParameterTable;

    // Initialise default parameters for all biome types
    void InitialiseBiomeParameters();

    // Simple noise-based biome determination (no external dependency)
    EBiomeType CalculateBiomeFromNoise(float NX, float NY) const;

    // Advance time of day
    void TickTimeOfDay(float DeltaTime);

    // Update weather based on biome + time
    void UpdateWeather();

    // Cached player pawn reference
    UPROPERTY()
    APawn* CachedPlayerPawn = nullptr;
};
