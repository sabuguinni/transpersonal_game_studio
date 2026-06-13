#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Light_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Dust        UMETA(DisplayName = "Dust Storm")
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
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogColor = FLinearColor(0.7f, 0.63f, 0.47f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;
};

UCLASS(Blueprintable)
class TRANSPERSONALGAME_API ALight_WeatherSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_WeatherSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTime = 12.0f; // 24-hour format

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    TMap<ELight_WeatherType, FLight_WeatherSettings> WeatherPresets;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeOfDay(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeather(ELight_WeatherType TargetWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Time System")
    ELight_TimeOfDay GetTimeOfDayFromHour(float Hour) const;

private:
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyLight* SkyLightActor;

    UPROPERTY()
    class AExponentialHeightFog* FogActor;

    void InitializeWeatherPresets();
    void FindLightingActors();
    void UpdateSunPosition();
    void UpdateAtmosphericFog();
    void UpdateSkyLight();

    FLight_WeatherSettings CurrentSettings;
    FLight_WeatherSettings TargetSettings;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 5.0f;
    bool bIsTransitioning = false;
};

#include "Light_WeatherSystem.generated.h"