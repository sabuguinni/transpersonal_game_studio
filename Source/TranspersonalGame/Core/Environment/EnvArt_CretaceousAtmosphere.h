#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "TranspersonalGame.h"
#include "EnvArt_CretaceousAtmosphere.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_CretaceousTimeOfDay : uint8
{
    EarlyMorning    UMETA(DisplayName = "Early Morning (6-8 AM)"),
    MidMorning      UMETA(DisplayName = "Mid Morning (8-10 AM)"),
    Midday          UMETA(DisplayName = "Midday (10 AM-2 PM)"),
    Afternoon       UMETA(DisplayName = "Afternoon (2-5 PM)"),
    GoldenHour      UMETA(DisplayName = "Golden Hour (5-7 PM)"),
    Dusk            UMETA(DisplayName = "Dusk (7-8 PM)")
};

UENUM(BlueprintType)
enum class EEnvArt_CretaceousWeather : uint8
{
    Clear           UMETA(DisplayName = "Clear Tropical"),
    PartlyCloudy    UMETA(DisplayName = "Partly Cloudy"),
    Overcast        UMETA(DisplayName = "Overcast Humid"),
    LightRain       UMETA(DisplayName = "Light Tropical Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Monsoon"),
    Misty           UMETA(DisplayName = "Morning Mist")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_CretaceousLightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphericDensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float HumidityLevel = 0.75f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_CretaceousParticleSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float PollenDensity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float SporeDensity = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float DustDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float InsectActivity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    FVector WindDirection = FVector(1.0f, 0.5f, 0.1f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float WindStrength = 2.0f;
};

/**
 * Cretaceous Atmosphere Controller
 * Manages scientifically accurate atmospheric conditions for the Cretaceous period
 * Handles lighting, weather, particles, and ambient effects for prehistoric realism
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousAtmosphere : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousAtmosphere();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Atmospheric Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    EEnvArt_CretaceousTimeOfDay CurrentTimeOfDay = EEnvArt_CretaceousTimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    EEnvArt_CretaceousWeather CurrentWeather = EEnvArt_CretaceousWeather::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    FEnvArt_CretaceousLightingSettings LightingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    FEnvArt_CretaceousParticleSettings ParticleSettings;

    // Lighting References
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ADirectionalLight* MainSunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ASkyLight* SkyLightActor;

    // Particle Systems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
    UParticleSystemComponent* PollenParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
    UParticleSystemComponent* SporeParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
    UParticleSystemComponent* DustParticles;

    // Atmospheric Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TropicalHumidity = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "20.0", ClampMax = "40.0"))
    float TemperatureCelsius = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float VisibilityKilometers = 8.0f;

    // Dynamic Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
    bool bEnableTimeProgression = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
    float TimeProgressionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
    bool bEnableWeatherChanges = true;

public:
    // Atmospheric Control Functions
    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void SetTimeOfDay(EEnvArt_CretaceousTimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void SetWeatherCondition(EEnvArt_CretaceousWeather NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void UpdateLightingForTimeOfDay();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void UpdateParticlesForWeather();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void ApplyCretaceousLighting();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void SpawnAtmosphericParticles();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void SetTropicalAtmosphere();

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Cretaceous Atmosphere")
    FLinearColor GetCurrentSunColor() const;

    UFUNCTION(BlueprintPure, Category = "Cretaceous Atmosphere")
    float GetCurrentHumidity() const;

    UFUNCTION(BlueprintPure, Category = "Cretaceous Atmosphere")
    bool IsGoldenHour() const;

private:
    // Internal Update Functions
    void UpdateAtmosphericConditions();
    void FindOrCreateLightingActors();
    void ConfigureParticleSystems();
    void ApplyScientificLighting();
    void UpdateVolumetricFog();

    // Time Progression
    float CurrentTimeHours = 17.0f; // Start at golden hour
    float WeatherTransitionTimer = 0.0f;
    
    // Lighting Interpolation
    FEnvArt_CretaceousLightingSettings TargetLightingSettings;
    float LightingTransitionSpeed = 2.0f;
};