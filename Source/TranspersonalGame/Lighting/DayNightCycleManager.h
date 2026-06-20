#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "DayNightCycleManager.generated.h"

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
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Haze        UMETA(DisplayName = "Haze")
};

USTRUCT(BlueprintType)
struct FLight_SunSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Intensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor LightColor = FLinearColor(1.0f, 0.92f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunYaw = 45.0f;
};

USTRUCT(BlueprintType)
struct FLight_FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogMaxOpacity = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float VolumetricFogDistance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLinearColor FogAlbedo = FLinearColor(0.85f, 0.88f, 0.92f, 1.0f);
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDayPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    FLight_SunSettings SunSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    FLight_FogSettings FogSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    float TransitionDuration = 300.0f;
};

/**
 * ADayNightCycleManager
 * Manages the full day/night cycle for the Cretaceous prehistoric world.
 * Controls DirectionalLight, ExponentialHeightFog, SkyLight, and SkyAtmosphere
 * to create cinematically accurate prehistoric atmosphere.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Current State ───────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|State",
        meta = (AllowPrivateAccess = "true"))
    ELight_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|State",
        meta = (AllowPrivateAccess = "true"))
    ELight_WeatherState CurrentWeather;

    /** Current time of day as 0.0–24.0 hours */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    float CurrentHour = 10.0f;

    /** How many real seconds = 1 in-game hour */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    float SecondsPerHour = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    bool bCycleEnabled = true;

    // ─── References ──────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ASkyLight> SkyLightActor;

    // ─── Presets ─────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    TArray<FLight_TimeOfDayPreset> TimeOfDayPresets;

    // ─── Blueprint Functions ──────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Lighting|Cycle")
    void SetTimeOfDay(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Cycle")
    void SetWeather(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Cycle")
    ELight_TimeOfDay GetTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|Cycle")
    float GetCurrentHour() const { return CurrentHour; }

    UFUNCTION(BlueprintCallable, Category = "Lighting|Cycle")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|Cycle")
    bool IsDawnOrDusk() const;

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyCurrentSettings();

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void AutoFindLightActors();

private:
    void UpdateSunPosition(float Hour);
    void UpdateFogForTimeOfDay(float Hour);
    void UpdateSkyLightForTimeOfDay(float Hour);
    void ApplyWeatherOverlay();

    FLight_SunSettings InterpolateSunSettings(const FLight_SunSettings& A, const FLight_SunSettings& B, float Alpha) const;
    FLight_FogSettings InterpolateFogSettings(const FLight_FogSettings& A, const FLight_FogSettings& B, float Alpha) const;

    FLight_SunSettings GetSunSettingsForHour(float Hour) const;
    FLight_FogSettings GetFogSettingsForHour(float Hour) const;

    float ElapsedSeconds = 0.0f;
};
