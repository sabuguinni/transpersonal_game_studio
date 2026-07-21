#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyLightComponent.h"
#include "SharedTypes.h"
#include "Eng_WeatherSystemManager.generated.h"

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_WeatherSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Weather Control
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherType(EEng_WeatherType NewWeather, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetRainIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWindStrength(float Strength);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetFogDensity(float Density);

    // Weather State
    UFUNCTION(BlueprintPure, Category = "Weather System")
    EEng_WeatherType GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintPure, Category = "Weather System")
    float GetRainIntensity() const { return RainIntensity; }

    UFUNCTION(BlueprintPure, Category = "Weather System")
    float GetWindStrength() const { return WindStrength; }

    UFUNCTION(BlueprintPure, Category = "Weather System")
    float GetFogDensity() const { return FogDensity; }

    UFUNCTION(BlueprintPure, Category = "Weather System")
    bool IsTransitioning() const { return bIsTransitioning; }

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SpawnLightning(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateEnvironmentalLighting();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    EEng_WeatherType CurrentWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    EEng_WeatherType TargetWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    float RainIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    float WindStrength;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    float FogDensity;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    bool bIsTransitioning;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    float TransitionProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    float TransitionDuration;

    // Weather Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Config")
    float WeatherUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Config")
    float MaxRainIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Config")
    float MaxWindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Config")
    float MaxFogDensity;

private:
    void UpdateWeatherTransition(float DeltaTime);
    void ApplyWeatherEffects();
    void UpdateLightingForWeather();
    void SpawnWeatherParticles();
    
    float LastWeatherUpdate;
    TWeakObjectPtr<ADirectionalLight> SunLight;
    TWeakObjectPtr<USkyLightComponent> SkyLight;
};