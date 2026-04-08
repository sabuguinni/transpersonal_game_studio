// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataAsset.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Curves/CurveFloat.h"
#include "TimerManager.h"
#include "../WorldGeneration/BiomeSystem.h"
#include "LightingAtmosphereSystem.generated.h"

/**
 * @brief Lighting & Atmosphere System for Transpersonal Game Studio
 * 
 * "Light doesn't illuminate — light means." — Roger Deakins
 * 
 * This system is the emotional director of the game. Every moment of the day tells a story,
 * every weather change shifts the mood, every shadow placement guides the player's feelings.
 * The lighting isn't just technical — it's the invisible narrator that makes the player
 * feel vulnerable at dawn, hopeful at midday, and terrified at dusk.
 * 
 * Core Philosophy (Roger Deakins + RDR2 Lighting Team):
 * - Light serves emotion, not realism
 * - The best lighting is invisible when correct, destructive when wrong
 * - Every time of day has a unique emotional signature
 * - Weather is a character in the story
 * - Shadows are as important as light
 * 
 * Emotional Time Signatures:
 * - Pre-Dawn (04:00-06:00): Vulnerability, uncertainty, cold fear
 * - Dawn (06:00-08:00): Hope, new beginnings, cautious optimism
 * - Morning (08:00-11:00): Activity, clarity, determination
 * - Midday (11:00-14:00): Harsh reality, exposure, no hiding
 * - Afternoon (14:00-17:00): Warmth, comfort, golden hour approach
 * - Golden Hour (17:00-19:00): Beauty, melancholy, time running out
 * - Dusk (19:00-21:00): Danger approaching, last light, urgency
 * - Night (21:00-04:00): Terror, unknown threats, survival mode
 * 
 * Technical Features:
 * - Dynamic time-of-day system with emotional curve mapping
 * - Lumen Global Illumination for realistic bounce lighting
 * - Sky Atmosphere with physically accurate scattering
 * - Volumetric clouds with dynamic weather patterns
 * - Biome-specific lighting characteristics
 * - Interior lighting with narrative mood control
 * - Performance-optimized shadow cascades
 * - Temporal weather system affecting gameplay
 * 
 * @author Lighting & Atmosphere Agent — Agent #8
 * @version 1.0 — March 2026
 */

/** Time periods with distinct emotional characteristics */
UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    PreDawn         UMETA(DisplayName = "Pre-Dawn (04:00-06:00)"),
    Dawn            UMETA(DisplayName = "Dawn (06:00-08:00)"),
    Morning         UMETA(DisplayName = "Morning (08:00-11:00)"),
    Midday          UMETA(DisplayName = "Midday (11:00-14:00)"),
    Afternoon       UMETA(DisplayName = "Afternoon (14:00-17:00)"),
    GoldenHour      UMETA(DisplayName = "Golden Hour (17:00-19:00)"),
    Dusk            UMETA(DisplayName = "Dusk (19:00-21:00)"),
    Night           UMETA(DisplayName = "Night (21:00-04:00)")
};

/** Weather states that affect mood and gameplay */
UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear           UMETA(DisplayName = "Clear Skies"),
    PartlyCloudy    UMETA(DisplayName = "Partly Cloudy"),
    Overcast        UMETA(DisplayName = "Overcast"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Storm           UMETA(DisplayName = "Thunderstorm"),
    Fog             UMETA(DisplayName = "Dense Fog"),
    Mist            UMETA(DisplayName = "Light Mist"),
    Haze            UMETA(DisplayName = "Heat Haze"),
    WindyDust       UMETA(DisplayName = "Windy Dust Storm")
};

/** Emotional mood states driven by lighting */
UENUM(BlueprintType)
enum class EEmotionalMood : uint8
{
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Tense           UMETA(DisplayName = "Tense"),
    Hopeful         UMETA(DisplayName = "Hopeful"),
    Melancholic     UMETA(DisplayName = "Melancholic"),
    Threatening     UMETA(DisplayName = "Threatening"),
    Mysterious      UMETA(DisplayName = "Mysterious"),
    Comfortable     UMETA(DisplayName = "Comfortable"),
    Harsh           UMETA(DisplayName = "Harsh"),
    Dramatic        UMETA(DisplayName = "Dramatic"),
    Serene          UMETA(DisplayName = "Serene")
};

