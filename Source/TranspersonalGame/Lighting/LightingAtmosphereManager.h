#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "LightingAtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Rain        UMETA(DisplayName = "Rain")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void AdvanceTime(float DeltaHours);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AActor* SkyAtmosphereActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Settings")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Settings")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Settings", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDaySettings DawnPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDaySettings NoonPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDaySettings DuskPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDaySettings NightPreset;

private:
    UPROPERTY()
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Noon;

    UPROPERTY()
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    float TimeAccumulator = 0.0f;

    void UpdateSunPosition();
    void UpdateFogForWeather();
    FLight_TimeOfDaySettings InterpolatePresets(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha);
    ELight_TimeOfDay HourToTimeOfDay(float Hour) const;
};
