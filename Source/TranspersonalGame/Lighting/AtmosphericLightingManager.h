#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "AtmosphericLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherCondition : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Storm       UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct FLight_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereScale = 0.0331f;

    FLight_LightingSettings()
    {
        SunIntensity = 8.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SunAngle = -45.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
        AtmosphereScale = 0.0331f;
    }
};

USTRUCT(BlueprintType)
struct FLight_BiomeLighting
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensityMultiplier = 1.0f;

    FLight_BiomeLighting()
    {
        BiomeName = TEXT("Default");
        AmbientColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
        AmbientIntensity = 2.0f;
        FogDensityMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAtmosphericLightingManager : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphericLightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimePhase = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherCondition CurrentWeather = ELight_WeatherCondition::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_LightingSettings CurrentLightingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    TArray<FLight_BiomeLighting> BiomeLightingSettings;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ASkyAtmosphere* SkyAtmosphere;

public:
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetDayNightCycleEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherCondition(ELight_WeatherCondition NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphericFog();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyBiomeLighting(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureLumenSettings();

    UFUNCTION(BlueprintPure, Category = "Day/Night Cycle")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Day/Night Cycle")
    ELight_TimeOfDay GetCurrentTimePhase() const { return CurrentTimePhase; }

    UFUNCTION(BlueprintPure, Category = "Weather")
    ELight_WeatherCondition GetCurrentWeather() const { return CurrentWeather; }

private:
    void FindLightingActors();
    void UpdateTimePhase();
    void CalculateSunPosition();
    void InterpolateLightingSettings(float Alpha);
    FLight_LightingSettings GetLightingSettingsForTime(ELight_TimeOfDay TimePhase);
    FLight_BiomeLighting* FindBiomeLighting(const FString& BiomeName);
};