#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_EnvironmentalEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Dust        UMETA(DisplayName = "Dust Storm"),
    VolcanicAsh UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FVFX_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EVFX_WeatherType WeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f; // 5 minutes default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float WindSpeed = 10.0f;

    FVFX_WeatherSettings()
    {
        WeatherType = EVFX_WeatherType::Clear;
        Intensity = 0.5f;
        Duration = 300.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        WindSpeed = 10.0f;
    }
};

USTRUCT(BlueprintType)
struct FVFX_BiomeEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EVFX_WeatherType> AllowedWeatherTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientDustLevel = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AtmosphereColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VisibilityRange = 10000.0f;

    FVFX_BiomeEffects()
    {
        BiomeType = EBiomeType::Savana;
        AllowedWeatherTypes = {EVFX_WeatherType::Clear, EVFX_WeatherType::Rain};
        AmbientDustLevel = 0.1f;
        AtmosphereColor = FLinearColor::White;
        VisibilityRange = 10000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_EnvironmentalEffectsManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EnvironmentalEffectsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVFX_WeatherSettings CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TArray<FVFX_BiomeEffects> BiomeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionTime = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAutoWeatherCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherCycleInterval = 900.0f; // 15 minutes

    // VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* RainEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* DustEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* VolcanicAshEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* FogEffect;

    // Timers
    float WeatherTimer = 0.0f;
    float TransitionTimer = 0.0f;
    bool bTransitioning = false;
    FVFX_WeatherSettings TargetWeather;

public:
    // Weather control functions
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EVFX_WeatherType NewWeatherType, float NewIntensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(EVFX_WeatherType NewWeatherType, float TransitionDuration = 60.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EVFX_WeatherType GetCurrentWeatherType() const { return CurrentWeather.WeatherType; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    float GetWeatherIntensity() const { return CurrentWeather.Intensity; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetBiomeWeatherSettings(EBiomeType Biome, const FVFX_BiomeEffects& Settings);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    FVFX_BiomeEffects GetBiomeWeatherSettings(EBiomeType Biome) const;

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void CreateVolcanicAshCloud(FVector Location, float Radius = 5000.0f, float Duration = 600.0f);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void CreateDustStorm(FVector Location, FVector Direction, float Speed = 20.0f);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void CreateRainEffect(float Intensity = 0.5f, FVector Area = FVector(10000.0f));

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void CreateFogBank(FVector Location, float Density = 0.3f, float Coverage = 8000.0f);

protected:
    void UpdateWeatherEffects();
    void UpdateBiomeAtmosphere();
    void ProcessWeatherTransition(float DeltaTime);
    void InitializeBiomeSettings();
    void ActivateWeatherEffect(EVFX_WeatherType WeatherType, float Intensity);
    void DeactivateAllWeatherEffects();
    FVFX_BiomeEffects* FindBiomeSettings(EBiomeType Biome);
    EBiomeType GetCurrentBiome() const;
};