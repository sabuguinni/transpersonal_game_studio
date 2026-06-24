#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "CretaceousLightingManager.generated.h"

// ============================================================
// Lighting preset for different times of day
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
    Night       UMETA(DisplayName = "Night")
};

// ============================================================
// Weather state for atmosphere variation
// ============================================================
UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Hazy        UMETA(DisplayName = "Hazy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

// ============================================================
// Lighting preset data
// ============================================================
USTRUCT(BlueprintType)
struct FLight_TimePreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.45f, 0.62f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ExposureBias = 1.0f;
};

// ============================================================
// ACretaceousLightingManager — manages dynamic day/night cycle
// and weather transitions for the Cretaceous world
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
    float CurrentTimeOfDay = 10.0f; // Hours (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.1", ClampMax = "100.0"))
    float DayDurationMinutes = 20.0f; // Real-time minutes per full day

    UPROPERTY(BlueprintReadOnly, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimePreset = ELight_TimeOfDay::Morning;

    // ---- Weather ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherTransitionSpeed = 0.1f;

    // ---- Light References ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<AActor> SunActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<AActor> SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<AActor> FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TObjectPtr<AActor> PostProcessActor;

    // ---- Time Presets ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimePreset DawnPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimePreset MiddayPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimePreset GoldenHourPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimePreset NightPreset;

    // ---- Blueprint-callable functions ----
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewHour);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(const FLight_TimePreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_TimePreset GetCurrentPreset() const;

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetNormalizedTimeOfDay() const;

    UFUNCTION(BlueprintPure, Category = "Lighting")
    bool IsNighttime() const;

    UFUNCTION(CallInEditor, Category = "Lighting")
    void AutoFindLightActors();

private:
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateSunPosition();
    void UpdateFogForWeather();
    FLight_TimePreset InterpolatePresets(const FLight_TimePreset& A, const FLight_TimePreset& B, float Alpha) const;
    ELight_TimeOfDay GetTimePresetForHour(float Hour) const;

    float SecondsPerGameDay = 0.0f;
    float AccumulatedTime = 0.0f;
};
