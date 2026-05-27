#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "World_BiomeWeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Sandstorm   UMETA(DisplayName = "Sandstorm"),
    Snow        UMETA(DisplayName = "Snow")
};

UENUM(BlueprintType)
enum class EWorld_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-45.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.5f;

    FWorld_WeatherSettings()
    {
        SunColor = FLinearColor::White;
        SunIntensity = 3.0f;
        SunRotation = FRotator(-45.0f, 0.0f, 0.0f);
        SkyColor = FLinearColor(0.4f, 0.7f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
        RainIntensity = 0.0f;
        WindStrength = 1.0f;
        AmbientVolume = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeWeatherSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeWeatherSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Current weather and time state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    EWorld_WeatherType CurrentWeather = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    EWorld_TimeOfDay CurrentTimeOfDay = EWorld_TimeOfDay::Noon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float CurrentTime = 12.0f; // 0-24 hour format

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle

    // Weather transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Transition")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Transition")
    float WeatherChangeChance = 0.1f; // Per minute chance

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Transition")
    bool bAutoWeatherChanges = true;

    // Biome-specific weather preferences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TMap<FString, float> BiomeWeatherProbabilities;

    // Weather effect references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TObjectPtr<UNiagaraSystem> RainEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TObjectPtr<UNiagaraSystem> SnowEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TObjectPtr<UNiagaraSystem> SandstormEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundCue> RainAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundCue> WindAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundCue> ThunderSound;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNiagaraComponent> ActiveWeatherEffect;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    // Weather control functions
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeather(EWorld_WeatherType NewWeather, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetTimeOfDay(EWorld_TimeOfDay NewTime, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetCurrentTime(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    FWorld_WeatherSettings GetCurrentWeatherSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void ApplyWeatherToEnvironment();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TriggerWeatherChange();

    // Biome integration
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void SetBiomeWeatherPreferences(const FString& BiomeName, const TMap<EWorld_WeatherType, float>& Preferences);

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    EWorld_WeatherType SelectBiomeAppropriateWeather(const FString& BiomeName) const;

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void SpawnWeatherEffect(EWorld_WeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void ClearWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAmbientAudio();

private:
    // Internal state
    float WeatherTransitionTimer = 0.0f;
    float NextWeatherCheckTimer = 0.0f;
    EWorld_WeatherType TargetWeather = EWorld_WeatherType::Clear;
    FWorld_WeatherSettings CurrentSettings;
    FWorld_WeatherSettings TargetSettings;

    // Environment references
    UPROPERTY()
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY()
    TObjectPtr<AExponentialHeightFog> FogActor;

    UPROPERTY()
    TObjectPtr<ASkyAtmosphere> SkyAtmosphere;

    UPROPERTY()
    TObjectPtr<APostProcessVolume> PostProcessVolume;

    // Helper functions
    void FindEnvironmentActors();
    void UpdateSunPosition();
    void UpdateFogSettings();
    void UpdateSkyAtmosphere();
    void UpdatePostProcessing();
    FWorld_WeatherSettings GetWeatherSettings(EWorld_WeatherType WeatherType) const;
    EWorld_TimeOfDay GetTimeOfDayFromHour(float Hour) const;
    void InterpolateWeatherSettings(float Alpha);
};