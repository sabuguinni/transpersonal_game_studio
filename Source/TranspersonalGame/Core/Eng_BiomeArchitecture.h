#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "Eng_BiomeArchitecture.generated.h"

// ============================================================================
// BIOME ARCHITECTURE SYSTEM - AGENT #2
// Defines the core biome structure and management for the prehistoric world
// ============================================================================

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Swamp,          // SW - Wetlands with dense vegetation and water
    Forest,         // NW - Dense prehistoric forest with tall trees
    Savanna,        // Center - Open grasslands with scattered trees
    Desert,         // E - Arid landscape with sparse vegetation
    Mountains       // NE - Rocky terrain with elevation changes
};

UENUM(BlueprintType)
enum class EEng_BiomeWeather : uint8
{
    Clear,          // Sunny, clear skies
    Overcast,       // Cloudy but no precipitation
    LightRain,      // Light precipitation
    HeavyRain,      // Heavy precipitation with reduced visibility
    Storm,          // Thunderstorm with lightning and heavy rain
    Fog             // Dense fog with very low visibility
};

UENUM(BlueprintType)
enum class EEng_BiomeTimeOfDay : uint8
{
    Dawn,           // Early morning light
    Morning,        // Full daylight
    Midday,         // Peak sun intensity
    Afternoon,      // Late day light
    Dusk,           // Evening twilight
    Night           // Dark with moonlight
};

USTRUCT(BlueprintType)
struct FEng_BiomeProperties
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType BiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector WorldPosition;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector2D BiomeBounds;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float TemperatureRange;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float HumidityLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float VegetationDensity;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    TArray<FString> NativeSpecies;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    TArray<FString> VegetationTypes;

    FEng_BiomeProperties()
    {
        BiomeType = EEng_BiomeType::Savanna;
        BiomeName = TEXT("Unknown Biome");
        WorldPosition = FVector::ZeroVector;
        BiomeBounds = FVector2D(2000.0f, 2000.0f);
        TemperatureRange = 25.0f;
        HumidityLevel = 0.5f;
        VegetationDensity = 0.3f;
        NativeSpecies.Empty();
        VegetationTypes.Empty();
    }
};

USTRUCT(BlueprintType)
struct FEng_BiomeWeatherState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    EEng_BiomeWeather CurrentWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float WeatherIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float WindSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    FVector WindDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float Visibility;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float Temperature;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float Humidity;

    FEng_BiomeWeatherState()
    {
        CurrentWeather = EEng_BiomeWeather::Clear;
        WeatherIntensity = 0.0f;
        WindSpeed = 5.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        Visibility = 1.0f;
        Temperature = 25.0f;
        Humidity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FEng_BiomeLighting
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    EEng_BiomeTimeOfDay TimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    float SunIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    FLinearColor SunColor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    FRotator SunRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    float SkyIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    FLinearColor SkyColor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    float FogDensity;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    FLinearColor FogColor;

    FEng_BiomeLighting()
    {
        TimeOfDay = EEng_BiomeTimeOfDay::Midday;
        SunIntensity = 3.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SunRotation = FRotator(-45.0f, 45.0f, 0.0f);
        SkyIntensity = 1.0f;
        SkyColor = FLinearColor(0.3f, 0.7f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ========================================================================
    // BIOME MANAGEMENT
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void CreateBiome(EEng_BiomeType BiomeType, const FVector& Position, const FVector2D& Bounds);

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    FEng_BiomeProperties GetBiomeProperties(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void SetBiomeProperties(EEng_BiomeType BiomeType, const FEng_BiomeProperties& Properties);

    // ========================================================================
    // WEATHER SYSTEM
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void UpdateWeatherSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void SetWeatherForBiome(EEng_BiomeType BiomeType, EEng_BiomeWeather WeatherType, float Intensity);

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    FEng_BiomeWeatherState GetCurrentWeather(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void TransitionWeather(EEng_BiomeType BiomeType, EEng_BiomeWeather NewWeather, float TransitionTime);

    // ========================================================================
    // DAY/NIGHT CYCLE
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void SetTimeOfDay(EEng_BiomeTimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    EEng_BiomeTimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    float GetDayProgress() const;

    // ========================================================================
    // LIGHTING MANAGEMENT
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void UpdateLightingForBiome(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void SetBiomeLighting(EEng_BiomeType BiomeType, const FEng_BiomeLighting& Lighting);

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    FEng_BiomeLighting GetBiomeLighting(EEng_BiomeType BiomeType) const;

    // ========================================================================
    // ENVIRONMENTAL EFFECTS
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void SpawnEnvironmentalEffects(EEng_BiomeType BiomeType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void UpdateEnvironmentalAmbience(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Architecture")
    void ValidateBiomeSetup();

protected:
    // ========================================================================
    // BIOME DATA
    // ========================================================================

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    TMap<EEng_BiomeType, FEng_BiomeProperties> BiomeProperties;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    TMap<EEng_BiomeType, FEng_BiomeWeatherState> BiomeWeatherStates;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    TMap<EEng_BiomeType, FEng_BiomeLighting> BiomeLightingStates;

    // ========================================================================
    // TIME MANAGEMENT
    // ========================================================================

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    EEng_BiomeTimeOfDay CurrentTimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    float DayProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    float DayDurationMinutes;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    bool bDayNightCycleEnabled;

    // ========================================================================
    // WEATHER MANAGEMENT
    // ========================================================================

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    float WeatherUpdateInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    float WeatherTransitionSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    bool bDynamicWeatherEnabled;

private:
    void InitializeDefaultBiomes();
    void SetupSwampBiome();
    void SetupForestBiome();
    void SetupSavannaBiome();
    void SetupDesertBiome();
    void SetupMountainBiome();
    
    void UpdateSunPosition();
    void UpdateSkyLighting();
    void UpdateFogSettings();
    
    bool IsLocationInBiome(const FVector& Location, EEng_BiomeType BiomeType) const;
    float CalculateDistanceToBiomeCenter(const FVector& Location, EEng_BiomeType BiomeType) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_BiomeComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Component")
    void RegisterWithBiomeManager();

    UFUNCTION(BlueprintPure, Category = "Biome Component")
    EEng_BiomeType GetOwnerBiome() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Component")
    void UpdateBiomeInfluence(float DeltaTime);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Biome Component")
    EEng_BiomeType AssignedBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Component")
    float BiomeInfluenceRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Component")
    bool bAutoRegisterWithManager;

private:
    UPROPERTY()
    AEng_BiomeManager* BiomeManager;
};

#include "Eng_BiomeArchitecture.generated.h"