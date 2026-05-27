#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/World.h"
#include "LightingSystemManager.generated.h"

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
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float Temperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float MieScattering = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float AtmosphericDensity = 12.0f;

    FLight_AtmosphericSettings()
    {
        SunIntensity = 3.5f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        Temperature = 5500.0f;
        RayleighScattering = 0.8f;
        MieScattering = 0.6f;
        AtmosphericDensity = 12.0f;
    }
};

USTRUCT(BlueprintType)
struct FLight_DayNightCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDurationMinutes = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bAutoAdvanceTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeMultiplier = 1.0f;

    FLight_DayNightCycle()
    {
        DayDurationMinutes = 24.0f;
        CurrentTimeOfDay = 12.0f;
        bAutoAdvanceTime = true;
        TimeMultiplier = 1.0f;
    }
};

/**
 * Lighting System Manager for Cretaceous period atmospheric lighting
 * Manages day/night cycle, weather systems, and atmospheric effects
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === LIGHTING COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UDirectionalLightComponent* SunLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class USkyAtmosphereComponent* AtmosphereComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UVolumetricCloudComponent* CloudComponent;

    // === ATMOSPHERIC SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLight_AtmosphericSettings CretaceousSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    FLight_DayNightCycle DayNightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

public:
    // === LIGHTING CONTROL FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float TimeInHours);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetWeatherType(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateSunPosition(float TimeInHours);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateAtmosphericScattering(ELight_WeatherType Weather);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ConfigureLumenSettings();

    // === GETTER FUNCTIONS ===
    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    float GetSunIntensity() const;

    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    FLinearColor GetSunColor() const;

    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    bool IsNightTime() const;

private:
    // === INTERNAL FUNCTIONS ===
    void InitializeLightingComponents();
    void UpdateDayNightCycle(float DeltaTime);
    void InterpolateLightingSettings(float TimeInHours);
    void ApplyWeatherEffects();

    // === CACHED REFERENCES ===
    UPROPERTY()
    class ADirectionalLight* CachedDirectionalLight;

    UPROPERTY()
    class ASkyAtmosphere* CachedSkyAtmosphere;

    UPROPERTY()
    class AVolumetricCloud* CachedVolumetricCloud;
};