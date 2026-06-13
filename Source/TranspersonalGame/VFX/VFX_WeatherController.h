#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "VFX_ParticleManager.h"
#include "VFX_WeatherController.generated.h"

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Fog             UMETA(DisplayName = "Fog"),
    Dust            UMETA(DisplayName = "Dust Storm"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),
    Snow            UMETA(DisplayName = "Snow")
};

USTRUCT(BlueprintType)
struct FVFX_WeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EVFX_WeatherType WeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Visibility = 1.0f;

    FVFX_WeatherData()
    {
        WeatherType = EVFX_WeatherType::Clear;
        Intensity = 1.0f;
        Duration = 60.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        Temperature = 20.0f;
        Visibility = 1.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AVFX_WeatherController : public AActor
{
    GENERATED_BODY()

public:
    AVFX_WeatherController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UVFX_ParticleManager* ParticleManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FVFX_WeatherData CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    TArray<FVFX_WeatherData> WeatherQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bAutoWeatherCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float WeatherTransitionTime = 10.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Runtime")
    float CurrentWeatherTimer = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Runtime")
    bool bWeatherTransitioning = false;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EVFX_WeatherType WeatherType, float Intensity = 1.0f, float Duration = 60.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartWeatherTransition(const FVFX_WeatherData& NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void QueueWeather(const FVFX_WeatherData& WeatherData);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ClearWeatherQueue();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    FVFX_WeatherData GetCurrentWeather() const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartRainWeather(float Intensity = 1.0f, float Duration = 120.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartFogWeather(float Density = 0.5f, float Duration = 180.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartDustStorm(float Intensity = 1.0f, FVector WindDirection = FVector(1.0f, 0.0f, 0.0f));

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartVolcanicAshfall(float Intensity = 0.7f, float Duration = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ClearWeather();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    bool IsWeatherActive() const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    float GetWeatherIntensity() const;

private:
    void UpdateWeatherEffects();
    void ProcessWeatherQueue();
    void SpawnWeatherParticles();
    void UpdateEnvironmentalEffects();
};