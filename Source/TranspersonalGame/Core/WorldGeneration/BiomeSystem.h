// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Classes/LandscapeProxy.h"
#include "BiomeSystem.generated.h"

/**
 * @brief Biome System for Transpersonal Game Studio
 * 
 * Defines biomes for the prehistoric world with specific characteristics:
 * - Dense Jungle: High vegetation density, tall trees, limited visibility
 * - River Valley: Water features, fertile soil, diverse wildlife
 * - Rocky Highlands: Sparse vegetation, elevated terrain, caves
 * - Coastal Plains: Beach areas, palm trees, ocean access
 * - Volcanic Regions: Dangerous terrain, unique minerals, hot springs
 * 
 * Each biome affects:
 * - Vegetation distribution and types
 * - Wildlife spawning patterns
 * - Resource availability
 * - Environmental hazards
 * - Weather patterns
 * 
 * @author Procedural World Generator — Agent #5
 * @version 1.0 — March 2026
 */

/** Biome types for the prehistoric world */
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    DenseJungle     UMETA(DisplayName = "Dense Jungle"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    CoastalPlains   UMETA(DisplayName = "Coastal Plains"),
    VolcanicRegion  UMETA(DisplayName = "Volcanic Region"),
    Swampland       UMETA(DisplayName = "Swampland"),
    Grasslands      UMETA(DisplayName = "Grasslands"),
    CaveSystem      UMETA(DisplayName = "Cave System")
};

/** Vegetation density levels */
UENUM(BlueprintType)
enum class EVegetationDensity : uint8
{
    None        UMETA(DisplayName = "No Vegetation"),
    Sparse      UMETA(DisplayName = "Sparse"),
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dense       UMETA(DisplayName = "Dense"),
    VeryDense   UMETA(DisplayName = "Very Dense")
};

/** Wildlife spawn frequency */
UENUM(BlueprintType)
enum class EWildlifeFrequency : uint8
{
    None        UMETA(DisplayName = "No Wildlife"),
    Rare        UMETA(DisplayName = "Rare"),
    Uncommon    UMETA(DisplayName = "Uncommon"),
    Common      UMETA(DisplayName = "Common"),
    Abundant    UMETA(DisplayName = "Abundant"),
    Overrun     UMETA(DisplayName = "Overrun")
};

/** Resource availability levels */
UENUM(BlueprintType)
enum class EResourceAvailability : uint8
{
    None        UMETA(DisplayName = "No Resources"),
    Scarce      UMETA(DisplayName = "Scarce"),
    Limited     UMETA(DisplayName = "Limited"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Rich        UMETA(DisplayName = "Rich"),
    Abundant    UMETA(DisplayName = "Abundant")
};

/** Vegetation asset data for biomes */
USTRUCT(BlueprintType)
struct FVegetationAsset
{
    GENERATED_BODY()

    /** Static mesh for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> VegetationMesh;

    /** Material override for this vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TSoftObjectPtr<UMaterialInterface> VegetationMaterial;

    /** Spawn weight (higher = more likely to spawn) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float SpawnWeight = 1.0f;

    /** Minimum scale multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float MinScale = 0.8f;

    /** Maximum scale multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float MaxScale = 1.2f;

    /** Minimum spacing between instances (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation", meta = (ClampMin = "50.0", ClampMax = "2000.0"))
    float MinSpacing = 200.0f;

    /** Can this vegetation be destroyed by dinosaurs? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    bool bCanBeDestroyed = true;

    /** Provides food for herbivores? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    bool bProvidesFood = false;

    /** Provides shelter/hiding spots? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    bool bProvidesShelter = false;
};

/** Resource spawn data for biomes */
USTRUCT(BlueprintType)
struct FResourceSpawnData
{
    GENERATED_BODY()

    /** Resource type identifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    FString ResourceType;

    /** Static mesh for this resource */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    TSoftObjectPtr<UStaticMesh> ResourceMesh;

    /** Spawn probability (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 0.1f;

    /** Minimum instances per km² */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = "0", ClampMax = "1000"))
    int32 MinInstancesPerKm2 = 5;

    /** Maximum instances per km² */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = "0", ClampMax = "1000"))
    int32 MaxInstancesPerKm2 = 20;

    /** Resource quality/value multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float QualityMultiplier = 1.0f;
};

/** Environmental hazard data */
USTRUCT(BlueprintType)
struct FEnvironmentalHazard
{
    GENERATED_BODY()

    /** Hazard type identifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard")
    FString HazardType;

    /** Hazard intensity (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    /** Frequency of hazard occurrence */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Frequency = 0.1f;

    /** Area of effect radius (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float EffectRadius = 500.0f;

    /** Damage per second to player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hazard", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float DamagePerSecond = 10.0f;
};

/**
 * @brief Biome Configuration Data Asset
 * 
 * Defines all characteristics of a specific biome including:
 * - Visual appearance (landscape materials, vegetation)
 * - Gameplay mechanics (resource spawning, hazards)
 * - Wildlife behavior parameters
 * - Environmental conditions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBiomeData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UBiomeData();

    /** Biome identification */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Info")
    EBiomeType BiomeType = EBiomeType::DenseJungle;

