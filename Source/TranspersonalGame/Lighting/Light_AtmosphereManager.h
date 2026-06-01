#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/AtmosphericFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/TriggerVolume.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Light_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Dusty       UMETA(DisplayName = "Dusty")
};

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_BiomeLighting
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor AmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float AmbientIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float VolumetricScattering;

    FLight_BiomeLighting()
    {
        BiomeType = EBiomeType::Savanna;
        AmbientColor = FLinearColor::White;
        AmbientIntensity = 1.0f;
        FogColor = FLinearColor::White;
        FogDensity = 0.1f;
        VolumetricScattering = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    ELight_TimeOfDay TimePhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FRotator SunRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLinearColor SkyColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float AtmosphereOpacity;

    FLight_TimeOfDaySettings()
    {
        TimePhase = ELight_TimeOfDay::Noon;
        SunRotation = FRotator(-45.0f, 0.0f, 0.0f);
        SunColor = FLinearColor::White;
        SunIntensity = 5.0f;
        SkyColor = FLinearColor(0.4f, 0.7f, 1.0f);
        AtmosphereOpacity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType WeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor WeatherTint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightingModifier;

    FLight_WeatherSettings()
    {
        WeatherType = ELight_WeatherType::Clear;
        CloudCoverage = 0.3f;
        FogIntensity = 0.1f;
        WindStrength = 0.5f;
        WeatherTint = FLinearColor::White;
        LightingModifier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Lighting References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class AAtmosphericFog* AtmosphericFog;

    // Time and Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDaySpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bEnableDynamicTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    ELight_WeatherType CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float WeatherTransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bEnableDynamicWeather;

    // Biome Lighting Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    TArray<FLight_BiomeLighting> BiomeLightingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Lighting")
    TArray<FLight_TimeOfDaySettings> TimeOfDaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Lighting")
    TArray<FLight_WeatherSettings> WeatherSettings;

    // Volumetric and Atmospheric Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Lighting")
    float VolumetricFogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Lighting")
    FLinearColor VolumetricFogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Lighting")
    float VolumetricScatteringIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "God Rays")
    TArray<class ASpotLight*> GodRayLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "God Rays")
    float GodRayIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "God Rays")
    bool bEnableGodRays;

    // Biome-Specific Lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Specific")
    TArray<class APointLight*> BiomeAmbientLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Specific")
    TArray<class ATriggerVolume*> WeatherControlVolumes;

public:
    // Core Lighting Functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere Management")
    void InitializeLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Management")
    void SetupBiomeLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Management")
    void ConfigureVolumetricLighting();

    // Time of Day System
    UFUNCTION(BlueprintCallable, Category = "Time System")
    void UpdateTimeOfDay(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    ELight_TimeOfDay GetCurrentTimePhase() const;

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void ApplyTimeOfDayLighting(ELight_TimeOfDay TimePhase);

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateWeatherSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeather(ELight_WeatherType TargetWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void ApplyWeatherEffects(ELight_WeatherType WeatherType);

    // Biome Lighting
    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    void ApplyBiomeLighting(EBiomeType BiomeType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    FLight_BiomeLighting GetBiomeLightingSettings(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    void UpdateBiomeAmbientLighting();

    // Volumetric and Atmospheric Effects
    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void UpdateVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void UpdateGodRays();

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void SetVolumetricFogDensity(float NewDensity);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Utilities")
    void RefreshAllLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting Utilities")
    void OptimizeLightingPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void SetupDefaultLighting();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void CreateBiomeLightingActors();

private:
    // Internal update functions
    void UpdateSunPosition();
    void UpdateAtmosphericScattering();
    void UpdateFogParameters();
    void UpdateBiomeSpecificLighting();

    // Timer handles
    FTimerHandle TimeOfDayTimer;
    FTimerHandle WeatherUpdateTimer;
    FTimerHandle LightingOptimizationTimer;

    // Internal state
    float WeatherTransitionProgress;
    ELight_WeatherType TargetWeather;
    bool bWeatherTransitioning;
};