// BiomeAudioManager.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260620_001
// Provides runtime biome detection and ambient temperature data for
// SurvivalComponent and Audio Agent (#16) MetaSound integration.
// All types use "World_" prefix per RULE 2.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BiomeAudioManager.generated.h"

// ── Enums ──────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Unknown     UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear   UMETA(DisplayName = "Clear"),
    Rain    UMETA(DisplayName = "Rain"),
    Storm   UMETA(DisplayName = "Storm"),
    Mist    UMETA(DisplayName = "Mist")
};

// ── Structs ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeRecord : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    /** World-space centre of the biome zone (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    /** Radius of influence in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RadiusCm = 250000.f;

    /** Ambient temperature in Celsius — fed to SurvivalComponent */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float TemperatureCelsius = 22.f;

    /** Relative humidity 0-100% */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float HumidityPercent = 50.f;

    /** Chance of precipitation per in-game hour (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Weather")
    float PrecipitationChance = 0.2f;

    /** Wind speed km/h — affects stamina drain and sound propagation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Survival")
    float WindSpeedKmh = 10.f;

    /** Exponential height fog density override for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensity = 0.1f;

    /** MetaSound cue asset path for ambient audio */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    FString AmbientSoundCuePath;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherRecord : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState WeatherState = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float BlendRadiusCm = 300000.f;

    /** Rain particle intensity 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.f;

    /** Probability of lightning strike per in-game minute */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightningChance = 0.f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeQuery
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType PrimaryBiome = EWorld_BiomeType::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float TemperatureCelsius = 22.f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float HumidityPercent = 50.f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float WindSpeedKmh = 10.f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float FogDensity = 0.1f;

    /** Blend weight 0-1 of the primary biome at query location */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendWeight = 1.f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FString AmbientSoundCuePath;
};

// ── Manager Actor ──────────────────────────────────────────────────────────

/**
 * UWorld_BiomeAudioManager
 *
 * Singleton actor placed in MinPlayableMap.
 * Provides:
 *   - QueryBiomeAtLocation() → FWorld_BiomeQuery for SurvivalComponent
 *   - GetWeatherAtLocation() → EWorld_WeatherState for Audio/Lighting agents
 *   - Driven by BiomeData.json loaded from Content/WorldData/ at BeginPlay
 *
 * Agent #16 (Audio) queries this actor to select MetaSound parameters.
 * Agent #08 (Lighting) queries this actor to blend fog density.
 * SurvivalComponent queries this actor for ambient temperature.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "WorldGen")
class TRANSPERSONALGAME_API AWorld_BiomeAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeAudioManager();

protected:
    virtual void BeginPlay() override;

public:
    // ── Data ──────────────────────────────────────────────────────────────

    /** Biome zones loaded from BiomeData.json */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Data")
    TArray<FWorld_BiomeRecord> BiomeZones;

    /** Weather zones loaded from BiomeData.json */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Data")
    TArray<FWorld_WeatherRecord> WeatherZones;

    /** Path to BiomeData.json relative to Content dir */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    FString BiomeDataJsonPath = TEXT("WorldData/BiomeData.json");

    /** Whether data was loaded successfully */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Data")
    bool bDataLoaded = false;

    // ── Query API ─────────────────────────────────────────────────────────

    /**
     * Query biome data at a world-space location.
     * Returns the nearest biome with blend weight based on distance.
     * Called by SurvivalComponent every 5 seconds to update temperature.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    FWorld_BiomeQuery QueryBiomeAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns the dominant weather state at a world-space location.
     * Called by Audio agent to select rain/storm MetaSound parameters.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    EWorld_WeatherState GetWeatherAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns ambient temperature at location in Celsius.
     * Direct integration point for SurvivalComponent::SetAmbientTemperature().
     */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns fog density override for the biome at location.
     * Used by Lighting agent to blend ExponentialHeightFog density.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetFogDensityAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns the MetaSound cue path for ambient audio at location.
     * Used by Audio agent to switch MetaSound sources.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    FString GetAmbientSoundCueAtLocation(const FVector& WorldLocation) const;

    // ── Utility ───────────────────────────────────────────────────────────

    /** Force reload BiomeData.json from disk */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void ReloadBiomeData();

    /** Log all loaded biome zones to output log */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DebugLogBiomes() const;

private:
    /** Load and parse BiomeData.json */
    void LoadBiomeDataFromJson();

    /** Find nearest biome record to location, returns index or -1 */
    int32 FindNearestBiomeIndex(const FVector& WorldLocation) const;

    /** Find nearest weather zone to location, returns index or -1 */
    int32 FindNearestWeatherIndex(const FVector& WorldLocation) const;
};
