#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/PostProcessVolume.h"
#include "CretaceousLightingController.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.96f, 0.88f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunElevation = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SkyLuminanceFactor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AerosolDensity = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudLayerBottomAltitude = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudLayerHeight = 4.0f;

    FLight_AtmosphericSettings()
    {
        // Default constructor with Cretaceous tropical settings
    }
};

/**
 * Master lighting controller for Cretaceous period environment
 * Manages realistic tropical atmosphere, day/night cycle, and volumetric lighting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousLightingController : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === CORE COMPONENTS ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class USkyAtmosphereComponent* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UExponentialHeightFogComponent* AtmosphericFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UVolumetricCloudComponent* VolumetricClouds;

    // === DAY/NIGHT CYCLE ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationInSeconds = 1200.0f; // 20 minutes real time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay = 12.0f; // Start at midday

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float TimeScale = 1.0f;

    // === ATMOSPHERIC PRESETS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericSettings MorningSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericSettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericSettings AfternoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericSettings NightSettings;

    // === LUMEN CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableLumenGlobalIllumination = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableLumenReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    int32 MaxLightBounces = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    int32 FinalGatherQuality = 2;

    // === VOLUMETRIC LIGHTING ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    int32 VolumetricFogGridPixelSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    int32 VolumetricFogGridSizeZ = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float VolumetricScatteringIntensity = 1.0f;

public:
    // === PUBLIC INTERFACE ===
    
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyPresetForTimeOfDay(ELight_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void EnableLumenGlobalIllumination(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void EnableVolumetricLighting(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ConfigureCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetDayDuration(float NewDurationInSeconds);

private:
    // === INTERNAL METHODS ===
    
    void InitializeComponents();
    void InitializeAtmosphericPresets();
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateSunPosition();
    void UpdateAtmosphericParameters();
    void ApplyLumenSettings();
    void ApplyVolumetricSettings();
    FLight_AtmosphericSettings InterpolateAtmosphericSettings(const FLight_AtmosphericSettings& A, const FLight_AtmosphericSettings& B, float Alpha) const;
    void FindExistingLightingActors();

    // === CACHED REFERENCES ===
    
    UPROPERTY()
    class ADirectionalLight* CachedDirectionalLight;

    UPROPERTY()
    class ASkyAtmosphere* CachedSkyAtmosphere;

    UPROPERTY()
    class AExponentialHeightFog* CachedHeightFog;

    UPROPERTY()
    class ASkyLight* CachedSkyLight;

    UPROPERTY()
    class AVolumetricCloud* CachedVolumetricCloud;

    UPROPERTY()
    class APostProcessVolume* CachedPostProcessVolume;
};