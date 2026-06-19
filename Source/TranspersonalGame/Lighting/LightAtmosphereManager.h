#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/PostProcessVolume.h"
#include "LightAtmosphereManager.generated.h"

// ============================================================
// Enums — global scope (UHT rule)
// ============================================================

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
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Rainy       UMETA(DisplayName = "Rainy")
};

// ============================================================
// Structs — global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.45f, 0.55f, 0.65f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ExposureBias = 1.0f;
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState WeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float SunIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsRaining = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsThundering = false;
};

// ============================================================
// ALightAtmosphereManager — Day/Night Cycle + Weather System
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightAtmosphereManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Day/Night Cycle ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float DayDurationSeconds = 600.0f;  // 10 real minutes = 1 game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float CurrentTimeOfDayNormalized = 0.35f;  // 0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DayNight")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Morning;

    // ---- Weather ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 0.5f;

    // ---- Light References ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AActor* SunActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AActor* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AActor* FogActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    APostProcessVolume* PostProcessVolume = nullptr;

    // ---- Presets ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimeOfDaySettings NightSettings;

    // ---- Functions ----

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentTimeNormalized() const;

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ApplyMiddayPreset();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ApplyDawnPreset();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ApplyDuskPreset();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ApplyNightPreset();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void LogAtmosphereStatus();

private:
    void UpdateSunPosition(float NormalizedTime);
    void UpdateFogSettings(float NormalizedTime);
    void UpdateSkyLight(float NormalizedTime);
    void UpdatePostProcess(float NormalizedTime);
    void ApplyTimeOfDaySettings(const FLight_TimeOfDaySettings& Settings);
    FLight_TimeOfDaySettings LerpTimeSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha);
    ELight_TimeOfDay ClassifyTimeOfDay(float NormalizedTime) const;

    float WeatherTransitionAlpha = 0.0f;
    ELight_WeatherState TargetWeather = ELight_WeatherState::Clear;
};
