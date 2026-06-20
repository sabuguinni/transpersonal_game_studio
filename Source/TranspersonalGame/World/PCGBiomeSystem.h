// PCGBiomeSystem.h
// Procedural World Generator — Agent #05
// PROD_CYCLE_AUTO_20260620_007
// Biome zone definitions, scatter parameters, and world partition integration.
// All types prefixed World_ per RULE 2.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "PCGBiomeSystem.generated.h"

// ─── BIOME TYPE ENUM ──────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Cretaceous Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Rocky       UMETA(DisplayName = "Rocky Badlands"),
    Swamp       UMETA(DisplayName = "Swamp / Wetland"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    COUNT       UMETA(Hidden)
};

// ─── WEATHER STATE ENUM ───────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    COUNT       UMETA(Hidden)
};

// ─── BIOME SCATTER PARAMETERS ─────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeScatterParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    // World-space center of this biome zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FVector ZoneCenter = FVector::ZeroVector;

    // Radius in cm within which foliage is scattered
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float ZoneRadius = 2500.0f;

    // Max foliage instances per biome zone (performance cap from PerformanceConfig.h)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    int32 MaxFoliageInstances = 200;

    // Foliage density: instances per 10000 cm² (100m²)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float FoliageDensity = 0.8f;

    // Average tree/rock height scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float AverageHeightScale = 2.0f;

    // Scale variance (±)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float HeightScaleVariance = 0.8f;

    // Random seed for deterministic scatter
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    int32 ScatterSeed = 42;

    // Cull distance for foliage in this zone (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float CullDistanceCm = 20000.0f;
};

// ─── BIOME ZONE ACTOR ─────────────────────────────────────────────────────────
// Placed in the level to define a biome region.
// PCGWorldGenerator reads these at runtime to drive foliage scatter.
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Zone Marker"))
class TRANSPERSONALGAME_API AWorld_BiomeZoneMarker : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeZoneMarker();

    // Biome parameters for this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FWorld_BiomeScatterParams BiomeParams;

    // Current weather state in this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Weather")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    // Temperature in Celsius (affects dinosaur behaviour)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Climate")
    float TemperatureCelsius = 28.0f;

    // Humidity 0-1 (affects fog density, plant density)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Climate")
    float Humidity = 0.6f;

    // Whether this zone has a water body (river/lake)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Water")
    bool bHasWaterBody = false;

    // Water body center offset from zone center
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Water",
              meta = (EditCondition = "bHasWaterBody"))
    FVector WaterBodyOffset = FVector::ZeroVector;

    // Returns true if a world-space point is inside this biome zone
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    bool IsPointInZone(const FVector& WorldPoint) const;

    // Returns the biome type at a given point (accounts for blend radius)
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtPoint(const FVector& WorldPoint) const;

    // Returns a normalised blend weight (0-1) for this zone at WorldPoint
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetBlendWeightAtPoint(const FVector& WorldPoint) const;

    // Called by PCGWorldGenerator to regenerate scatter in this zone
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biome")
    void RegenerateScatter();

    // Debug: draw zone boundary in editor
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Debug")
    void DrawZoneBoundary();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    // Blend falloff radius (10% of zone radius)
    float GetBlendFalloffRadius() const { return BiomeParams.ZoneRadius * 0.1f; }
};

// ─── WORLD BIOME MANAGER ──────────────────────────────────────────────────────
// Singleton-style actor that aggregates all BiomeZoneMarkers in the level
// and provides query interface for other systems.
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "World Biome Manager"))
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

    // All registered biome zones (auto-populated on BeginPlay)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Biome")
    TArray<AWorld_BiomeZoneMarker*> RegisteredZones;

    // Global weather override (overrides per-zone weather when active)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Weather")
    bool bGlobalWeatherOverride = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Weather",
              meta = (EditCondition = "bGlobalWeatherOverride"))
    EWorld_WeatherState GlobalWeatherState = EWorld_WeatherState::Clear;

    // Day/night cycle speed multiplier (1.0 = real-time, 60.0 = 1 game day per 24 real minutes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|DayNight",
              meta = (ClampMin = "0.1", ClampMax = "3600.0"))
    float DayNightSpeedMultiplier = 60.0f;

    // Current time of day in hours (0-24)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|DayNight")
    float CurrentTimeOfDay = 8.0f;

    // Query: what biome is at this world position?
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtWorldPosition(const FVector& WorldPos) const;

    // Query: what weather is active at this world position?
    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    EWorld_WeatherState GetWeatherAtWorldPosition(const FVector& WorldPos) const;

    // Query: temperature at world position
    UFUNCTION(BlueprintCallable, Category = "World|Climate")
    float GetTemperatureAtWorldPosition(const FVector& WorldPos) const;

    // Trigger a weather transition in a specific zone
    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void SetWeatherInZone(AWorld_BiomeZoneMarker* Zone, EWorld_WeatherState NewWeather);

    // Scan level for all BiomeZoneMarkers and register them
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biome")
    void ScanAndRegisterZones();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void AdvanceDayNightCycle(float DeltaTime);
    void UpdateDirectionalLight();

    // Cached directional light reference
    UPROPERTY()
    class ADirectionalLight* SunLight = nullptr;
};
