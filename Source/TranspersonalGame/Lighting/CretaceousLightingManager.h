#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "CretaceousLightingManager.generated.h"

// ============================================================
// LIGHTING ENUMS — prefixed Light_ to avoid collision
// ============================================================

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Rain        UMETA(DisplayName = "Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain")
};

// ============================================================
// LIGHTING PRESET STRUCT
// ============================================================

USTRUCT(BlueprintType)
struct FLight_DayPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawDegrees = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.82f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.025f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.72f, 0.58f, 0.42f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;

    FLight_DayPreset() {}
};

// ============================================================
// MAIN LIGHTING MANAGER ACTOR
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting Manager"))
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Day/Night Cycle ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHourOfDay = 14.5f;  // Default: golden hour afternoon

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.1", ClampMax = "120.0"))
    float DayDurationMinutes = 30.0f;  // Real-time minutes per in-game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    // ---- Weather ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherTransitionSpeed = 0.1f;

    // ---- Light References ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<ADirectionalLight> SunActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<ASkyLight> SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<AExponentialHeightFog> FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<APostProcessVolume> PostProcessActor;

    // ---- Presets ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_DayPreset GoldenHourPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_DayPreset MiddayPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_DayPreset NightPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_DayPreset DawnPreset;

    // ---- Blueprint Functions ----

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float HourOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(const FLight_DayPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetTimeOfDayEnum(float Hour) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetSunPitchForHour(float Hour) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLinearColor GetSunColorForHour(float Hour) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void AutoFindLightActors();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ApplyGoldenHourNow();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ApplyMiddayNow();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ApplyNightNow();

private:
    float TimeAccumulator = 0.0f;
    FLight_DayPreset CurrentPreset;
    FLight_DayPreset TargetPreset;
    float PresetBlendAlpha = 1.0f;

    void TickDayNightCycle(float DeltaTime);
    void UpdateSunTransform(float Hour);
    void UpdateFogForWeather(ELight_WeatherState Weather);
    FLight_DayPreset GetPresetForHour(float Hour) const;
    FLight_DayPreset LerpPresets(const FLight_DayPreset& A, const FLight_DayPreset& B, float Alpha) const;
};
