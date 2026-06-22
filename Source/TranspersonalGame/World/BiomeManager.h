// BiomeManager.h
// Engine Architect #02 — Transpersonal Game Studio
// P1 World Generation — Biome system header

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// Biome data struct — defined at global scope (UHT rule)
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "-60.0", ClampMax = "80.0"))
    float TemperatureMin = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "-60.0", ClampMax = "80.0"))
    float TemperatureMax = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bAllowsDinosaurSpawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float DinosaurSpawnWeight = 1.0f;
};

/**
 * UBiomeManager — World Subsystem
 * Manages biome data, temperature, humidity, and dinosaur spawn weights
 * for the prehistoric survival world. Queried by PCGWorldGenerator,
 * FoliageManager, and DinosaurSpawnSystem.
 *
 * Architecture: WorldSubsystem (auto-created per world, no manual registration)
 * Dependencies: SharedTypes.h (EBiomeType), PCGWorldGenerator (consumer)
 */
UCLASS(BlueprintType, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Biome Queries (Blueprint-callable) ---

    /** Returns the biome type at the given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full biome data for a given biome type. Returns false if not found. */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    bool GetBiomeData(EBiomeType BiomeType, FEng_BiomeData& OutBiomeData) const;

    /** Returns interpolated temperature at location (accounts for altitude) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Environment")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Returns humidity value [0..1] at location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Environment")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    /** Returns true if dinosaurs can spawn at this location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Dinosaurs")
    bool CanDinosaursSpawnAtLocation(const FVector& WorldLocation) const;

    /** Returns spawn weight multiplier for dinosaurs at this location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Dinosaurs")
    float GetDinosaurSpawnWeightAtLocation(const FVector& WorldLocation) const;

    /** Returns list of all currently active biome types */
    UFUNCTION(BlueprintCallable, Category = "Biome|Management")
    TArray<EBiomeType> GetActiveBiomes() const;

    /** Register a biome as active (called by PCGWorldGenerator) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Management")
    void RegisterActiveBiome(EBiomeType BiomeType);

    /** Blend radius for biome transitions in world units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Settings")
    float TransitionBlendRadius;

    /** Currently active biome at player position */
    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    EBiomeType ActiveBiomeType;

private:
    void InitializeBiomeData();

    UPROPERTY()
    TMap<EBiomeType, FEng_BiomeData> BiomeDataMap;

    UPROPERTY()
    TArray<EBiomeType> ActiveBiomes;

    bool bBiomeSystemInitialized;
};