/** Interior lighting mood presets */
UENUM(BlueprintType)
enum class EInteriorMood : uint8
{
    Abandoned       UMETA(DisplayName = "Long Abandoned"),
    RecentlyLeft    UMETA(DisplayName = "Recently Abandoned"),
    Lived           UMETA(DisplayName = "Recently Lived-In"),
    Sacred          UMETA(DisplayName = "Sacred/Ritual Space"),
    Threatening     UMETA(DisplayName = "Threatening/Dangerous"),
    Shelter         UMETA(DisplayName = "Safe Shelter"),
    Storage         UMETA(DisplayName = "Storage/Cache"),
    Workshop        UMETA(DisplayName = "Work Area"),
    Sleeping        UMETA(DisplayName = "Sleeping Area"),
    Gathering       UMETA(DisplayName = "Gathering Place")
};

/** Time of day lighting configuration */
USTRUCT(BlueprintType)
struct FTimeOfDayConfig
{
    GENERATED_BODY()

    /** Time period this config represents */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
    ETimeOfDay TimePeriod = ETimeOfDay::Morning;

    /** Start time (24-hour format) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float StartTime = 8.0f;

    /** End time (24-hour format) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float EndTime = 11.0f;

    /** Sun elevation angle (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sun", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
    float SunElevation = 45.0f;

    /** Sun azimuth angle (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sun", meta = (ClampMin = "0.0", ClampMax = "360.0"))
    float SunAzimuth = 180.0f;

    /** Sun light intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sun", meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float SunIntensity = 5.0f;

    /** Sun light color */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    /** Sun disk size (angular diameter) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sun", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float SunDiskSize = 0.5f;

    /** Sky light intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sky", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float SkyLightIntensity = 1.0f;

    /** Sky light color tint */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sky")
    FLinearColor SkyLightTint = FLinearColor::White;

    /** Atmosphere density */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float AtmosphereDensity = 1.0f;

    /** Fog density */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FogDensity = 0.1f;

    /** Fog color */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    /** Cloud coverage (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clouds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CloudCoverage = 0.3f;

    /** Cloud density */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clouds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CloudDensity = 0.5f;

    /** Emotional mood this time evokes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emotion")
    EEmotionalMood PrimaryMood = EEmotionalMood::Peaceful;

    /** Secondary emotional undertone */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emotion")
    EEmotionalMood SecondaryMood = EEmotionalMood::Comfortable;

    /** Tension level (0-1, affects shadow sharpness and contrast) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TensionLevel = 0.3f;

    /** Visibility range for dinosaurs (affects AI behavior) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float DinosaurVisibilityRange = 1000.0f;

    /** Player stealth bonus (0-1, higher = easier to hide) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StealthBonus = 0.0f;
};

/** Weather pattern configuration */
USTRUCT(BlueprintType)
struct FWeatherConfig
{
    GENERATED_BODY()

    /** Weather state */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    EWeatherState WeatherType = EWeatherState::Clear;

    /** Duration range (minutes) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    FVector2D DurationRange = FVector2D(30.0f, 120.0f);

    /** Probability of occurrence (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Probability = 0.3f;

    /** Transition time to this weather (minutes) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float TransitionTime = 10.0f;

    /** Sun intensity modifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float SunIntensityMultiplier = 1.0f;

    /** Sky light intensity modifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float SkyLightIntensityMultiplier = 1.0f;

    /** Fog density modifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float FogDensityMultiplier = 1.0f;

    /** Cloud coverage override */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CloudCoverageOverride = -1.0f; // -1 means use time-of-day default

    /** Wind intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float WindIntensity = 1.0f;

    /** Temperature modifier (Celsius) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float TemperatureModifier = 0.0f;

    /** Visibility range modifier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float VisibilityMultiplier = 1.0f;

    /** Affects dinosaur behavior */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bAffectsDinosaurBehavior = false;

