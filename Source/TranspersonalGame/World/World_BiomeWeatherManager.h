#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "SharedTypes.h"
#include "World_BiomeWeatherManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyTint = FLinearColor::Blue;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeWeatherManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeWeatherManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FWorld_WeatherState CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TArray<FWorld_WeatherState> BiomeWeatherPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableDynamicWeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherUpdateInterval = 30.0f;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherForBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(const FWorld_WeatherState& TargetWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateEnvironmentalEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    FWorld_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartRainEffect(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StopRainEffect();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateLightingSettings();

private:
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class AExponentialHeightFog* WorldFog;

    UPROPERTY()
    float WeatherTimer = 0.0f;

    UPROPERTY()
    FWorld_WeatherState TargetWeather;

    UPROPERTY()
    bool bIsTransitioning = false;

    void InitializeWeatherPresets();
    void FindWorldLightingActors();
    void InterpolateWeatherState(float Alpha);
    FWorld_WeatherState GetBiomeDefaultWeather(EWorld_BiomeType BiomeType);
};