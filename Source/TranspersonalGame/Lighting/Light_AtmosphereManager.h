#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Light_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Storm       UMETA(DisplayName = "Storm")
};

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
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float StartDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    FLight_AtmosphericSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        SunTemperature = 5500.0f;
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        StartDistance = 5000.0f;
        bVolumetricFog = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class APostProcessVolume* PostProcessVolume;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherState(ELight_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeAtmosphericComponents();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void TransitionToWeather(ELight_WeatherState TargetWeather, float TransitionDuration = 5.0f);

private:
    void FindAtmosphericActors();
    void ConfigureLumenSettings();
    void UpdateFogSettings();
    void UpdateSunLighting();
    
    float WeatherTransitionTimer = 0.0f;
    ELight_WeatherState TargetWeatherState;
    bool bIsTransitioning = false;
};