    /** Makes dinosaurs seek shelter */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bCausesSeekShelter = false;

    /** Provides stealth bonus to player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StealthBonus = 0.0f;
};

/** Interior lighting configuration */
USTRUCT(BlueprintType)
struct FInteriorLightingConfig
{
    GENERATED_BODY()

    /** Interior mood type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mood")
    EInteriorMood MoodType = EInteriorMood::Shelter;

    /** Base light intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float BaseIntensity = 1.0f;

    /** Light color tint */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting")
    FLinearColor LightTint = FLinearColor::White;

    /** Shadow sharpness (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ShadowSharpness = 0.5f;

    /** Ambient occlusion strength */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float AmbientOcclusionStrength = 1.0f;

    /** Dust particle density */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DustParticleDensity = 0.1f;

    /** Light shaft intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float LightShaftIntensity = 0.3f;

    /** Uses fire light simulation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special")
    bool bUseFireLight = false;

    /** Fire light flicker intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FireFlickerIntensity = 0.5f;

    /** Emotional impact on player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emotion")
    EEmotionalMood EmotionalImpact = EEmotionalMood::Comfortable;

    /** Safety feeling level (affects gameplay) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SafetyLevel = 0.5f;
};

/** Biome-specific lighting characteristics */
USTRUCT(BlueprintType)
struct FBiomeLightingProfile
{
    GENERATED_BODY()

    /** Biome this profile applies to */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::TemperateForest;

    /** Base atmosphere color tint */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    FLinearColor AtmosphereTint = FLinearColor::White;

    /** Fog color tint */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    FLinearColor FogTint = FLinearColor::White;

    /** Vegetation light filtering */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationLightFiltering = 0.3f;

    /** Humidity effect on atmosphere */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HumidityAtmosphereEffect = 0.5f;

    /** Preferred weather patterns */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    TArray<EWeatherState> PreferredWeatherPatterns;

    /** Temperature range (Celsius) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    FVector2D TemperatureRange = FVector2D(15.0f, 25.0f);

    /** Seasonal variation intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SeasonalVariation = 0.3f;
};

/**
 * @brief Main lighting and atmosphere system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULightingAtmosphereSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    ULightingAtmosphereSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Initialize the lighting system for the current level */
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void InitializeLightingSystem();

    /** Update lighting based on current time and weather */
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void UpdateLighting(float DeltaTime);

    /** Set current time of day (0-24 hours) */
    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void SetTimeOfDay(float Hours);

    /** Get current time of day */
    UFUNCTION(BlueprintPure, Category = "Time of Day")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    /** Get current time period */
    UFUNCTION(BlueprintPure, Category = "Time of Day")
    ETimeOfDay GetCurrentTimePeriod() const;

    /** Set weather state */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(EWeatherState NewWeather, float TransitionTime = 10.0f);

    /** Get current weather state */
    UFUNCTION(BlueprintPure, Category = "Weather")
    EWeatherState GetCurrentWeather() const { return CurrentWeatherState; }

    /** Force weather transition */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TriggerWeatherTransition();

    /** Set interior lighting mood */
    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void SetInteriorMood(EInteriorMood Mood, float TransitionTime = 2.0f);

    /** Get current emotional mood */
    UFUNCTION(BlueprintPure, Category = "Emotion")
    EEmotionalMood GetCurrentEmotionalMood() const;

    /** Get current tension level */
    UFUNCTION(BlueprintPure, Category = "Emotion")
    float GetCurrentTensionLevel() const;

    /** Get current visibility range for gameplay */
    UFUNCTION(BlueprintPure, Category = "Gameplay")
    float GetCurrentVisibilityRange() const;

    /** Get current stealth bonus */
    UFUNCTION(BlueprintPure, Category = "Gameplay")
    float GetCurrentStealthBonus() const;

    /** Set biome lighting profile */
    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    void SetBiomeLightingProfile(EBiomeType BiomeType);

