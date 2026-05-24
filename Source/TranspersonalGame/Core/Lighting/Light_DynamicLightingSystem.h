#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Curves/CurveFloat.h"
#include "../SharedTypes.h"
#include "Light_DynamicLightingSystem.generated.h"

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.2f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType WeatherType = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindStrength = 1.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_DynamicLightingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_DynamicLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Time of Day System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeSpeed = 1.0f; // Minutes per real second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLight_TimeOfDaySettings CurrentTimeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoProgressTime = true;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLight_WeatherSettings CurrentWeatherSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    // Light References
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ASkyLight* SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    AExponentialHeightFog* FogActor;

    // Curves for time-based lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
    UCurveFloat* SunIntensityCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
    UCurveFloat* SunElevationCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
    UCurveFloat* FogDensityCurve;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(EWeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void FindLightingActors();

private:
    void UpdateSunPosition();
    void UpdateSunLighting();
    void UpdateSkyLighting();
    void UpdateFogSettings();
    void UpdateWeatherEffects();
    
    float CalculateSunElevation(float TimeOfDay);
    FLinearColor GetSunColorForTime(float TimeOfDay);
    FLinearColor GetSkyColorForTime(float TimeOfDay);
    
    // Internal state
    float PreviousTimeOfDay = 0.0f;
    EWeatherType PreviousWeatherType = EWeatherType::Clear;
    bool bLightingActorsFound = false;
};