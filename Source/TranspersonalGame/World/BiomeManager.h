// BiomeManager.h
// Agent #5 — Procedural World Generator
// Manages biome zones, transitions, and environmental properties for the prehistoric world

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
    River       UMETA(DisplayName = "River Valley"),
    Forest      UMETA(DisplayName = "Dense Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Volcanic    UMETA(DisplayName = "Volcanic Region"),
    Swamp       UMETA(DisplayName = "Swampland"),
    Count       UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Count       UMETA(Hidden)
};

// ============================================================
// STRUCTS — declared at global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FWorld_BiomeProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown Biome");

    /** Ambient temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float AmbientTemperature = 25.0f;

    /** Humidity 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float Humidity = 0.5f;

    /** Vegetation density 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Foliage")
    float VegetationDensity = 0.5f;

    /** Predator spawn weight 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Fauna")
    float PredatorSpawnWeight = 0.3f;

    /** Herbivore spawn weight 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Fauna")
    float HerbivoreSpawnWeight = 0.6f;

    /** Fog density override 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Atmosphere")
    float FogDensity = 0.02f;

    /** Sky light color tint for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Atmosphere")
    FLinearColor SkyTint = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    /** Radius of this biome zone in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Zone")
    float BiomeRadius = 300000.0f;

    /** Center of this biome zone in world space */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Zone")
    FVector BiomeCenter = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FWorld_WeatherProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState WeatherState = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightningFrequency = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f;
};

// ============================================================
// UCLASS
// ============================================================

UCLASS(ClassGroup = (WorldGen), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Biome Query ----

    /** Returns the biome type at the given world location */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full biome properties at the given world location */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    FWorld_BiomeProperties GetBiomePropertiesAtLocation(const FVector& WorldLocation) const;

    /** Returns the current active weather state */
    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    EWorld_WeatherState GetCurrentWeather() const;

    /** Returns current weather properties */
    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    FWorld_WeatherProperties GetCurrentWeatherProperties() const;

    // ---- Biome Registration ----

    /** Register a biome zone at runtime */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    void RegisterBiomeZone(const FWorld_BiomeProperties& BiomeProps);

    /** Clear all registered biome zones */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    void ClearAllBiomes();

    // ---- Weather Control ----

    /** Trigger a weather transition */
    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void TriggerWeatherTransition(EWorld_WeatherState NewWeather, float TransitionDuration = 30.0f);

    /** Force immediate weather state (no transition) */
    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void SetWeatherImmediate(EWorld_WeatherState NewWeather);

    // ---- Day/Night Cycle ----

    /** Current time of day in hours (0-24) */
    UFUNCTION(BlueprintCallable, Category = "World|DayNight")
    float GetTimeOfDay() const;

    /** Set time of day directly */
    UFUNCTION(BlueprintCallable, Category = "World|DayNight")
    void SetTimeOfDay(float Hours);

    /** Day/night cycle speed multiplier (1.0 = real time) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|DayNight")
    float DayNightSpeedMultiplier = 60.0f;

    /** Whether day/night cycle is active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|DayNight")
    bool bDayNightCycleActive = true;

    // ---- Biome Data ----

    /** All registered biome zones */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    TArray<FWorld_BiomeProperties> RegisteredBiomes;

    /** Default biome used when no zone matches */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FWorld_BiomeProperties DefaultBiome;

    /** Weather transition blend alpha (0=old, 1=new) */
    UPROPERTY(BlueprintReadOnly, Category = "World|Weather", meta = (AllowPrivateAccess = "true"))
    float WeatherBlendAlpha = 1.0f;

private:
    /** Current time of day in hours */
    float CurrentTimeOfDay = 8.0f;

    /** Current weather state */
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    /** Target weather during transition */
    EWorld_WeatherState TargetWeather = EWorld_WeatherState::Clear;

    /** Current weather properties */
    FWorld_WeatherProperties CurrentWeatherProps;

    /** Weather transition duration */
    float WeatherTransitionDuration = 30.0f;

    /** Weather transition elapsed time */
    float WeatherTransitionElapsed = 0.0f;

    /** Whether a weather transition is in progress */
    bool bWeatherTransitioning = false;

    /** Update day/night cycle */
    void UpdateDayNightCycle(float DeltaTime);

    /** Update weather transition */
    void UpdateWeatherTransition(float DeltaTime);

    /** Apply sun rotation based on time of day */
    void ApplySunRotation(float TimeOfDay);

    /** Get default properties for a biome type */
    FWorld_BiomeProperties GetDefaultPropertiesForBiome(EWorld_BiomeType BiomeType) const;

    /** Initialize default biome data */
    void InitializeDefaultBiomes();
};