    /** Human-readable biome name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Info")
    FText BiomeName;

    /** Description of this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Info")
    FText BiomeDescription;

    /** Biome color for visualization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Info")
    FLinearColor BiomeColor = FLinearColor::Green;

    /** Landscape and terrain settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    TSoftObjectPtr<UMaterialInterface> LandscapeMaterial;

    /** Heightmap noise settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerrainRoughness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float TerrainScale = 1.0f;

    /** Elevation range for this biome (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    FVector2D ElevationRange = FVector2D(0.0f, 2000.0f);

    /** Slope preference (0 = flat, 1 = steep) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SlopePreference = 0.3f;

    /** Vegetation configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    EVegetationDensity VegetationDensity = EVegetationDensity::Dense;

    /** List of vegetation assets that can spawn in this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    TArray<FVegetationAsset> VegetationAssets;

    /** Tree height multiplier for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float TreeHeightMultiplier = 1.0f;

    /** Undergrowth density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float UndergrowthMultiplier = 1.0f;

    /** Wildlife configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wildlife")
    EWildlifeFrequency WildlifeFrequency = EWildlifeFrequency::Common;

    /** Preferred dinosaur types for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wildlife")
    TArray<FString> PreferredDinosaurTypes;

    /** Wildlife spawn density per km² */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wildlife", meta = (ClampMin = "0", ClampMax = "100"))
    int32 WildlifeSpawnDensity = 10;

    /** Predator to prey ratio */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wildlife", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PredatorRatio = 0.2f;

    /** Resource configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    EResourceAvailability ResourceAvailability = EResourceAvailability::Moderate;

    /** Available resources in this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    TArray<FResourceSpawnData> AvailableResources;

    /** Water availability (0.0 = desert, 1.0 = swamp) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterAvailability = 0.5f;

    /** Food availability for player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FoodAvailability = 0.5f;

    /** Environmental conditions */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    TArray<FEnvironmentalHazard> EnvironmentalHazards;

    /** Temperature range (Celsius) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    FVector2D TemperatureRange = FVector2D(20.0f, 35.0f);

    /** Humidity level (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HumidityLevel = 0.6f;

    /** Wind strength multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "3.0"))
    float WindStrength = 1.0f;

    /** Visibility range in this biome (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
    float VisibilityRange = 10000.0f;

    /** Audio settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> AmbientSound;

    /** Ambient sound volume multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float AmbientVolumeMultiplier = 1.0f;

    /** Biome transition settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions")
    TMap<EBiomeType, float> TransitionCompatibility;

    /** Transition blend distance (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
    float TransitionBlendDistance = 2000.0f;

    /** Performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxVegetationInstancesPerChunk = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxWildlifeInstancesPerChunk = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float LODDistance = 1000.0f;

public:
    /** Get vegetation asset by weight-based selection */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FVegetationAsset GetRandomVegetationAsset() const;

    /** Get resource spawn data by type */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FResourceSpawnData GetResourceSpawnData(const FString& ResourceType) const;

    /** Check if this biome is compatible with another for transitions */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsCompatibleWith(EBiomeType OtherBiomeType) const;

    /** Get transition blend factor with another biome */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTransitionBlendFactor(EBiomeType OtherBiomeType) const;

    /** Calculate vegetation density at a specific location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float CalculateVegetationDensityAt(const FVector& Location) const;

    /** Calculate resource availability at a specific location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float CalculateResourceAvailabilityAt(const FVector& Location, const FString& ResourceType) const;

    /** Get environmental danger level at location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetEnvironmentalDangerLevel(const FVector& Location) const;

#if WITH_EDITOR
    /** Validate biome data in editor */
    virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};

/**
 * @brief Biome Manager Component
 * 
 * Manages biome transitions and environmental effects for the current area.
 * Attached to the player or world manager to handle biome-specific logic.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBiomeManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeManagerComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Get current biome at player location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    UBiomeData* GetCurrentBiome() const;

    /** Get biome at specific world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    UBiomeData* GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Check if player is in biome transition zone */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsInTransitionZone() const;

    /** Get transition blend factor (0.0 = old biome, 1.0 = new biome) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTransitionBlendFactor() const;

    /** Force update biome detection */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateCurrentBiome();

    /** Event called when entering a new biome */
    UFUNCTION(BlueprintImplementableEvent, Category = "Biome")
    void OnBiomeEntered(UBiomeData* NewBiome, UBiomeData* PreviousBiome);

    /** Event called when leaving a biome */
    UFUNCTION(BlueprintImplementableEvent, Category = "Biome")
    void OnBiomeExited(UBiomeData* OldBiome, UBiomeData* NewBiome);

    /** Event called during biome transition */
    UFUNCTION(BlueprintImplementableEvent, Category = "Biome")
    void OnBiomeTransition(UBiomeData* FromBiome, UBiomeData* ToBiome, float BlendFactor);

protected:
    /** Current biome data */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    UBiomeData* CurrentBiome;

    /** Previous biome (for transitions) */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    UBiomeData* PreviousBiome;

    /** Available biome data assets */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    TArray<UBiomeData*> AvailableBiomes;

    /** Update frequency for biome detection */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float UpdateInterval = 1.0f;

    /** Distance threshold for biome changes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float BiomeChangeThreshold = 1000.0f;

private:
    /** Timer for biome updates */
    float UpdateTimer = 0.0f;

    /** Last known player location */
    FVector LastPlayerLocation = FVector::ZeroVector;

    /** Current transition state */
    bool bInTransition = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 2.0f;
};