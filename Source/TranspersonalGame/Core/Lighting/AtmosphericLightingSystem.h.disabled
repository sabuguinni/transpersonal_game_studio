#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/ExponentialHeightFog.h"
#include "GameFramework/Actor.h"
#include "UObject/NoExportTypes.h"
#include "AtmosphericLightingSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Storm       UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunElevationAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float RayleighScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float MieScattering = 0.003996f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float MieAbsorption = 0.000444f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.447f, 0.639f, 1.0f, 1.0f);

    FLight_AtmosphericSettings()
    {
        // Cretaceous tropical defaults
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAtmosphericLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphericLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Atmospheric settings for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    TMap<ELight_TimeOfDay, FLight_AtmosphericSettings> TimeOfDaySettings;

    // Weather settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TMap<ELight_WeatherType, FLight_AtmosphericSettings> WeatherSettings;

    // Current state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    float TimeOfDayProgress = 0.5f; // 0.0 = start of period, 1.0 = end of period

    // Day/night cycle settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Cycle")
    float DayDurationMinutes = 20.0f; // Real-time minutes for full day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Cycle")
    float CurrentDayTime = 0.5f; // 0.0 = midnight, 0.5 = noon

    // References to world lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "World References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "World References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "World References")
    class AVolumetricCloud* VolumetricClouds;

    UPROPERTY(BlueprintReadOnly, Category = "World References")
    class AExponentialHeightFog* HeightFog;

public:
    // Main lighting control functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetWeather(ELight_WeatherType NewWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetDayTime(float NewDayTime, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings, float TransitionDuration = 1.0f);

    // Cretaceous-specific presets
    UFUNCTION(BlueprintCallable, Category = "Cretaceous Presets")
    void SetCretaceousTropicalLighting();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Presets")
    void SetCretaceousSwampLighting();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Presets")
    void SetCretaceousDesertLighting();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Presets")
    void SetCretaceousForestLighting();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void FindWorldLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FLight_AtmosphericSettings GetCurrentAtmosphericSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void SaveCurrentSettingsAsPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool LoadPreset(const FString& PresetName);

protected:
    // Internal transition system
    void UpdateDayNightCycle(float DeltaTime);
    void InterpolateAtmosphericSettings(float DeltaTime);
    ELight_TimeOfDay GetTimeOfDayFromDayTime(float DayTime) const;
    FLight_AtmosphericSettings InterpolateSettings(const FLight_AtmosphericSettings& A, const FLight_AtmosphericSettings& B, float Alpha) const;

    // Transition state
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 1.0f;
    FLight_AtmosphericSettings TransitionStartSettings;
    FLight_AtmosphericSettings TransitionTargetSettings;

    // Initialize default settings
    void InitializeDefaultSettings();
    void InitializeCretaceousPresets();
};