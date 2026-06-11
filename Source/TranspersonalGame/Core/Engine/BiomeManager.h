#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../../SharedTypes.h"
#include "BiomeManager.generated.h"

// Forward declarations
class UMaterialInterface;
class UStaticMesh;
class UTexture2D;

/**
 * Biome configuration data structure
 * Defines all properties for a specific biome type
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData : public FTableRowBase
{
    GENERATED_BODY()

    FEng_BiomeData()
    {
        BiomeType = EBiomeType::Grassland;
        Temperature = 20.0f;
        Humidity = 0.5f;
        Elevation = 0.0f;
        FoliageDensity = 0.7f;
        WaterPresence = 0.3f;
        RockDensity = 0.2f;
        BiomeName = TEXT("Default Grassland");
        BiomeDescription = TEXT("A temperate grassland biome");
    }

    /** Type of biome from SharedTypes enum */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType;

    /** Average temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate", meta = (ClampMin = "-50.0", ClampMax = "60.0"))
    float Temperature;

    /** Humidity level (0.0 = arid, 1.0 = very humid) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity;

    /** Preferred elevation range in meters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    float Elevation;

    /** Density of vegetation (0.0 = barren, 1.0 = dense forest) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FoliageDensity;

    /** Water presence (0.0 = desert, 1.0 = swamp/wetland) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterPresence;

    /** Rock and stone density */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RockDensity;

    /** Display name for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FString BiomeName;

    /** Description of this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FString BiomeDescription;

    /** Ground material for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> GroundMaterial;

    /** Primary vegetation meshes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Meshes")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    /** Rock and stone meshes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Meshes")
    TArray<TSoftObjectPtr<UStaticMesh>> RockMeshes;
};

/**
 * Biome transition data for smooth blending between biomes
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    FEng_BiomeTransition()
    {
        FromBiome = EBiomeType::Grassland;
        ToBiome = EBiomeType::Forest;
        TransitionDistance = 500.0f;
        BlendStrength = 0.5f;
    }

    /** Source biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    EBiomeType FromBiome;

    /** Target biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    EBiomeType ToBiome;

    /** Distance over which transition occurs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float TransitionDistance;

    /** Strength of blending effect */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendStrength;
};

/**
 * BiomeManager - Core system for managing world biomes
 * Handles biome data, transitions, and integration with world generation
 * This is the architectural foundation for all biome-related systems
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBiomeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE BIOME SYSTEM ===

    /** Initialize the biome system with data table */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomeSystem(UDataTable* BiomeDataTable);

    /** Get biome data for a specific biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool GetBiomeData(EBiomeType BiomeType, FEng_BiomeData& OutBiomeData) const;

    /** Determine biome type at world location */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get blended biome influence at location (for transitions) */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TMap<EBiomeType, float> GetBiomeInfluenceAtLocation(const FVector& WorldLocation) const;

    // === BIOME QUERIES ===

    /** Check if location is within a specific biome */
    UFUNCTION(BlueprintCallable, Category = "Biome Queries")
    bool IsLocationInBiome(const FVector& WorldLocation, EBiomeType BiomeType) const;

    /** Get temperature at location based on biome */
    UFUNCTION(BlueprintCallable, Category = "Biome Queries")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Get humidity at location based on biome */
    UFUNCTION(BlueprintCallable, Category = "Biome Queries")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    /** Get foliage density at location */
    UFUNCTION(BlueprintCallable, Category = "Biome Queries")
    float GetFoliageDensityAtLocation(const FVector& WorldLocation) const;

    // === INTEGRATION POINTS ===

    /** Get suitable vegetation meshes for location */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<UStaticMesh*> GetVegetationMeshesForLocation(const FVector& WorldLocation) const;

    /** Get ground material for location */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    UMaterialInterface* GetGroundMaterialForLocation(const FVector& WorldLocation) const;

    /** Validate biome placement rules */
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateBiomePlacement(EBiomeType BiomeType, const FVector& WorldLocation) const;

    // === EDITOR TOOLS ===

    /** Debug draw biome boundaries in editor */
    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugDrawBiomeBoundaries();

    /** Generate biome preview map */
    UFUNCTION(CallInEditor, Category = "Debug")
    void GenerateBiomePreviewMap();

    /** Validate all biome data consistency */
    UFUNCTION(CallInEditor, Category = "Validation")
    void ValidateAllBiomeData();

protected:
    // === CORE DATA ===

    /** Data table containing all biome configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    UDataTable* BiomeDataTable;

    /** Cached biome data for fast lookups */
    UPROPERTY()
    TMap<EBiomeType, FEng_BiomeData> CachedBiomeData;

    /** Biome transition configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_BiomeTransition> BiomeTransitions;

    // === SYSTEM PARAMETERS ===

    /** World scale factor for biome calculations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System", meta = (AllowPrivateAccess = "true", ClampMin = "0.1", ClampMax = "10.0"))
    float WorldScale;

    /** Noise scale for biome distribution */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System", meta = (AllowPrivateAccess = "true", ClampMin = "0.001", ClampMax = "1.0"))
    float NoiseScale;

    /** Random seed for biome generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System", meta = (AllowPrivateAccess = "true"))
    int32 BiomeSeed;

    /** Enable debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
    bool bShowDebugInfo;

private:
    // === INTERNAL METHODS ===

    /** Calculate noise value for biome distribution */
    float CalculateBiomeNoise(const FVector& WorldLocation) const;

    /** Find transition data between two biomes */
    const FEng_BiomeTransition* FindBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome) const;

    /** Calculate biome influence with distance falloff */
    float CalculateBiomeInfluence(const FVector& Location, const FVector& BiomeCenter, float Radius) const;

    /** Load and cache biome data from data table */
    void CacheBiomeData();

    /** Validate biome data integrity */
    bool ValidateBiomeDataIntegrity() const;
};