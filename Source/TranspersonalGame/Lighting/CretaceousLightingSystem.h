#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/PostProcessVolume.h"
#include "CretaceousLightingSystem.generated.h"

/**
 * Cretaceous era dynamic lighting system.
 * Manages day/night cycle, weather transitions, and atmospheric conditions
 * for the prehistoric survival game world.
 */

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
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
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Hazy        UMETA(DisplayName = "Hazy")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.878f, 0.62f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientBrightness = 1.0f;
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState WeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightingIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionDuration = 30.0f;
};

UCLASS(ClassGroup = (Lighting), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCretaceousAtmosphereComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCretaceousAtmosphereComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 0.003f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bVolumetricFogEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogViewDistance = 6000.0f;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeather(ELight_WeatherState NewWeather, float TransitionTime = 30.0f);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;
    float WeatherTransitionProgress = 1.0f;
    float WeatherTransitionDuration = 30.0f;
    FLight_WeatherSettings TargetWeatherSettings;
    FLight_WeatherSettings CurrentWeatherSettings;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACretaceousLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingSystem();

    // ── Day/Night Cycle ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bDayNightCycleEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDayNormalized = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;

    // ── Light References ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<ASkyLight> SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<AExponentialHeightFog> HeightFogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<APostProcessVolume> PostProcessActor;

    // ── Time of Day Presets ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings NightSettings;

    // ── Weather ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bRandomWeatherEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeIntervalMin = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeIntervalMax = 300.0f;

    // ── Atmosphere Component ─────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCretaceousAtmosphereComponent> AtmosphereComponent;

    // ── Blueprint Callable Functions ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(ELight_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentTimeOfDayNormalized() const { return CurrentTimeOfDayNormalized; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    bool IsNight() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLinearColor GetAmbientLightColor() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting", CallInEditor)
    void ApplyCretaceousLightingNow();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    float WeatherChangeTimer = 0.0f;
    float NextWeatherChangeTime = 180.0f;

    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);
    void ApplySunSettings(const FLight_TimeOfDaySettings& Settings);
    void ApplyFogSettings(const FLight_TimeOfDaySettings& Settings);
    ELight_TimeOfDay ClassifyTimeOfDay(float NormalizedTime) const;
    FLight_TimeOfDaySettings LerpTimeSettings(
        const FLight_TimeOfDaySettings& A,
        const FLight_TimeOfDaySettings& B,
        float Alpha) const;

    void InitDefaultPresets();
};
