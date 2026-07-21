#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/DirectionalLight.h"
#include "Particles/ParticleSystemComponent.h"
#include "AtmosphericManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_AtmosphericState : uint8
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
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float ParticleIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphericManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric State")
    EEnvArt_AtmosphericState CurrentTimeOfDay = EEnvArt_AtmosphericState::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric State")
    EEnvArt_WeatherState CurrentWeather = EEnvArt_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoProgressTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAutoProgressWeather = false;

    // References to world lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TArray<class AActor*> ParticleEmitters;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TArray<class AActor*> WindZones;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(EEnvArt_AtmosphericState NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWeatherState(EEnvArt_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void TransitionToTimeOfDay(EEnvArt_AtmosphericState TargetTimeOfDay, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void TransitionToWeather(EEnvArt_WeatherState TargetWeather, float TransitionDuration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyAtmosphericSettings(const FEnvArt_AtmosphericSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    FEnvArt_AtmosphericSettings GetCurrentAtmosphericSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    EEnvArt_AtmosphericState GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    EEnvArt_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    float GetDayProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Query")
    bool IsStormyWeather() const;

protected:
    UFUNCTION()
    void UpdateLighting();

    UFUNCTION()
    void UpdateFog();

    UFUNCTION()
    void UpdateParticles();

    UFUNCTION()
    void UpdateWind();

    UFUNCTION()
    void FindWorldActors();

    UFUNCTION()
    void ProgressTimeOfDay(float DeltaTime);

    UFUNCTION()
    void ProgressWeather(float DeltaTime);

private:
    float CurrentDayTime = 0.0f;
    float WeatherTransitionTimer = 0.0f;
    bool bTransitioningTime = false;
    bool bTransitioningWeather = false;
    EEnvArt_AtmosphericState TargetTimeOfDay;
    EEnvArt_WeatherState TargetWeatherState;
    float TimeTransitionDuration = 5.0f;
    float WeatherTransitionDuration = 10.0f;
    float TimeTransitionTimer = 0.0f;
};