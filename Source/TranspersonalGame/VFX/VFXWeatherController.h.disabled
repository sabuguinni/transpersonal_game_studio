#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFXWeatherController.generated.h"

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Thunder Storm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Sandstorm   UMETA(DisplayName = "Sand Storm"),
    Snow        UMETA(DisplayName = "Snow Fall")
};

USTRUCT(BlueprintType)
struct FVFX_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    EVFX_WeatherType WeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    float Duration = 300.0f; // 5 minutes default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    FVector AffectedArea = FVector(10000.0f, 10000.0f, 2000.0f);

    FVFX_WeatherSettings()
    {
        WeatherType = EVFX_WeatherType::Clear;
        Intensity = 0.5f;
        Duration = 300.0f;
        AffectedArea = FVector(10000.0f, 10000.0f, 2000.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_WeatherController : public AActor
{
    GENERATED_BODY()

public:
    AVFX_WeatherController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Weather VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UParticleSystemComponent* RainParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UParticleSystemComponent* FogParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UParticleSystemComponent* SandstormParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UParticleSystemComponent* SnowParticles;

    // Weather Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control")
    FVFX_WeatherSettings CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control")
    bool bAutoWeatherCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control")
    float WeatherTransitionSpeed = 1.0f;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    float CurrentWeatherTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    bool bIsTransitioning = false;

public:
    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeather(EVFX_WeatherType NewWeatherType, float Intensity = 0.5f, float Duration = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void StartWeatherTransition(const FVFX_WeatherSettings& NewWeatherSettings);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void StopAllWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeatherIntensity(float NewIntensity);

    UFUNCTION(BlueprintPure, Category = "Weather State")
    EVFX_WeatherType GetCurrentWeatherType() const { return CurrentWeather.WeatherType; }

    UFUNCTION(BlueprintPure, Category = "Weather State")
    float GetWeatherIntensity() const { return CurrentWeather.Intensity; }

    UFUNCTION(BlueprintPure, Category = "Weather State")
    bool IsWeatherActive() const { return CurrentWeather.WeatherType != EVFX_WeatherType::Clear; }

protected:
    // Internal weather management
    void UpdateWeatherEffects(float DeltaTime);
    void ActivateWeatherParticles(EVFX_WeatherType WeatherType, float Intensity);
    void DeactivateAllParticles();
    void UpdateParticleIntensity(UParticleSystemComponent* ParticleComp, float Intensity);
    
    // Auto weather cycle
    void ProcessAutoWeatherCycle(float DeltaTime);
    EVFX_WeatherType GetRandomWeatherType() const;
    
    // Biome-specific weather
    EVFX_WeatherType GetBiomeAppropriateWeather() const;
    EBiomeType GetCurrentBiome() const;
};