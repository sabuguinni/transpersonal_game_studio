// BiomeManager.h
// Agent #05 — Procedural World Generator
// Manages biome zones, terrain variation, and environmental audio triggers
// CYCLE: PROD_CYCLE_AUTO_20260626_004

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// ENUMS — declared at global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    TropicalJungle    UMETA(DisplayName = "Tropical Jungle"),
    OpenSavanna       UMETA(DisplayName = "Open Savanna"),
    RockyBadlands     UMETA(DisplayName = "Rocky Badlands"),
    RiverDelta        UMETA(DisplayName = "River Delta"),
    VolcanicPlains    UMETA(DisplayName = "Volcanic Plains"),
    DenseForest       UMETA(DisplayName = "Dense Forest"),
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear             UMETA(DisplayName = "Clear"),
    Overcast          UMETA(DisplayName = "Overcast"),
    LightRain         UMETA(DisplayName = "Light Rain"),
    HeavyStorm        UMETA(DisplayName = "Heavy Storm"),
    HeatHaze          UMETA(DisplayName = "Heat Haze"),
};

// ============================================================
// STRUCTS — declared at global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::TropicalJungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;
};

USTRUCT(BlueprintType)
struct FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString FeatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsWaterSource = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsNavigable = true;
};

USTRUCT(BlueprintType)
struct FWorld_WaterBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FString WaterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Extent = FVector(5000.0f, 500.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bIsFreshWater = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float FlowSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bHasDinosaurActivity = false;
};

// ============================================================
// UCLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Biome Zones ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    float WeatherTransitionSpeed = 0.1f;

    // ---- Terrain Features ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float GlobalHeightScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float TerrainRoughness = 0.6f;

    // ---- Water Bodies ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Water")
    TArray<FWorld_WaterBody> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Water")
    float GlobalWaterLevel = 50.0f;

    // ---- PCG Settings ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    int32 VegetationSeed = 42;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    float FoliageSpawnRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    int32 MaxFoliageInstancesPerBiome = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    bool bEnablePCGGeneration = true;

    // ---- Day/Night ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|DayNight")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|DayNight")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|DayNight")
    bool bEnableDayNightCycle = true;

    // ---- Blueprint-callable functions ----

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FWorld_BiomeZone GetBiomeZoneData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    void SetWeatherState(EWorld_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    FWorld_TerrainFeature GetNearestWaterSource(FVector FromLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    bool IsLocationInWater(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "World|DayNight")
    float GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "World|DayNight")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    void RegenerateBiomeFoliage(EWorld_BiomeType BiomeType);

    UFUNCTION(CallInEditor, Category = "World|Debug")
    void PrintBiomeDebugInfo();

private:
    void InitializeDefaultBiomes();
    void InitializeDefaultWaterBodies();
    void AdvanceDayNightCycle(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);

    float WeatherBlendAlpha = 0.0f;
    EWorld_WeatherState TargetWeather = EWorld_WeatherState::Clear;
};
