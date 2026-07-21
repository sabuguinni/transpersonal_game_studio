#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Perf_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EPerf_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EPerf_WeatherType WeatherType = EPerf_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "50000.0"))
    float FogStartDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyLightColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float SkyLightIntensity = 1.0f;

    FPerf_WeatherSettings()
    {
        WeatherType = EPerf_WeatherType::Clear;
        Intensity = 1.0f;
        SunColor = FLinearColor::White;
        SunIntensity = 3.0f;
        FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
        FogDensity = 0.02f;
        FogStartDistance = 0.0f;
        SkyLightColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
        SkyLightIntensity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_WeatherSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_WeatherSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(EPerf_WeatherType NewWeatherType, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EPerf_WeatherType GetCurrentWeatherType() const { return CurrentWeatherSettings.WeatherType; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    float GetWeatherIntensity() const { return CurrentWeatherSettings.Intensity; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void RandomizeWeather();

    UFUNCTION(BlueprintCallable, Category = "Weather", CallInEditor)
    void RefreshWeatherReferences();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FPerf_WeatherSettings CurrentWeatherSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FPerf_WeatherSettings TargetWeatherSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsTransitioning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionDuration = 2.0f;

    // Weather presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    FPerf_WeatherSettings ClearWeatherPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    FPerf_WeatherSettings CloudyWeatherPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    FPerf_WeatherSettings RainyWeatherPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    FPerf_WeatherSettings StormyWeatherPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    FPerf_WeatherSettings FoggyWeatherPreset;

    // Lighting references
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TWeakObjectPtr<ADirectionalLight> DirectionalLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TWeakObjectPtr<ASkyLight> SkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TWeakObjectPtr<AExponentialHeightFog> HeightFog;

private:
    void InitializeWeatherPresets();
    void FindLightingActors();
    void ApplyWeatherSettings(const FPerf_WeatherSettings& Settings);
    FPerf_WeatherSettings GetWeatherPreset(EPerf_WeatherType WeatherType) const;
    void TickWeatherTransition(float DeltaTime);

    FTimerHandle WeatherUpdateTimer;
};