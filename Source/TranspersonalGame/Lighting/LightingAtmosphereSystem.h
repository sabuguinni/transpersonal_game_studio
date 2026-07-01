#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "LightingAtmosphereSystem.generated.h"

// ============================================================
// Enums — Light_* prefix (RULE 2: unique names across project)
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
    Stormy      UMETA(DisplayName = "Stormy Overcast")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyStorm  UMETA(DisplayName = "Heavy Storm"),
    Fog         UMETA(DisplayName = "Dense Fog")
};

// ============================================================
// Structs — Light_* prefix
// ============================================================

USTRUCT(BlueprintType)
struct FLight_AtmospherePalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunPitchDegrees = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunYawDegrees = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor FogColor = FLinearColor(0.5f, 0.55f, 0.65f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SkyLightColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct FLight_WeatherTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState FromState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState ToState = ELight_WeatherState::HeavyStorm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float TransitionDurationSeconds = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float CurrentAlpha = 0.0f;
};

// ============================================================
// ALightingAtmosphereSystem — main lighting manager actor
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Lighting Atmosphere System"))
class TRANSPERSONALGAME_API ALightingAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ---- Time of Day ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    bool bDynamicDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay",
        meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 12.0f;

    // ---- Weather ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    FLight_WeatherTransition ActiveTransition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    bool bWeatherTransitionActive = false;

    // ---- Scene References ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SceneSkyLight = nullptr;

    // ---- Palette Presets ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette MorningPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette NightPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette StormyPalette;

    // ---- Functions ----
    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    void AdvanceTime(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Weather")
    void TriggerWeatherTransition(ELight_WeatherState NewWeather, float DurationSeconds);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Weather")
    void ApplyWeatherState(ELight_WeatherState WeatherState);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Palette")
    void ApplyPalette(const FLight_AtmospherePalette& Palette);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Palette")
    FLight_AtmospherePalette GetPaletteForTime(ELight_TimeOfDay TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|Palette")
    FLight_AtmospherePalette LerpPalettes(
        const FLight_AtmospherePalette& A,
        const FLight_AtmospherePalette& B,
        float Alpha) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|Debug")
    FString GetLightingStatus() const;

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ForceApplyCurrentPalette();

private:
    void InitializeDefaultPalettes();
    void AutoDiscoverLightActors();
    void UpdateDayNightCycle(float DeltaSeconds);
    void UpdateWeatherTransition(float DeltaSeconds);
    ELight_TimeOfDay HourToTimeOfDay(float Hour) const;

    float TimeAccumulator = 0.0f;
};
