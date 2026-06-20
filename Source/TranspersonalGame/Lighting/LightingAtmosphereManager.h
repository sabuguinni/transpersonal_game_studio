#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "LightingAtmosphereManager.generated.h"

// ============================================================
// Enums — Light_* prefix to avoid conflicts
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
    MidNight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Hazy        UMETA(DisplayName = "Hazy")
};

// ============================================================
// Structs — Light_* prefix
// ============================================================

USTRUCT(BlueprintType)
struct FLight_TimeOfDayConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float SunPitchDegrees = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float SunYawDegrees = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float SunIntensityLux = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    FLinearColor SkyLightColor = FLinearColor(0.85f, 0.92f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct FLight_FogConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.6f, 0.75f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogMaxOpacity = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float StartDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float VolumetricFogViewDistance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float VolumetricFogExtinctionScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FLight_RuinSiteLight
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Ruins")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Ruins")
    FLinearColor LightColor = FLinearColor(1.0f, 0.7f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Ruins")
    float Intensity = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Ruins")
    float AttenuationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Ruins")
    FString Label = TEXT("RuinLight");
};

// ============================================================
// ALightingAtmosphereManager — main lighting controller
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Lighting Atmosphere Manager"))
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

    // ---- Time of Day ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    FLight_TimeOfDayConfig CurrentTimeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    bool bEnableDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time", meta = (ClampMin = "1.0", ClampMax = "3600.0"))
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float CurrentDayTimeNormalized = 0.6f;

    // ---- Fog ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLight_FogConfig FogConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    // ---- Ruin Site ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Ruins")
    TArray<FLight_RuinSiteLight> RuinSiteLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Ruins")
    FVector RuinSiteCenter = FVector(50000.0f, 50000.0f, 100.0f);

    // ---- References ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> SunActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> SkyLightActor;

    // ---- Functions ----

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyTimeOfDay(ELight_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyFogConfig(const FLight_FogConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunAngle(float PitchDegrees, float YawDegrees);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunIntensity(float Lux);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_TimeOfDayConfig GetTimeConfigForHour(float NormalizedTime) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentDayTimeNormalized() const { return CurrentDayTimeNormalized; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void RebuildLightingStack();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ValidateLightingInvariants();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void AdvanceDayNightCycle(float DeltaTime);
    void UpdateSunFromNormalizedTime(float NormalizedTime);
    FLinearColor LerpLinearColor(const FLinearColor& A, const FLinearColor& B, float Alpha) const;

private:
    float DayTimeAccumulator = 0.0f;
};
