#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Light_CretaceousLightingMaster.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn       UMETA(DisplayName = "Dawn"),
    Morning    UMETA(DisplayName = "Morning"),
    Midday     UMETA(DisplayName = "Midday"),
    Afternoon  UMETA(DisplayName = "Afternoon"),
    Sunset     UMETA(DisplayName = "Sunset"),
    Twilight   UMETA(DisplayName = "Twilight"),
    Night      UMETA(DisplayName = "Night"),
    Midnight   UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherCondition : uint8
{
    Clear      UMETA(DisplayName = "Clear"),
    Cloudy     UMETA(DisplayName = "Cloudy"),
    Overcast   UMETA(DisplayName = "Overcast"),
    Foggy      UMETA(DisplayName = "Foggy"),
    Stormy     UMETA(DisplayName = "Stormy"),
    Humid      UMETA(DisplayName = "Humid")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_LightingConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 30.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float VolumetricScattering = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    FLinearColor VolumetricAlbedo = FLinearColor(0.95f, 0.9f, 0.8f, 1.0f);

    FLight_LightingConfiguration()
    {
        // Default Cretaceous atmosphere values
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeLighting
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AttenuationRadius = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bCastVolumetricShadows = true;

    FLight_BiomeLighting()
    {
        // Default forest lighting
    }
};

/**
 * Master lighting controller for Cretaceous period atmosphere
 * Manages dynamic day/night cycle, weather effects, and biome-specific lighting
 * Integrates with UE5 Lumen for realistic global illumination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_CretaceousLightingMaster : public AActor
{
    GENERATED_BODY()

public:
    ALight_CretaceousLightingMaster();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootComp;

    // Lighting configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Configuration")
    FLight_LightingConfiguration LightingConfig;

    // Time and weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherCondition CurrentWeather = ELight_WeatherCondition::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    // Biome lighting configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    TArray<FLight_BiomeLighting> BiomeLightingConfigs;

    // Lumen settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableLumen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableScreenSpaceReflections = true;

    // Actor references
    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    class ADirectionalLight* SunActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    class ASkyLight* SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    class AExponentialHeightFog* AtmosphericFogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    TArray<class APointLight*> BiomeLights;

public:
    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetWeatherCondition(ELight_WeatherCondition NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateLightingForTimeOfDay();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateAtmosphericEffects();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void CreateBiomeLighting();

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void EnableLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void ConfigureLumenSettings();

    UFUNCTION(BlueprintPure, Category = "Time")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintPure, Category = "Time")
    float GetDayNightCycleProgress() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void InitializeLightingSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void FindExistingLightingActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void CreateMissingLightingActors();

protected:
    // Internal methods
    void UpdateSunLighting();
    void UpdateSkyLighting();
    void UpdateFogSettings();
    void UpdateBiomeLighting();
    void InterpolateLightingSettings(float Alpha);
    FLight_LightingConfiguration GetLightingConfigForTime(ELight_TimeOfDay TimeOfDay) const;
    FLinearColor GetSunColorForTime(float TimeOfDay) const;
    float GetSunIntensityForTime(float TimeOfDay) const;
    FRotator GetSunRotationForTime(float TimeOfDay) const;

private:
    // Internal state
    float TimeAccumulator = 0.0f;
    ELight_TimeOfDay PreviousTimeOfDay = ELight_TimeOfDay::Midday;
    bool bLightingSystemInitialized = false;
    bool bLumenEnabled = false;
};