#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// Biome type enumeration — prehistoric survival world biomes
// ============================================================
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Jungle          UMETA(DisplayName = "Cretaceous Jungle"),
    Savanna         UMETA(DisplayName = "Open Savanna"),
    Swamp           UMETA(DisplayName = "Prehistoric Swamp"),
    Volcanic        UMETA(DisplayName = "Volcanic Highlands"),
    Coastal         UMETA(DisplayName = "Coastal Shallows"),
    Forest          UMETA(DisplayName = "Conifer Forest"),
    Desert          UMETA(DisplayName = "Arid Badlands"),
    River           UMETA(DisplayName = "River Delta"),
    COUNT           UMETA(Hidden)
};

// ============================================================
// Biome definition data — parameters for world generation
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString DisplayName = TEXT("Unknown Biome");

    /** Temperature in Celsius — affects survival mechanics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival", meta = (ClampMin = "-20.0", ClampMax = "60.0"))
    float TemperatureCelsius = 28.0f;

    /** Humidity 0-1 — affects thirst drain rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.8f;

    /** Foliage density 0-1 — drives PCG tree placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FoliageDensity = 0.7f;

    /** Predator spawn weight — higher = more dangerous */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Dinosaurs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PredatorSpawnWeight = 0.3f;

    /** Herbivore spawn weight */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Dinosaurs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HerbivoreSpawnWeight = 0.6f;

    /** Fog density for atmosphere */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Atmosphere", meta = (ClampMin = "0.0", ClampMax = "0.1"))
    float FogDensity = 0.02f;

    /** Sky color tint for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Atmosphere")
    FLinearColor SkyColorTint = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    /** Ambient sound cue path for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    FSoftObjectPath AmbientSoundPath;

    /** Resource richness 0-1 — food/water/crafting materials */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Resources", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ResourceRichness = 0.5f;

    /** World-space bounds center for this biome zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    FVector WorldCenter = FVector::ZeroVector;

    /** Biome radius in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World", meta = (ClampMin = "1000.0"))
    float WorldRadius = 50000.0f;
};

// ============================================================
// Biome transition data — smooth blending between biomes
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType FromBiome = EEng_BiomeType::Jungle;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType ToBiome = EEng_BiomeType::Savanna;

    /** 0 = fully in FromBiome, 1 = fully in ToBiome */
    UPROPERTY(BlueprintReadOnly, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendAlpha = 0.0f;

    /** Distance to nearest biome boundary */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float DistanceToBoundary = 0.0f;
};

// ============================================================
// ABiomeManager — world actor that manages all biome zones
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ---- Biome Query API ----

    /** Get the dominant biome type at a world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get full biome definition at a world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeDefinition GetBiomeDefinitionAtLocation(const FVector& WorldLocation) const;

    /** Get transition blend data between two biomes at a location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeTransition GetBiomeTransitionAtLocation(const FVector& WorldLocation) const;

    /** Get temperature at location (accounts for biome blending) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Survival")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Get humidity at location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Survival")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    /** Get predator danger level at location (0-1) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Dinosaurs")
    float GetPredatorDangerAtLocation(const FVector& WorldLocation) const;

    /** Get foliage density at location (0-1) */
    UFUNCTION(BlueprintCallable, Category = "Biome|World")
    float GetFoliageDensityAtLocation(const FVector& WorldLocation) const;

    /** Register a new biome zone at runtime */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(const FEng_BiomeDefinition& BiomeDef);

    /** Get all registered biome definitions */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FEng_BiomeDefinition> GetAllBiomes() const { return BiomeDefinitions; }

    /** Get count of registered biomes */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetBiomeCount() const { return BiomeDefinitions.Num(); }

    // ---- Configuration ----

    /** All biome zones in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Configuration")
    TArray<FEng_BiomeDefinition> BiomeDefinitions;

    /** Transition blend width in cm — how wide the biome borders are */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Configuration", meta = (ClampMin = "500.0"))
    float TransitionBlendWidth = 5000.0f;

    /** Whether to use noise-based biome boundaries (more organic) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Configuration")
    bool bUseNoiseBoundaries = true;

    /** Noise scale for boundary variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Configuration", meta = (ClampMin = "0.001"))
    float BoundaryNoiseScale = 0.0001f;

    /** Debug: draw biome boundaries in viewport */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDrawBiomeBoundaries = false;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Initialize default biome definitions for the prehistoric world */
    void InitializeDefaultBiomes();

    /** Find the nearest biome definition to a world location */
    int32 FindNearestBiomeIndex(const FVector& WorldLocation) const;

    /** Find the two nearest biomes for blending */
    void FindTwoNearestBiomes(const FVector& WorldLocation, int32& OutPrimaryIdx, int32& OutSecondaryIdx, float& OutBlendAlpha) const;

    /** Simple noise function for boundary variation */
    float SampleNoise(float X, float Y) const;

private:
    /** Cached noise seed */
    int32 NoiseSeed = 42;
};
