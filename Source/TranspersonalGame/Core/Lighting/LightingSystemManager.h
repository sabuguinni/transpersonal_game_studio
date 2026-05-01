#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Subsystems/WorldSubsystem.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Sunset      UMETA(DisplayName = "Sunset"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FLight_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.2f, 0.4f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogStartDistance = 500.0f;

    FLight_LightingSettings()
    {
        SunColor = FLinearColor::White;
        SunIntensity = 3.0f;
        SunAngle = 45.0f;
        SkyColor = FLinearColor(0.2f, 0.4f, 0.8f, 1.0f);
        SkyIntensity = 1.0f;
        FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
        FogDensity = 0.02f;
        FogStartDistance = 500.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ULightingSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULightingSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World Subsystem interface
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherType(ELight_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingSettings(const FLight_LightingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_LightingSettings GetCurrentLightingSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentTimeOfDayProgress() const { return TimeOfDayProgress; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_WeatherType GetCurrentWeatherType() const { return CurrentWeatherType; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ELight_WeatherType CurrentWeatherType = ELight_WeatherType::Clear;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    float TimeOfDayProgress = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    bool bDayNightCycleEnabled = true;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    float DayNightCycleSpeed = 1.0f;

private:
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    ASkyLight* SkyLightActor;

    UPROPERTY()
    ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY()
    AExponentialHeightFog* FogActor;

    void FindOrCreateLightingActors();
    void UpdateSunLighting();
    void UpdateSkyLighting();
    void UpdateFogSettings();
    void UpdateAtmosphere();
    FLight_LightingSettings GetLightingSettingsForTimeOfDay(ELight_TimeOfDay TimeOfDay) const;
    FLight_LightingSettings GetLightingSettingsForWeather(ELight_WeatherType WeatherType) const;
    FLight_LightingSettings BlendLightingSettings(const FLight_LightingSettings& A, const FLight_LightingSettings& B, float Alpha) const;
};