    /** Enable/disable automatic time progression */
    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void SetTimeProgression(bool bEnabled, float TimeScale = 1.0f);

    /** Enable/disable dynamic weather */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetDynamicWeather(bool bEnabled);

protected:
    /** Current time of day (0-24 hours) */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentTimeOfDay = 12.0f;

    /** Current weather state */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EWeatherState CurrentWeatherState = EWeatherState::Clear;

    /** Current biome type */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EBiomeType CurrentBiomeType = EBiomeType::TemperateForest;

    /** Time progression enabled */
    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    bool bTimeProgressionEnabled = true;

    /** Time scale multiplier */
    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    float TimeScale = 1.0f;

    /** Dynamic weather enabled */
    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    bool bDynamicWeatherEnabled = true;

    /** Time of day configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FTimeOfDayConfig> TimeOfDayConfigs;

    /** Weather configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FWeatherConfig> WeatherConfigs;

    /** Interior lighting configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FInteriorLightingConfig> InteriorLightingConfigs;

    /** Biome lighting profiles */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FBiomeLightingProfile> BiomeLightingProfiles;

    /** Material parameter collection for global lighting parameters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialParameterCollection> GlobalLightingParameters;

    /** Time of day transition curve */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curves")
    TSoftObjectPtr<UCurveFloat> TimeTransitionCurve;

    /** Weather transition curve */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curves")
    TSoftObjectPtr<UCurveFloat> WeatherTransitionCurve;

private:
    /** Scene lighting components */
    UPROPERTY()
    TWeakObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY()
    TWeakObjectPtr<ASkyLight> SkyLight;

    UPROPERTY()
    TWeakObjectPtr<APostProcessVolume> PostProcessVolume;

    UPROPERTY()
    TWeakObjectPtr<AExponentialHeightFog> HeightFog;

    /** Component references */
    UPROPERTY()
    TWeakObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

    UPROPERTY()
    TWeakObjectPtr<UVolumetricCloudComponent> VolumetricClouds;

    /** Weather transition timer */
    FTimerHandle WeatherTransitionTimer;

    /** Current weather transition progress */
    float WeatherTransitionProgress = 0.0f;

    /** Target weather state */
    EWeatherState TargetWeatherState = EWeatherState::Clear;

    /** Weather transition duration */
    float WeatherTransitionDuration = 10.0f;

    /** Last weather change time */
    float LastWeatherChangeTime = 0.0f;

    /** Initialize default configurations */
    void InitializeDefaultConfigurations();

    /** Find scene lighting components */
    void FindSceneLightingComponents();

    /** Apply time of day lighting */
    void ApplyTimeOfDayLighting(const FTimeOfDayConfig& Config, float BlendWeight = 1.0f);

    /** Apply weather effects */
    void ApplyWeatherEffects(const FWeatherConfig& Config, float BlendWeight = 1.0f);

    /** Apply biome lighting characteristics */
    void ApplyBiomeLighting(const FBiomeLightingProfile& Profile);

    /** Update weather transition */
    void UpdateWeatherTransition(float DeltaTime);

    /** Calculate sun position based on time */
    FRotator CalculateSunPosition(float TimeOfDay) const;

    /** Update material parameter collection */
    void UpdateGlobalLightingParameters();

    /** Determine if weather should change */
    bool ShouldChangeWeather() const;

    /** Select next weather state */
    EWeatherState SelectNextWeatherState() const;
};

/**
 * @brief Data asset for lighting and atmosphere configuration
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ULightingAtmosphereDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Time of day configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time of Day")
    TArray<FTimeOfDayConfig> TimeOfDayConfigs;

    /** Weather configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    TArray<FWeatherConfig> WeatherConfigs;

    /** Interior lighting configurations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
    TArray<FInteriorLightingConfig> InteriorConfigs;

    /** Biome lighting profiles */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biomes")
    TArray<FBiomeLightingProfile> BiomeProfiles;

    /** Global lighting settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global")
    TSoftObjectPtr<UMaterialParameterCollection> GlobalParameters;
};