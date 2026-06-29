#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BiomeManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    None            UMETA(DisplayName = "None"),
    Jungle          UMETA(DisplayName = "Jungle"),
    Savanna         UMETA(DisplayName = "Savanna"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    Coastal         UMETA(DisplayName = "Coastal"),
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert"),
    Mountain        UMETA(DisplayName = "Mountain"),
    River           UMETA(DisplayName = "River")
};

UENUM(BlueprintType)
enum class EBiomeClimate : uint8
{
    Tropical        UMETA(DisplayName = "Tropical"),
    Temperate       UMETA(DisplayName = "Temperate"),
    Arid            UMETA(DisplayName = "Arid"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    Coastal         UMETA(DisplayName = "Coastal")
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FBiomeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeClimate Climate = EBiomeClimate::Tropical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString DisplayName = TEXT("Unknown Biome");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Temperature")
    float MinTemperatureCelsius = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Temperature")
    float MaxTemperatureCelsius = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Humidity")
    float HumidityPercent = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Vegetation")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Danger")
    float DinosaurSpawnDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Danger")
    float DangerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Resources")
    float WaterAvailability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Resources")
    float FoodAvailability = 0.5f;
};

USTRUCT(BlueprintType)
struct FBiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType FromBiome = EBiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType ToBiome = EBiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TransitionWidthMeters = 200.0f;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Query API ----

    /** Returns the biome type at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full biome data for a given biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FBiomeData GetBiomeData(EBiomeType BiomeType) const;

    /** Returns the temperature at a world location (Celsius) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Environment")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Returns the humidity at a world location (0-100) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Environment")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    /** Returns the danger level at a world location (0-1) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Danger")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    /** Returns true if the location is a biome transition zone */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsInTransitionZone(const FVector& WorldLocation) const;

    /** Returns all biome types present in the current world */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<EBiomeType> GetAllActiveBiomes() const;

    // ---- Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldSizeKm = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    int32 BiomeSeed = 42;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeNoiseScale = 0.0005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FBiomeData> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FBiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDebugDrawBiomeBoundaries = false;

private:
    /** Initializes default biome definitions for the prehistoric world */
    void InitializeDefaultBiomes();

    /** Samples noise at a world location to determine biome */
    float SampleBiomeNoise(float X, float Y) const;

    /** Cached biome grid for fast lookup */
    TArray<EBiomeType> BiomeGrid;

    int32 BiomeGridResolution = 64;
};
