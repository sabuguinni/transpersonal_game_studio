#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Jungle Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Swamp       UMETA(DisplayName = "Prehistoric Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic Zone"),
    River       UMETA(DisplayName = "River Corridor"),
    Lake        UMETA(DisplayName = "Lake Basin"),
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor;

    FWorld_BiomeZone()
        : ZoneName(TEXT("Unknown"))
        , BiomeType(EWorld_BiomeType::Plains)
        , CenterLocation(FVector::ZeroVector)
        , Radius(5000.0f)
        , BaseElevation(0.0f)
        , Temperature(25.0f)
        , Humidity(0.5f)
        , BiomeColor(FLinearColor::Green)
    {}
};

USTRUCT(BlueprintType)
struct FWorld_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TimeOfDay;

    FWorld_WeatherState()
        : RainIntensity(0.0f)
        , WindSpeed(5.0f)
        , FogDensity(0.1f)
        , CloudCoverage(0.3f)
        , TimeOfDay(12.0f)
    {}
};

/**
 * ABiomeManager — Manages biome zones, weather states, and environmental audio
 * for the prehistoric survival world. Placed once in the level.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ── Biome Data ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Biomes")
    EWorld_BiomeType CurrentPlayerBiome;

    // ── Weather ───────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Weather")
    FWorld_WeatherState CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Weather")
    float WeatherTransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Weather")
    bool bEnableDynamicWeather;

    // ── Day/Night ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|DayNight")
    float DayLengthSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|DayNight")
    float CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|DayNight")
    bool bEnableDayNightCycle;

    // ── PCG Settings ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    int32 WorldSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    float WorldScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    int32 MaxBiomeZones;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Biome Query API ───────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FWorld_BiomeZone GetBiomeZoneData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetElevationAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // ── Weather API ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void SetWeatherState(const FWorld_WeatherState& NewWeather);

    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    FWorld_WeatherState GetCurrentWeather() const;

    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void TriggerStorm(float Intensity, float DurationSeconds);

    // ── Day/Night API ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "World|DayNight")
    float GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "World|DayNight")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "World|DayNight")
    FString GetTimeOfDayName() const;

    // ── Initialization ────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void GenerateDefaultBiomes();

    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    void RegisterBiomeZone(const FWorld_BiomeZone& NewZone);

private:
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void UpdatePlayerBiome();
    void InitializeDefaultBiomes();

    FWorld_WeatherState TargetWeather;
    float StormTimer;
    bool bStormActive;
};
