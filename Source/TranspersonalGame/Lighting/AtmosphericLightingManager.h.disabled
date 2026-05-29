#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
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

USTRUCT(BlueprintType)
struct FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAzimuth = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float WhiteTemperature = 6200.0f;

    FLight_TimeSettings()
    {
        SunAngle = -45.0f;
        SunAzimuth = 120.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SunIntensity = 3.5f;
        FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
        FogDensity = 0.02f;
        WhiteTemperature = 6200.0f;
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

public:
    virtual void Tick(float DeltaTime) override;

    // Day/Night Cycle Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayDurationMinutes = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    // Lighting References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    APostProcessVolume* PostProcessVolume;

    // Time of Day Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    TMap<ELight_TimeOfDay, FLight_TimeSettings> TimeSettings;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity = 0.7f; // Cretaceous period was more humid

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDayEnum(ELight_TimeOfDay TimeEnum);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeLightingReferences();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdatePostProcessSettings();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherCondition(float NewCloudCoverage, float NewHumidity);

    UFUNCTION(BlueprintCallable, Category = "Lighting", CallInEditor)
    void ApplyCretaceousLighting();

private:
    void InitializeTimeSettings();
    FLight_TimeSettings InterpolateTimeSettings(float TimeOfDay) const;
    void ApplyTimeSettings(const FLight_TimeSettings& Settings);
    
    float LastUpdateTime = 0.0f;
    const float UpdateInterval = 0.1f; // Update every 0.1 seconds
};