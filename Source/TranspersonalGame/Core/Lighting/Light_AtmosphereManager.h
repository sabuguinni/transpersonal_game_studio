#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Light_AtmosphereManager.generated.h"

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
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Dust        UMETA(DisplayName = "Dust")
};

USTRUCT(BlueprintType)
struct FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor::Gray;
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Precipitation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Visibility = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor AtmosphericTint = FLinearColor::White;
};

/**
 * Manages dynamic day/night cycle and weather systems for prehistoric survival game
 * Controls Lumen lighting, sky atmosphere, and environmental conditions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULight_AtmosphereManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULight_AtmosphereManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World Subsystem interface
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherType(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetDayNightCycleEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetCycleSpeed(float Speed);

    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_WeatherType GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetDayProgress() const { return DayProgress; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void TransitionToWeather(ELight_WeatherType NewWeather, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetupLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void OptimizeLightingForPerformance();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bDayNightCycleEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CycleSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayProgress = 0.5f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    TMap<ELight_TimeOfDay, FLight_TimeSettings> TimeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    TMap<ELight_WeatherType, FLight_WeatherSettings> WeatherSettings;

    // Lighting actors
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyLight* SkyLightActor;

    UPROPERTY()
    class ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY()
    class AExponentialHeightFog* FogActor;

    // Transition state
    UPROPERTY()
    bool bIsTransitioning = false;

    UPROPERTY()
    float TransitionProgress = 0.0f;

    UPROPERTY()
    float TransitionDuration = 5.0f;

    UPROPERTY()
    ELight_WeatherType TargetWeather;

private:
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateLighting();
    void UpdateWeatherEffects();
    void InitializeTimeSettings();
    void InitializeWeatherSettings();
    void FindOrCreateLightingActors();
    void ApplyTimeSettings(const FLight_TimeSettings& Settings);
    void ApplyWeatherSettings(const FLight_WeatherSettings& Settings);
    ELight_TimeOfDay CalculateTimeOfDayFromProgress(float Progress);
    
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(ULight_AtmosphereManager, STATGROUP_Tickables); }
};