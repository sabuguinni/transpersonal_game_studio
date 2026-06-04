#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Light_DynamicLightingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDay
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float Hours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float Minutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float Seconds;

    FLight_TimeOfDay()
        : Hours(12.0f), Minutes(0.0f), Seconds(0.0f) {}

    float GetTotalHours() const { return Hours + (Minutes / 60.0f) + (Seconds / 3600.0f); }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength;

    FLight_WeatherState()
        : CloudCoverage(0.3f), RainIntensity(0.0f), FogDensity(0.1f), WindStrength(0.5f) {}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_DynamicLightingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_DynamicLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Time System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    FLight_TimeOfDay CurrentTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLight_WeatherState CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed;

    // Lighting Components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    UMaterialParameterCollection* LightingParameters;

    // Atmosphere Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor DawnColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor DayColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor DuskColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor NightColor;

    // Fire Pit Lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    TArray<APointLight*> FirePitLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float FireLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float FireLightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    FLinearColor FireLightColor;

    // Cave Lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    TArray<APointLight*> CaveLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float CaveAmbientIntensity;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeOfDay(float Hours, float Minutes = 0.0f, float Seconds = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    float GetTimeOfDayNormalized() const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(const FLight_WeatherState& NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(const FLight_WeatherState& TargetWeather, float TransitionTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphereColors();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Fire Lighting")
    void RegisterFirePit(APointLight* FireLight);

    UFUNCTION(BlueprintCallable, Category = "Fire Lighting")
    void UnregisterFirePit(APointLight* FireLight);

    UFUNCTION(BlueprintCallable, Category = "Fire Lighting")
    void UpdateFireLighting();

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void RegisterCaveLight(APointLight* CaveLight);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void UpdateCaveLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting", CallInEditor)
    void InitializeLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Lighting", CallInEditor)
    void FixAtmosphereSettings();

private:
    void UpdateTimeOfDay(float DeltaTime);
    void FindLightingActors();
    FLinearColor InterpolateAtmosphereColor() const;
    float CalculateSunAngle() const;
    void ApplyWeatherToLighting();
    void UpdateFireFlicker();

    // Timer handles
    FTimerHandle WeatherUpdateTimer;
    FTimerHandle FireFlickerTimer;

    // Weather transition
    FLight_WeatherState TargetWeather;
    FLight_WeatherState StartWeather;
    float WeatherTransitionProgress;
    bool bIsTransitioningWeather;
};