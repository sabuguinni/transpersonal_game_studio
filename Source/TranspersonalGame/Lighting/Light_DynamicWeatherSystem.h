#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "SharedTypes.h"
#include "Light_DynamicWeatherSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear           UMETA(DisplayName = "Clear Skies"),
    PartlyCloudy    UMETA(DisplayName = "Partly Cloudy"),
    Overcast        UMETA(DisplayName = "Overcast"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Storm           UMETA(DisplayName = "Thunderstorm"),
    Fog             UMETA(DisplayName = "Dense Fog"),
    Volcanic        UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float WhiteTemperature = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float WhiteTint = -0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    FVector4 ColorSaturation = FVector4(1.1f, 1.05f, 0.95f, 1.0f);

    FLight_WeatherSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        SkyLightIntensity = 1.0f;
        FogDensity = 0.02f;
        FogHeightFalloff = 0.1f;
        FogInscatteringColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
        WhiteTemperature = 6000.0f;
        WhiteTint = -0.1f;
        ColorSaturation = FVector4(1.1f, 1.05f, 0.95f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_DynamicWeatherSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_DynamicWeatherSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState TargetWeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAutoWeatherTransition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float AutoTransitionInterval = 300.0f; // 5 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TimeSinceLastTransition = 0.0f;

    // Weather presets for different states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    TMap<ELight_WeatherState, FLight_WeatherSettings> WeatherPresets;

    // Current interpolated settings
    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    FLight_WeatherSettings CurrentSettings;

    // References to lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ASkyLight* SkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class APostProcessVolume* PostProcessVolume;

public:
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(ELight_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(ELight_WeatherState NewWeatherState, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    ELight_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    float GetWeatherTransitionProgress() const { return WeatherTransitionProgress; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void InitializeWeatherPresets();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ApplyWeatherSettings(const FLight_WeatherSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    FLight_WeatherSettings InterpolateWeatherSettings(const FLight_WeatherSettings& From, const FLight_WeatherSettings& To, float Alpha);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Weather")
    void PreviewWeatherState();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Weather")
    void SaveCurrentLightingState();

private:
    void UpdateWeatherTransition(float DeltaTime);
    void TriggerRandomWeatherTransition();
    ELight_WeatherState GetRandomWeatherState();
    float CalculateWeatherProbability(ELight_WeatherState WeatherState);
};