#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
//  Biome type enumeration — prehistoric survival world
// ============================================================
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Jungle          UMETA(DisplayName = "Jungle"),
    Savanna         UMETA(DisplayName = "Savanna"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    CoastalPlain    UMETA(DisplayName = "Coastal Plain"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    MontaneForest   UMETA(DisplayName = "Montane Forest"),
    Badlands        UMETA(DisplayName = "Badlands")
};

// ============================================================
//  Per-biome data — survival parameters + visual identity
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString DisplayName = TEXT("Savanna");

    /** World-space radius of this biome zone in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RadiusCm = 50000.0f;

    /** Centre of this biome in world space */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CentreLocation = FVector::ZeroVector;

    // --- Survival modifiers ---
    /** Temperature in Celsius — affects stamina drain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float AmbientTemperatureC = 28.0f;

    /** Humidity 0-1 — affects thirst drain rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Humidity = 0.5f;

    /** Predator density multiplier 0-2 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float PredatorDensity = 1.0f;

    /** Food resource availability 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FoodAvailability = 0.5f;

    // --- Visual identity ---
    /** Fog density multiplier for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float FogDensityMultiplier = 1.0f;

    /** Ground colour tint (R,G,B) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor GroundTint = FLinearColor(0.4f, 0.35f, 0.2f, 1.0f);

    /** Foliage density 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float FoliageDensity = 0.5f;
};

// ============================================================
//  BiomeManager — world-space biome authority
//  Placed once in the level; queried by all other systems.
// ============================================================
UCLASS(ClassGroup = "TranspersonalGame", meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // --- Blueprint-callable API ---

    /** Returns the biome type at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full biome data at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    /** Returns true if the location is inside the named biome */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInBiome(const FVector& WorldLocation, EEng_BiomeType BiomeType) const;

    /** Returns predator density multiplier at location */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetPredatorDensityAt(const FVector& WorldLocation) const;

    /** Returns ambient temperature at location */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetTemperatureAt(const FVector& WorldLocation) const;

    /** Returns food availability at location */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFoodAvailabilityAt(const FVector& WorldLocation) const;

    /** Registers a new biome zone at runtime (called by PCGWorldGenerator) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiome(const FEng_BiomeData& BiomeData);

    /** Returns all registered biomes */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FEng_BiomeData> GetAllBiomes() const;

    /** Clears all biomes and rebuilds defaults */
    UFUNCTION(CallInEditor, Category = "Biome")
    void RebuildDefaultBiomes();

protected:
    virtual void BeginPlay() override;

    /** All biome zones in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FEng_BiomeData> BiomeZones;

    /** Default biome returned when no zone matches */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FEng_BiomeData DefaultBiome;

private:
    /** Builds the 8 default prehistoric biomes on first run */
    void InitializeDefaultBiomes();

    /** Finds the closest biome zone to a world location */
    const FEng_BiomeData* FindClosestBiome(const FVector& WorldLocation) const;
};
