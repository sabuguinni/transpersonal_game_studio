#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Light_DynamicAtmosphereController.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    FLight_AtmosphericSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        SunAngle = 45.0f;
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_DynamicAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    ALight_DynamicAtmosphereController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Lighting References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class APostProcessVolume* PostProcessVolume;

    // Day/Night Cycle Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayDurationMinutes = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDynamicCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bStartAtRandomTime = false;

    // Atmospheric Presets for different times
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericSettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericSettings NightSettings;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RainIntensity = 0.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDayEnum(ELight_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeatherConditions(float NewCloudCoverage, float NewRainIntensity);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void FindAndAssignLightingActors();

    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    float GetSunAngleFromTime(float TimeOfDay) const;

private:
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateSunPosition();
    void UpdateAtmosphericLighting();
    void InterpolateAtmosphericSettings(const FLight_AtmosphericSettings& SettingsA, const FLight_AtmosphericSettings& SettingsB, float Alpha, FLight_AtmosphericSettings& OutSettings);
    void InitializeAtmosphericPresets();
};