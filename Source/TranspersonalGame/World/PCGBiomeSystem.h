// PCGBiomeSystem.h
// Transpersonal Game Studio — Agent #05 Procedural World Generator
// Biome system for Cretaceous world — forest, plains, rocky highlands, wetlands
// PROD_CYCLE_AUTO_20260625_003

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "PCGBiomeSystem.generated.h"

// ============================================================
// ENUMS — Global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    None            UMETA(DisplayName = "None"),
    TropicalJungle  UMETA(DisplayName = "Tropical Jungle"),
    OpenSavanna     UMETA(DisplayName = "Open Savanna"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    Wetlands        UMETA(DisplayName = "Wetlands / River Delta"),
    CoastalBeach    UMETA(DisplayName = "Coastal Beach"),
    VolcanicPlains  UMETA(DisplayName = "Volcanic Plains")
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyStorm  UMETA(DisplayName = "Heavy Storm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Drought     UMETA(DisplayName = "Drought / Heat Haze")
};

// ============================================================
// STRUCTS — Global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::TropicalJungle;

    // World-space center of this biome zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector ZoneCenter = FVector::ZeroVector;

    // Radius in Unreal units
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ZoneRadius = 5000.0f;

    // Blend falloff at zone edges (0 = hard edge, 1 = full blend)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BlendRadius = 1000.0f;

    // Vegetation density 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Vegetation")
    float VegetationDensity = 0.7f;

    // Average tree height in Unreal units
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Vegetation")
    float AverageTreeHeight = 2000.0f;

    // Ground height variation amplitude
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float TerrainAmplitude = 500.0f;

    // Noise frequency for terrain variation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float TerrainFrequency = 0.001f;

    // Fog density for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Atmosphere")
    float FogDensity = 0.02f;

    // Ambient temperature (affects survival mechanics)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float AmbientTemperatureCelsius = 28.0f;

    // Humidity 0-1 (affects stamina drain)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float Humidity = 0.8f;

    // Dominant dinosaur species in this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Fauna")
    TArray<FName> DominantDinoSpecies;

    // Water sources count in zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Water")
    int32 WaterSourceCount = 2;
};

USTRUCT(BlueprintType)
struct FWorld_WeatherEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState WeatherType = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f; // seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionTime = 30.0f; // blend time in seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainfallIntensity = 0.0f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed = 0.0f; // km/h

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightningProbability = 0.0f; // 0-1 per minute
};

USTRUCT(BlueprintType)
struct FWorld_PCGSpawnRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    FName AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    FSoftObjectPath AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    EWorld_BiomeType TargetBiome = EWorld_BiomeType::TropicalJungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    float SpawnDensity = 0.5f; // instances per 1000 sq units

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    FVector ScaleMin = FVector(0.8f, 0.8f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    FVector ScaleMax = FVector(1.5f, 1.5f, 1.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    float MinSlopeAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    float MaxSlopeAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    bool bAlignToSurface = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    bool bUseHierarchicalInstancing = true;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(ClassGroup = (WorldGeneration), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    // === BIOME ZONES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    // === WEATHER ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TArray<FWorld_WeatherEvent> WeatherSchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionAlpha = 0.0f;

    // === PCG SPAWN RULES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TArray<FWorld_PCGSpawnRule> SpawnRules;

    // === DAY/NIGHT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float CurrentTimeOfDay = 0.5f; // 0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float DayDurationSeconds = 1200.0f; // 20 minutes real time = 1 game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    bool bEnableDayNightCycle = true;

    // === WORLD BOUNDS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    FVector WorldExtent = FVector(20000.0f, 20000.0f, 5000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 PCGSeed = 12345;

    // === RUNTIME STATE ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime",
              meta = (AllowPrivateAccess = "true"))
    int32 TotalVegetationInstances = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime",
              meta = (AllowPrivateAccess = "true"))
    int32 ActiveBiomeIndex = 0;

    // === FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeZone GetBiomeZoneAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetBiomeBlendWeight(FVector WorldLocation, int32 BiomeIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(EWorld_WeatherState NewWeather, float TransitionTime = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EWorld_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    FLinearColor GetSkyColorForTime(float TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "PCG")
    void RegenerateVegetation();

    UFUNCTION(BlueprintCallable, Category = "PCG")
    int32 GetVegetationCount() const { return TotalVegetationInstances; }

    UFUNCTION(BlueprintCallable, Category = "World")
    float SampleTerrainHeight(FVector2D WorldXY) const;

    UFUNCTION(BlueprintCallable, Category = "World")
    float SampleNoise(float X, float Y, float Frequency, int32 Octaves = 4) const;

    // === OVERRIDES ===
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    UFUNCTION(CallInEditor, Category = "PCG")
    void GenerateWorldInEditor();

    UFUNCTION(CallInEditor, Category = "PCG")
    void ClearGeneratedActors();
#endif

protected:
    void InitializeDefaultBiomes();
    void TickDayNightCycle(float DeltaTime);
    void TickWeatherSystem(float DeltaTime);
    void UpdateDirectionalLight();

private:
    float WeatherTimer = 0.0f;
    int32 WeatherScheduleIndex = 0;

    UPROPERTY()
    TArray<AActor*> GeneratedActors;
};
