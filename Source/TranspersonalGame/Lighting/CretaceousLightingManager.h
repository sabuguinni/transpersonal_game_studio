#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/PointLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/PointLightComponent.h"
#include "TimerManager.h"
#include "CretaceousLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn       UMETA(DisplayName = "Dawn"),
    Morning    UMETA(DisplayName = "Morning"),
    Midday     UMETA(DisplayName = "Midday"),
    Afternoon  UMETA(DisplayName = "Afternoon"),
    Dusk       UMETA(DisplayName = "Dusk"),
    Night      UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherCondition : uint8
{
    Clear      UMETA(DisplayName = "Clear"),
    Overcast   UMETA(DisplayName = "Overcast"),
    Foggy      UMETA(DisplayName = "Foggy"),
    Stormy     UMETA(DisplayName = "Stormy"),
    Volcanic   UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieAbsorption = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float OtherAbsorption = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_SunSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float Elevation = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float Azimuth = 135.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float Intensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor Color = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float Temperature = 5800.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_FireLightData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float Intensity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float AttenuationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    FLinearColor Color = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    bool bFlickerEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float FlickerSpeed = 2.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCretaceousLightingManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCretaceousLightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Day/Night Cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 12.0f;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherCondition CurrentWeather = ELight_WeatherCondition::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float WeatherTransitionDurationMinutes = 5.0f;

    // Atmospheric Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphericSettings CretaceousAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<FLight_AtmosphericSettings> WeatherAtmospherePresets;

    // Sun Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLight_SunSettings SunSettings;

    // Fire Lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    TArray<FLight_FireLightData> FireLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    bool bEnableFireFlicker = true;

    // Lighting References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ADirectionalLight* SunActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    AExponentialHeightFog* FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    APostProcessVolume* PostProcessActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TArray<APointLight*> FireLightActors;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherCondition(ELight_WeatherCondition NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void CreateFireLight(const FLight_FireLightData& FireData);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void RemoveAllFireLights();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetArchitecturalLighting(const FVector& Location, float Intensity = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentLightIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLinearColor GetCurrentAmbientColor() const;

private:
    // Internal Methods
    void InitializeLightingReferences();
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherEffects(float DeltaTime);
    void UpdateFireFlicker(float DeltaTime);
    void ApplyWeatherToAtmosphere();
    void CalculateSunAngle(float Hour, float& OutElevation, float& OutAzimuth);
    FLinearColor GetSunColorForTime(float Hour);
    float GetSunIntensityForTime(float Hour);

    // Timer Handles
    FTimerHandle WeatherTransitionTimer;

    // Internal State
    float DayNightCycleTime = 0.0f;
    float WeatherTransitionProgress = 0.0f;
    bool bIsTransitioningWeather = false;
    ELight_WeatherCondition TargetWeather;
    float FireFlickerTime = 0.0f;
};