#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "TranspersonalGame/SharedTypes.h"
#include "AtmosphericSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAtmosphericSystem, Log, All);

/**
 * Atmospheric configuration for different biomes
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericBiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    EBiomeType BiomeType = EBiomeType::Savanna;

    // Fog parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.447f, 0.638f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 1.0f;

    // Sky light parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor::White;

    // Volumetric cloud parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudDensity = 0.3f;

    // Wind parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindStrength = 1.0f;

    // Particle effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnableDustParticles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnablePollenParticles = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float ParticleSpawnRate = 10.0f;
};

/**
 * Time of day configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_TimeOfDayConfig
{
    GENERATED_BODY()

    // Sun parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunElevation = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAzimuth = 180.0f;

    // Atmospheric scattering
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereThickness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor GroundAlbedo = FLinearColor(0.3f, 0.25f, 0.2f);

    // Temperature and humidity (affects haze and particles)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Temperature = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Humidity = 0.6f; // 0-1 range
};

/**
 * Manages atmospheric effects, lighting, and environmental ambiance
 * Creates the visual mood and atmosphere for different biomes
 */
UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAtmosphericSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAtmosphericSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === ATMOSPHERIC CONFIGURATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric System")
    float TimeOfDay = 12.0f; // 0-24 hour format

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric System")
    float DayDuration = 1200.0f; // Seconds for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric System")
    bool bEnableDynamicTimeOfDay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric System")
    float AtmosphericUpdateRadius = 10000.0f;

    // === LIGHTING REFERENCES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    USkyLightComponent* SkyLight = nullptr;

    // === BIOME CONFIGURATIONS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configs")
    TMap<EBiomeType, FEnvArt_AtmosphericBiomeConfig> BiomeAtmosphericConfigs;

    // === TIME OF DAY CONFIGURATIONS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Configs")
    FEnvArt_TimeOfDayConfig DawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Configs")
    FEnvArt_TimeOfDayConfig NoonConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Configs")
    FEnvArt_TimeOfDayConfig DuskConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Configs")
    FEnvArt_TimeOfDayConfig NightConfig;

    // === ATMOSPHERIC METHODS ===

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void InitializeAtmosphericSystem();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateTimeOfDay(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateBiomeAtmosphere(EBiomeType BiomeType, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void CreateGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void CreateVolumetricFog(EBiomeType BiomeType, const FVector& FogCenter, float FogRadius);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SpawnAtmosphericParticles(EBiomeType BiomeType, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    EBiomeType GetCurrentBiome(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    FEnvArt_TimeOfDayConfig GetCurrentTimeConfig() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void ApplyWeatherEffects(EWeatherType WeatherType, float Intensity);

private:
    // === INTERNAL METHODS ===

    void InitializeBiomeAtmosphericConfigs();
    void InitializeTimeOfDayConfigs();
    void UpdateSunPosition();
    void UpdateSkyLighting();
    void UpdateFogParameters();
    void InterpolateTimeConfigs(const FEnvArt_TimeOfDayConfig& ConfigA, const FEnvArt_TimeOfDayConfig& ConfigB, float Alpha, FEnvArt_TimeOfDayConfig& OutConfig);

    // === INTERNAL STATE ===

    UPROPERTY()
    TArray<AActor*> AtmosphericActors;

    UPROPERTY()
    EBiomeType CurrentBiome = EBiomeType::Savanna;

    UPROPERTY()
    EBiomeType PreviousBiome = EBiomeType::Savanna;

    float BiomeTransitionTimer = 0.0f;
    float BiomeTransitionDuration = 5.0f;

    // Cached components for performance
    UPROPERTY()
    UExponentialHeightFogComponent* CachedFogComponent = nullptr;

    UPROPERTY()
    UVolumetricCloudComponent* CachedCloudComponent = nullptr;
};