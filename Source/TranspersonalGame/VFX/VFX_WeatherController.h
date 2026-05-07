#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "VFX_WeatherController.generated.h"

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear Sky"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Fog             UMETA(DisplayName = "Fog"),
    Dust            UMETA(DisplayName = "Dust Storm"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EVFX_WeatherType WeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float Duration = 300.0f; // 5 minutes default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Visibility = 1.0f;

    FVFX_WeatherSettings()
    {
        WeatherType = EVFX_WeatherType::Clear;
        Intensity = 0.5f;
        Duration = 300.0f;
        SkyTint = FLinearColor::White;
        Visibility = 1.0f;
    }
};

/**
 * VFX Weather Controller - Manages dynamic weather effects for prehistoric environments
 * Handles rain, fog, dust storms, and volcanic ash effects using Niagara systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_WeatherController : public AActor
{
    GENERATED_BODY()

public:
    AVFX_WeatherController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Weather System Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Weather", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* RainSystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Weather", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* FogSystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Weather", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* DustSystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Weather", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* VolcanicAshComponent;

    // Weather Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> FogSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> DustStormSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> VolcanicAshSystem;

    // Current Weather State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control")
    FVFX_WeatherSettings CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control")
    bool bAutoWeatherCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control", meta = (ClampMin = "60.0", ClampMax = "3600.0"))
    float WeatherCycleDuration = 600.0f; // 10 minutes

    // Timing
    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    float CurrentWeatherTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    float TransitionTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control", meta = (ClampMin = "5.0", ClampMax = "60.0"))
    float TransitionDuration = 15.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    bool bIsTransitioning = false;

public:
    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void SetWeather(EVFX_WeatherType NewWeatherType, float Intensity = 0.5f, float Duration = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void StartWeatherTransition(const FVFX_WeatherSettings& NewWeatherSettings);

    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void StopAllWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void SetWeatherIntensity(float NewIntensity);

    UFUNCTION(BlueprintPure, Category = "VFX Weather")
    EVFX_WeatherType GetCurrentWeatherType() const { return CurrentWeather.WeatherType; }

    UFUNCTION(BlueprintPure, Category = "VFX Weather")
    float GetCurrentWeatherIntensity() const { return CurrentWeather.Intensity; }

    UFUNCTION(BlueprintPure, Category = "VFX Weather")
    bool IsWeatherActive() const;

    // Biome-specific weather
    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void ApplyBiomeWeather(const FString& BiomeName);

private:
    void UpdateWeatherEffects();
    void UpdateWeatherCycle(float DeltaTime);
    void ActivateWeatherSystem(EVFX_WeatherType WeatherType, float Intensity);
    void DeactivateWeatherSystem(EVFX_WeatherType WeatherType);
    void LoadWeatherAssets();
    EVFX_WeatherType GetRandomWeatherForBiome(const FString& BiomeName);
};