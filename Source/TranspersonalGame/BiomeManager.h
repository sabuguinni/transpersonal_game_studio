#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

/**
 * FBiomeData — Per-biome configuration data.
 * Defines ecological parameters for each biome type in the Cretaceous world.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString DisplayName = TEXT("Dense Forest");

    /** Base temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float BaseTemperature = 28.0f;

    /** Humidity 0-1 (affects thirst drain rate) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float Humidity = 0.7f;

    /** Fog density multiplier for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensityMultiplier = 1.0f;

    /** Vegetation density 0-1 (used by FoliageManager) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Ecology")
    float VegetationDensity = 0.8f;

    /** Predator spawn weight (higher = more predators) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Ecology")
    float PredatorSpawnWeight = 0.3f;

    /** Herbivore spawn weight */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Ecology")
    float HerbivoreSpawnWeight = 0.7f;

    /** Dominant dinosaur species in this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Ecology")
    TArray<EDinosaurSpecies> DominantSpecies;

    /** Ambient colour tint for sky/fog in this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor AmbientTint = FLinearColor(0.4f, 0.6f, 0.3f, 1.0f);

    /** Water source availability 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Resources")
    float WaterAvailability = 0.6f;

    /** Food resource density 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Resources")
    float FoodAvailability = 0.5f;

    FBiomeData() {}
};

/**
 * FBiomeTransition — Defines a blend zone between two biomes.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType BiomeA = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType BiomeB = EBiomeType::Savanna;

    /** World-space centre of the transition zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FVector TransitionCentre = FVector::ZeroVector;

    /** Radius of the blend zone in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendRadius = 5000.0f;

    /** 0=fully BiomeA, 1=fully BiomeB */
    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    float CurrentBlendAlpha = 0.0f;

    FBiomeTransition() {}
};

/**
 * ABiomeManager — World actor that manages Cretaceous biome data,
 * queries the active biome at any world position, and drives
 * ecological parameters for FoliageManager and DinosaurAI systems.
 *
 * Placed once in the level. Other systems call GetBiomeAtLocation().
 */
UCLASS(ClassGroup = "TranspersonalGame", meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Biome Query API ──────────────────────────────────────────────

    /** Returns the biome type at the given world location. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full biome data struct for a given biome type. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FBiomeData GetBiomeData(EBiomeType BiomeType) const;

    /** Returns blended biome data at a location (handles transition zones). */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FBiomeData GetBlendedBiomeData(const FVector& WorldLocation) const;

    /** Returns temperature at a world location (biome base + altitude modifier). */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Returns humidity at a world location. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    /** Returns vegetation density at a world location (used by FoliageManager). */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    /** Returns predator spawn weight at a location. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetPredatorWeightAtLocation(const FVector& WorldLocation) const;

    /** Returns true if the location is inside a biome transition zone. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsInTransitionZone(const FVector& WorldLocation, FBiomeTransition& OutTransition) const;

    // ── Biome Registration ───────────────────────────────────────────

    /** Register a biome zone (called by PCGWorldGenerator during world gen). */
    UFUNCTION(BlueprintCallable, Category = "Biome|Setup")
    void RegisterBiomeZone(EBiomeType BiomeType, const FVector& Centre, float Radius);

    /** Register a transition zone between two biomes. */
    UFUNCTION(BlueprintCallable, Category = "Biome|Setup")
    void RegisterTransitionZone(EBiomeType A, EBiomeType B, const FVector& Centre, float BlendRadius);

    /** Initialise default Cretaceous biome data for all biome types. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Setup")
    void InitialiseDefaultBiomes();

    // ── Singleton Access ─────────────────────────────────────────────

    /** Get the BiomeManager instance from the world. */
    UFUNCTION(BlueprintCallable, Category = "Biome", meta = (WorldContext = "WorldContextObject"))
    static ABiomeManager* GetInstance(const UObject* WorldContextObject);

    // ── Properties ───────────────────────────────────────────────────

    /** All registered biome data, keyed by biome type. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Data")
    TMap<EBiomeType, FBiomeData> BiomeDataMap;

    /** Registered biome zones: centre + radius pairs per type. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Zones")
    TArray<FVector> BiomeZoneCentres;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Zones")
    TArray<float> BiomeZoneRadii;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Zones")
    TArray<EBiomeType> BiomeZoneTypes;

    /** Registered transition zones. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Transitions")
    TArray<FBiomeTransition> TransitionZones;

    /** Altitude above which temperature drops (cm). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float AltitudeTemperatureDropThreshold = 5000.0f;

    /** Temperature drop per 1000cm above threshold. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float TemperatureDropPerKiloCm = 2.0f;

    /** Whether to draw debug spheres for biome zones in editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDrawDebugZones = false;

private:
    /** Finds the nearest biome zone to a world location. */
    int32 FindNearestBiomeZoneIndex(const FVector& WorldLocation) const;

    /** Blend two FBiomeData structs by alpha (0=A, 1=B). */
    FBiomeData BlendBiomeData(const FBiomeData& A, const FBiomeData& B, float Alpha) const;

    /** Static singleton reference. */
    static ABiomeManager* Instance;
};
