// BiomeManager.h
// Engine Architect #02 — P1 World Generation: Biome System
// Cycle: PROD_CYCLE_AUTO_20260619_010

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BiomeManager.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    TropicalJungle  UMETA(DisplayName = "Tropical Jungle"),
    Savanna         UMETA(DisplayName = "Savanna"),
    TemperateForest UMETA(DisplayName = "Temperate Forest"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    Coastal         UMETA(DisplayName = "Coastal"),
    Highland        UMETA(DisplayName = "Highland"),
    Desert          UMETA(DisplayName = "Desert"),
};

// ============================================================
// STRUCTS — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeID = EEng_BiomeType::TemperateForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FName BiomeName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FText DisplayName;

    // Temperature range [0..1] — 0=arctic, 1=tropical
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinTemperature = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MaxTemperature = 1.0f;

    // Moisture range [0..1] — 0=arid, 1=saturated
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinMoisture = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MaxMoisture = 1.0f;

    // Altitude range [0..1] — 0=sea level, 1=mountain peak
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinAltitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MaxAltitude = 1.0f;

    // Gameplay properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterBodyChance = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DangerLevel = 0.5f;

    // Visual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor GroundColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
};

// ============================================================
// UCLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ---- Query API ----

    /** Returns the biome type at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns the full definition struct for a biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType BiomeType) const;

    /** Returns all registered biome types */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<EEng_BiomeType> GetAllBiomeTypes() const;

    /** Returns danger level [0..1] for a biome */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetBiomeDangerLevel(EEng_BiomeType BiomeType) const;

    /** Returns foliage density [0..1] at a world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetFoliageDensityAt(const FVector& WorldLocation) const;

    /** Returns true if this location is likely a water body */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsWaterBodyLocation(const FVector& WorldLocation) const;

    /** Returns the ground color for a biome (used by terrain material) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FLinearColor GetBiomeGroundColor(EEng_BiomeType BiomeType) const;

    /** Debug string for a location — biome name + T/M/A values */
    UFUNCTION(BlueprintCallable, Category = "Biome|Debug")
    FString GetBiomeDebugString(const FVector& WorldLocation) const;

    // ---- Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float TemperatureScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float MoistureScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float AltitudeScale = 1.0f;

private:
    /** All registered biome definitions, keyed by type */
    UPROPERTY()
    TMap<EEng_BiomeType, FEng_BiomeDefinition> RegisteredBiomes;

    bool bBiomesInitialized = false;

    void RegisterDefaultBiomes();

    /** Sample temperature, moisture, altitude at a world location */
    void SampleEnvironmentAt(const FVector& WorldLocation, float& OutTemperature, float& OutMoisture, float& OutAltitude) const;

    /** Classify biome from sampled values */
    EEng_BiomeType ClassifyBiome(float Temperature, float Moisture, float Altitude) const;
};
