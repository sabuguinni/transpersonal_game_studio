#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/DirectionalLight.h"
#include "Particles/ParticleSystemComponent.h"
#include "SharedTypes.h"
#include "World_AdvancedWeatherController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Visibility;

    FWorld_WeatherState()
        : CurrentWeather(EWeatherType::Clear)
        , Intensity(0.0f)
        , WindSpeed(5.0f)
        , Temperature(20.0f)
        , Humidity(50.0f)
        , Visibility(10000.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StormData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    FVector StormCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    float StormRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    float LightningFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    float RainIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    float MovementSpeed;

    FWorld_StormData()
        : StormCenter(FVector::ZeroVector)
        , StormRadius(5000.0f)
        , LightningFrequency(2.0f)
        , RainIntensity(0.8f)
        , Duration(600.0f)
        , MovementSpeed(100.0f)
    {}
};

/**
 * Advanced Weather Controller for dynamic atmospheric effects
 * Manages weather transitions, storms, and environmental atmosphere
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_AdvancedWeatherController : public AActor
{
    GENERATED_BODY()

public:
    AWorld_AdvancedWeatherController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WeatherControllerMesh;

    // Weather State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather State")
    FWorld_WeatherState CurrentWeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather State")
    FWorld_WeatherState TargetWeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather State")
    float WeatherTransitionSpeed;

    // Storm System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm System")
    FWorld_StormData ActiveStorm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm System")
    bool bStormActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm System")
    float StormSpawnChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm System")
    float TimeBetweenStorms;

    // Weather Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
    TObjectPtr<AExponentialHeightFog> WeatherFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
    TObjectPtr<ADirectionalLight> WeatherLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
    TArray<UParticleSystemComponent*> RainSystems;

    // Performance Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxRenderBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxParticleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance;

public:
    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeatherType(EWeatherType NewWeatherType, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void StartStorm(const FWorld_StormData& StormData);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void EndStorm(float FadeTime = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void UpdateWeatherIntensity(float NewIntensity);

    // Weather Query Functions
    UFUNCTION(BlueprintPure, Category = "Weather Query")
    FWorld_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintPure, Category = "Weather Query")
    bool IsStormActive() const { return bStormActive; }

    UFUNCTION(BlueprintPure, Category = "Weather Query")
    float GetWeatherIntensityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "Weather Query")
    FVector GetWindDirection() const;

    // Performance Functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeWeatherEffects(float TargetFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetWeatherLOD(int32 LODLevel);

private:
    // Internal Update Functions
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateStormSystem(float DeltaTime);
    void UpdateWeatherEffects();
    void UpdatePerformanceMetrics();

    // Effect Management
    void CreateWeatherEffects();
    void UpdateRainEffects();
    void UpdateFogEffects();
    void UpdateLightingEffects();

    // Storm Management
    void SpawnLightning();
    void UpdateStormMovement(float DeltaTime);
    bool ShouldSpawnStorm() const;

    // Performance Optimization
    void AdjustEffectQuality(float PerformanceRatio);
    void CullDistantEffects();

    // Timers and State
    float LastStormTime;
    float WeatherTransitionTimer;
    float StormTimer;
    float LightningTimer;
    float PerformanceCheckTimer;
};