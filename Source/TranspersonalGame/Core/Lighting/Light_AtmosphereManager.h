#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Light_AtmosphereManager.generated.h"

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
    Hazy        UMETA(DisplayName = "Hazy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 5800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AerosolDensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FVector AerosolAbsorption = FVector(0.15f, 0.25f, 0.4f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(1.0f, 0.92f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
    FLinearColor SkyLightColor = FLinearColor(0.96f, 0.92f, 0.84f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle")
    float DayDurationMinutes = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereSettings ClearDaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereSettings HazyDaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereSettings NightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereSettings VolcanicSettings;

private:
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    ASkyLight* SkyLightActor;

    UPROPERTY()
    AExponentialHeightFog* FogActor;

    UPROPERTY()
    ASkyAtmosphere* AtmosphereActor;

    UPROPERTY()
    UDirectionalLightComponent* SunLightComponent;

    UPROPERTY()
    USkyLightComponent* SkyLightComponent;

    UPROPERTY()
    UExponentialHeightFogComponent* FogComponent;

    UPROPERTY()
    USkyAtmosphereComponent* AtmosphereComponent;

    float WeatherBlendAlpha = 0.0f;
    ELight_WeatherType TargetWeather = ELight_WeatherType::Clear;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FLight_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void BlendWeatherSettings(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FLight_AtmosphereSettings GetCurrentWeatherSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ForceRefreshAtmosphere();

private:
    void FindOrCreateAtmosphereActors();
    void SetupCretaceousDefaults();
    FRotator CalculateSunRotation(float TimeHours) const;
    FLight_AtmosphereSettings LerpAtmosphereSettings(const FLight_AtmosphereSettings& A, const FLight_AtmosphereSettings& B, float Alpha) const;
};