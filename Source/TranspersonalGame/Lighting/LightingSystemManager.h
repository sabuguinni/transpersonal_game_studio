#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "LightingSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDay
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentHour = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDuration = 1200.0f; // Seconds for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bIsPaused = false;

    FLight_TimeOfDay()
    {
        CurrentHour = 12.0f;
        DayDuration = 1200.0f;
        bIsPaused = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SkyAtmosphereScale = 0.0331f;

    FLight_AtmosphericSettings()
    {
        SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
        SunIntensity = 5.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        SkyAtmosphereScale = 0.0331f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ULightingSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULightingSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core lighting functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyAtmosphericCorrection();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherCondition(EWeatherType WeatherType);

    // Lighting queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    float GetCurrentTimeOfDay() const { return TimeOfDay.CurrentHour; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    bool IsNightTime() const { return TimeOfDay.CurrentHour < 6.0f || TimeOfDay.CurrentHour > 18.0f; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    FLinearColor GetCurrentSunColor() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    float GetCurrentSunIntensity() const;

    // Lighting actor management
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateFogSettings();

protected:
    // Time and atmospheric data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLight_TimeOfDay TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    // Lighting actor references
    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    class ASkyLight* SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    class APostProcessVolume* PostProcessVolume;

    // Internal update functions
    void UpdateSunLightProperties();
    void UpdateSkyLightProperties();
    void UpdateHeightFogProperties();
    void UpdatePostProcessProperties();

    // Time calculation helpers
    float CalculateSunAngle() const;
    FLinearColor CalculateSunColorForTime() const;
    float CalculateSunIntensityForTime() const;
    FLinearColor CalculateFogColorForTime() const;
};