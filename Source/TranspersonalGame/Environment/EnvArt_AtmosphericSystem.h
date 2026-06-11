#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/PostProcessVolume.h"
#include "Components/AudioComponent.h"
#include "EnvArt_AtmosphericSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Dusty       UMETA(DisplayName = "Dusty")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "1000", ClampMax = "10000"))
    float SunTemperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float MieScattering = 0.003f;

    FEnvArt_AtmosphericSettings()
    {
        // Default constructor with initialization above
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FEnvArt_AtmosphericSettings CurrentSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    EEnvArt_WeatherState CurrentWeather = EEnvArt_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDayHours = 14.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System", meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float DayDurationSeconds = 1200.0f; // 20 minutes real time = 24 hours game time

    // References to atmospheric actors
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    APostProcessVolume* PostProcessVolume;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetWeatherState(EEnvArt_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetTimeOfDayByHours(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void ApplyAtmosphericSettings(const FEnvArt_AtmosphericSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Atmospheric System")
    float GetCurrentTimeOfDayHours() const { return TimeOfDayHours; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric System")
    EEnvArt_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric System")
    EEnvArt_WeatherState GetCurrentWeather() const { return CurrentWeather; }

protected:
    UFUNCTION()
    void UpdateAtmosphericLighting();

    UFUNCTION()
    void UpdateFogSettings();

    UFUNCTION()
    void UpdateSkySettings();

    UFUNCTION()
    void FindAtmosphericActors();

    UFUNCTION()
    FEnvArt_AtmosphericSettings GetSettingsForTimeOfDay(EEnvArt_TimeOfDay TimeOfDay);

    UFUNCTION()
    FEnvArt_AtmosphericSettings GetSettingsForWeather(EEnvArt_WeatherState Weather);
};