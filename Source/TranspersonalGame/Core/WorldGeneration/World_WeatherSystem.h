#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "World_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear Skies"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Sandstorm   UMETA(DisplayName = "Sandstorm"),
    Snow        UMETA(DisplayName = "Snow")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType WeatherType = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float Duration = 300.0f; // 5 minutes default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.5f;

    FWorld_WeatherSettings()
    {
        WeatherType = EWorld_WeatherType::Clear;
        Intensity = 1.0f;
        Duration = 300.0f;
        SunIntensity = 3.0f;
        SunColor = FLinearColor::White;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
        AmbientVolume = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_WeatherSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_WeatherSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Weather Control
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EWorld_WeatherType NewWeather, float NewIntensity = 1.0f, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartRandomWeather();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ClearWeather(float TransitionTime = 3.0f);

    UFUNCTION(BlueprintPure, Category = "Weather")
    EWorld_WeatherType GetCurrentWeather() const { return CurrentWeather.WeatherType; }

    UFUNCTION(BlueprintPure, Category = "Weather")
    float GetWeatherIntensity() const { return CurrentWeather.Intensity; }

    UFUNCTION(BlueprintPure, Category = "Weather")
    bool IsWeatherActive() const { return bWeatherActive; }

    // Biome Integration
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetBiomeWeatherPreferences(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherForBiome(EBiomeType BiomeType);

protected:
    // Core weather state
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    FWorld_WeatherSettings CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    FWorld_WeatherSettings TargetWeather;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    bool bWeatherActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    float WeatherTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    float TransitionTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    float TransitionDuration = 5.0f;

    // Environment references
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    class AExponentialHeightFog* HeightFog;

    // Weather effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    UParticleSystemComponent* RainEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    UParticleSystemComponent* SnowEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    UParticleSystemComponent* SandstormEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* WeatherAudio;

    // Weather presets by biome
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Weather")
    TMap<EBiomeType, TArray<EWorld_WeatherType>> BiomeWeatherPresets;

private:
    // Internal methods
    void InitializeEnvironmentReferences();
    void UpdateWeatherTransition(float DeltaTime);
    void ApplyWeatherSettings(const FWorld_WeatherSettings& Settings);
    void UpdateWeatherEffects();
    void UpdateWeatherAudio();
    FWorld_WeatherSettings GetWeatherPreset(EWorld_WeatherType WeatherType);
    EWorld_WeatherType GetRandomWeatherForBiome(EBiomeType BiomeType);
    void SetupBiomeWeatherPresets();
};