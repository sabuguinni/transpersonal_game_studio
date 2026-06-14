#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "EnvArt_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.4f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-20.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.8f, 0.9f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EEnvArt_WeatherType CurrentWeather = EEnvArt_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvArt_AtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* AtmosphereFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle")
    bool bEnableDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle", meta = (ClampMin = "0.1", ClampMax = "60.0"))
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "30.0", ClampMax = "600.0"))
    float WeatherTransitionTime = 120.0f;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherType(EEnvArt_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FindAtmosphereActors();

    UFUNCTION(BlueprintCallable, Category = "Effects", CallInEditor)
    void CreateGoldenHourAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Effects", CallInEditor)
    void CreateMistyForestAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Effects", CallInEditor)
    void CreateStormyAtmosphere();

private:
    float CurrentTimeOfDayFloat = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight again
    float WeatherTransitionTimer = 0.0f;
    EEnvArt_WeatherType TargetWeather = EEnvArt_WeatherType::Clear;

    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    FEnvArt_AtmosphereSettings GetSettingsForTimeAndWeather(EEnvArt_TimeOfDay TimeOfDay, EEnvArt_WeatherType Weather);
};