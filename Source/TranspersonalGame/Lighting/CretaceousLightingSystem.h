#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "CretaceousLightingSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums & Structs — must be at global scope (UE5 compilation rule)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDayConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ColorTemperature = 5500.0f;
};

USTRUCT(BlueprintType)
struct FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogScattering = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float CloudLayerBottom = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float CloudLayerHeight = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogAlbedo = FLinearColor(0.863f, 0.922f, 0.784f, 1.0f);
};

// ─────────────────────────────────────────────────────────────────────────────
// Main Lighting System Actor
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting System"))
class TRANSPERSONALGAME_API ACretaceousLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Time of Day ──────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float DayDurationSeconds = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float CurrentTimeNormalized = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    bool bEnableDayNightCycle = true;

    // ── Weather ───────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float WeatherTransitionSpeed = 0.5f;

    // ── Configuration ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Config")
    FLight_TimeOfDayConfig AfternoonConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Config")
    FLight_TimeOfDayConfig DawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Config")
    FLight_TimeOfDayConfig NightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Config")
    FLight_AtmosphereSettings AtmosphereSettings;

    // ── Actor References ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    // ── Functions ─────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingConfig(const FLight_TimeOfDayConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentSunPitch() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLinearColor GetCurrentSkyColor() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyAfternoonPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyDawnPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyNightPreset();

private:
    void TickDayNightCycle(float DeltaTime);
    void UpdateSunPosition(float TimeNormalized);
    void UpdateSkyLightIntensity(float TimeNormalized);
    void UpdateFogDensity(float TimeNormalized);
    FLight_TimeOfDayConfig LerpConfigs(const FLight_TimeOfDayConfig& A, const FLight_TimeOfDayConfig& B, float Alpha) const;

    float ElapsedTime = 0.0f;
};
