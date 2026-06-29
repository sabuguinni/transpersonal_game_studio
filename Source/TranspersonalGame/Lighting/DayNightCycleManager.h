#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "DayNightCycleManager.generated.h"

// ============================================================
// Day/Night Cycle Manager — Agent #08 Lighting & Atmosphere
// Cycle: PROD_CYCLE_AUTO_20260629_001 | CAP v50
// ============================================================

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Evening     UMETA(DisplayName = "Evening"),
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
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Haze        UMETA(DisplayName = "Haze")
};

USTRUCT(BlueprintType)
struct FLight_SkyPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity;

    FLight_SkyPalette()
        : SunColor(FLinearColor(1.0f, 0.88f, 0.65f, 1.0f))
        , SunIntensity(8.0f)
        , SunPitch(-45.0f)
        , FogColor(FLinearColor(0.6f, 0.75f, 0.9f, 1.0f))
        , FogDensity(0.03f)
        , SkyLightIntensity(1.5f)
    {}
};

UCLASS(ClassGroup = (Lighting), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Time of Day ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeOfDayHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDurationSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bPauseCycle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay;

    // ---- Weather ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed;

    // ---- Scene References ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ASkyLight* SkyLightActor;

    // ---- Palettes ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Palettes")
    FLight_SkyPalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Palettes")
    FLight_SkyPalette NoonPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Palettes")
    FLight_SkyPalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Palettes")
    FLight_SkyPalette NightPalette;

    // ---- Functions ----
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewHours);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetWeather(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetNormalizedTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    bool IsDaytime() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FLinearColor GetCurrentSunColor() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentSunIntensity() const;

    UFUNCTION(CallInEditor, Category = "Day Night Cycle")
    void AutoFindSceneActors();

private:
    void AdvanceTime(float DeltaTime);
    void UpdateSunPosition();
    void UpdateSkyColors();
    void UpdateFogAtmosphere();
    void UpdateSkyLight();
    void ApplyWeatherEffects();
    FLight_SkyPalette InterpolatePalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const;
    ELight_TimeOfDay ClassifyTimeOfDay(float Hours) const;

    float CurrentWeatherBlend;
    FLight_SkyPalette ActivePalette;
};
