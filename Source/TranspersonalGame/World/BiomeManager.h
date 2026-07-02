#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// Biome type enum — prehistoric Cretaceous biomes
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Jungle         UMETA(DisplayName = "Jungle"),
    Savanna        UMETA(DisplayName = "Savanna"),
    Swamp          UMETA(DisplayName = "Swamp"),
    VolcanicPlains UMETA(DisplayName = "Volcanic Plains"),
    RiverDelta     UMETA(DisplayName = "River Delta"),
    ForestCanopy   UMETA(DisplayName = "Forest Canopy"),
    CoastalFlats   UMETA(DisplayName = "Coastal Flats"),
    Unknown        UMETA(DisplayName = "Unknown")
};

// Per-biome data
USTRUCT(BlueprintType)
struct FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 28.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.6f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown Biome");
};

/**
 * ABiomeManager — manages biome zones, transitions, and environmental properties
 * for the Cretaceous prehistoric survival world.
 * Placed once in the level at origin; queries are made by world position.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- Query API ---

    /** Returns the biome type at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /** Returns full biome data at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FBiomeData GetBiomeDataAtLocation(FVector WorldLocation) const;

    /** Returns the blended fog color for a transition between two biomes */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FLinearColor GetBlendedFogColor(FVector WorldLocation) const;

    /** Returns temperature at location (affects survival stats) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    /** Returns humidity at location (affects thirst drain rate) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(FVector WorldLocation) const;

    /** Returns dinosaur spawn multiplier for the biome at location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDinosaurSpawnMultiplierAtLocation(FVector WorldLocation) const;

    // --- Configuration ---

    /** World radius used for biome zone calculations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldRadius = 10000.0f;

    /** Number of biome zones to generate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    int32 NumBiomeZones = 8;

    /** Blend transition width in world units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeBlendRadius = 500.0f;

    /** All registered biome data entries */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Data")
    TArray<FBiomeData> BiomeTable;

    /** Enable debug visualization of biome zones */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDebugDrawBiomes = false;

    // --- Internal ---

private:
    /** Initialize default biome table with Cretaceous biomes */
    void InitializeBiomeTable();

    /** Compute biome index from world XY using angular sectors */
    int32 ComputeBiomeSectorIndex(FVector WorldLocation) const;

    /** Draw debug spheres for biome zone centers */
    void DebugDrawBiomeZones() const;
};
