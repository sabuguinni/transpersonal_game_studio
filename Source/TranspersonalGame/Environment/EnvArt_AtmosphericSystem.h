#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "EnvArt_AtmosphericSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereThickness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnableDustParticles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float ParticleDensity = 1.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_AtmosphericSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_AtmosphericSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EEnvArt_WeatherType CurrentWeather = EEnvArt_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    bool bEnableDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    float CurrentTimeOfDayFloat = 0.5f; // 0=midnight, 0.5=noon, 1=midnight

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetWeather(EEnvArt_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void ApplyVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SpawnDustParticles();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateAtmosphericSettings();

private:
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class AExponentialHeightFog* HeightFog;

    UPROPERTY()
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY()
    TArray<class AEmitter*> ParticleEmitters;

    void FindOrCreateLightingActors();
    void UpdateSunLighting();
    void UpdateFogSettings();
    void UpdateParticleEffects();
    void CalculateTimeBasedSettings(float TimeOfDay, FEnvArt_AtmosphericSettings& OutSettings);
};