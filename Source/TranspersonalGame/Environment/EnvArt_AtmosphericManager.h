#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "GameFramework/Actor.h"
#include "EnvArt_AtmosphericManager.generated.h"

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
enum class EEnvArt_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Hazy        UMETA(DisplayName = "Hazy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Dusty       UMETA(DisplayName = "Dusty")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float OtherAbsorptionScale = 0.1f;

    FEnvArt_AtmosphericSettings()
    {
        // Default constructor with initialized values above
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === ATMOSPHERIC CONTROL ===
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetWeatherState(EEnvArt_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyAtmosphericSettings(const FEnvArt_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void TransitionToGoldenHour(float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void EnhanceVolumetricFog(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ConfigureCretaceousAtmosphere();

    // === COMPONENT REFERENCES ===
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void FindAtmosphericActors();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ValidateAtmosphericSetup();

    // === PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvArt_AtmosphericSettings CurrentSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EEnvArt_WeatherState CurrentWeatherState = EEnvArt_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bIsTransitioning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDuration = 5.0f;

private:
    // === CACHED REFERENCES ===
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class AExponentialHeightFog* FogActor;

    UPROPERTY()
    class ASkyAtmosphere* SkyActor;

    // === TRANSITION DATA ===
    FEnvArt_AtmosphericSettings StartSettings;
    FEnvArt_AtmosphericSettings TargetSettings;

    // === INTERNAL METHODS ===
    void UpdateSunLight();
    void UpdateFog();
    void UpdateSkyAtmosphere();
    void UpdateTransition(float DeltaTime);
    FEnvArt_AtmosphericSettings GetSettingsForTimeOfDay(EEnvArt_TimeOfDay TimeOfDay);
    FEnvArt_AtmosphericSettings GetSettingsForWeatherState(EEnvArt_WeatherState WeatherState);
};