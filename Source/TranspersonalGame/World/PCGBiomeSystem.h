// PCGBiomeSystem.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260620_005
// Biome classification, weather zones, and terrain feature data for PCG pipeline

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "PCGBiomeSystem.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highland"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    River       UMETA(DisplayName = "River Valley"),
    COUNT       UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Ash         UMETA(DisplayName = "Volcanic Ash"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    COUNT       UMETA(Hidden)
};

// ============================================================
// STRUCTS — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeWeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_WeatherState DefaultWeather = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float RainProbability = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float StormProbability = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float AshfallProbability = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FLinearColor AmbientTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float TemperatureCelsius = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float HumidityPercent = 50.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FName BiomeName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FVector WorldCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float MinElevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float MaxElevation = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FWorld_BiomeWeatherConfig WeatherConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float DinosaurSpawnWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    FName FeatureName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    EWorld_BiomeType OwningBiome = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    bool bIsNavigationObstacle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float HeightVariation = 0.0f;
};

// ============================================================
// UCLASS: Biome Zone Actor — placed in world to mark biome boundaries
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Zone Actor"))
class TRANSPERSONALGAME_API AWorld_BiomeZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeZoneActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    // Biome definition for this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FWorld_BiomeDefinition BiomeDefinition;

    // Current active weather state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Weather")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    // Time until next weather transition (seconds)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Weather")
    float WeatherTransitionTimer = 0.0f;

    // Check if a world location falls within this biome zone
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    bool IsLocationInBiome(const FVector& WorldLocation) const;

    // Get the biome type at this zone
    UFUNCTION(BlueprintPure, Category = "World|Biome")
    EWorld_BiomeType GetBiomeType() const { return BiomeDefinition.BiomeType; }

    // Get current weather state
    UFUNCTION(BlueprintPure, Category = "World|Weather")
    EWorld_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    // Get temperature at this biome
    UFUNCTION(BlueprintPure, Category = "World|Biome")
    float GetTemperature() const { return BiomeDefinition.WeatherConfig.TemperatureCelsius; }

    // Force a weather transition (for testing/scripting)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Weather")
    void ForceWeatherTransition(EWorld_WeatherState NewWeather);

    // Evaluate and potentially transition weather
    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void EvaluateWeatherTransition(float DeltaSeconds);

private:
    // Sphere component for biome boundary visualization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Biome", meta = (AllowPrivateAccess = "true"))
    class USphereComponent* BiomeBoundary;

    float WeatherCheckInterval = 60.0f;
    float TimeSinceLastWeatherCheck = 0.0f;
};

// ============================================================
// UCLASS: PCG Biome Manager — world subsystem managing all biome zones
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "PCG Biome Manager"))
class TRANSPERSONALGAME_API AWorld_PCGBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_PCGBiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    // All registered biome zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    TArray<AWorld_BiomeZoneActor*> RegisteredBiomes;

    // Default biome definitions (loaded at startup)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    TArray<FWorld_BiomeDefinition> DefaultBiomeDefinitions;

    // Terrain features registered in world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    // Global day/night cycle time (0-24 hours)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World|Time")
    float WorldTimeHours = 8.0f;

    // Day duration in real seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Time")
    float DayDurationSeconds = 600.0f;

    // Get biome at a world location
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Get weather at a world location
    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    EWorld_WeatherState GetWeatherAtLocation(const FVector& WorldLocation) const;

    // Get temperature at a world location
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    // Get vegetation density at a world location
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    // Get dinosaur spawn weight at a world location
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetDinoSpawnWeightAtLocation(const FVector& WorldLocation) const;

    // Register a biome zone actor
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    void RegisterBiomeZone(AWorld_BiomeZoneActor* BiomeZone);

    // Initialize default 5-biome world layout
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biome")
    void InitializeDefaultBiomes();

    // Advance world time
    UFUNCTION(BlueprintCallable, Category = "World|Time")
    void AdvanceWorldTime(float DeltaSeconds);

    // Get current world time (0-24)
    UFUNCTION(BlueprintPure, Category = "World|Time")
    float GetWorldTimeHours() const { return WorldTimeHours; }

    // Is it daytime?
    UFUNCTION(BlueprintPure, Category = "World|Time")
    bool IsDaytime() const { return WorldTimeHours >= 6.0f && WorldTimeHours < 20.0f; }

private:
    // Find nearest biome zone to a location
    AWorld_BiomeZoneActor* FindNearestBiome(const FVector& WorldLocation) const;

    // Update directional light based on world time
    void UpdateSunPosition(float TimeHours);

    float SunUpdateInterval = 5.0f;
    float TimeSinceLastSunUpdate = 0.0f;
};
