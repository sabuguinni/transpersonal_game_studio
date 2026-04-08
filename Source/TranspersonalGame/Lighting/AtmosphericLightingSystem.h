// Copyright Transpersonal Game Studio. All Rights Reserved.
// Atmospheric Lighting System - Core atmospheric rendering control
// Agent #8 - Lighting & Atmosphere Agent

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "AtmosphericLightingSystem.generated.h"

/**
 * Core atmospheric lighting system that manages:
 * - Sky atmosphere component configuration
 * - Sun/moon directional lighting
 * - Sky light real-time capture
 * - Volumetric clouds and fog
 * - Atmospheric scattering parameters
 * 
 * This system provides the foundation for all atmospheric rendering
 * and works with Lumen for dynamic global illumination.
 */

UENUM(BlueprintType)
enum class EAtmosphericQuality : uint8
{
    Mobile      UMETA(DisplayName = "Mobile - Optimized for mobile platforms"),
    Console     UMETA(DisplayName = "Console - Next-gen console quality"),
    PC_High     UMETA(DisplayName = "PC High - High-end PC quality"),
    Cinematic   UMETA(DisplayName = "Cinematic - Maximum quality")
};

USTRUCT(BlueprintType)
struct FAtmosphereParameters
{
    GENERATED_BODY()

    // Planet properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float GroundRadius = 6360.0f; // Earth-like in kilometers

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float AtmosphereHeight = 60.0f; // Atmosphere height in kilometers

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    FLinearColor GroundAlbedo = FLinearColor(0.3f, 0.25f, 0.2f, 1.0f); // Prehistoric earth tones

    // Rayleigh scattering (air molecules)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rayleigh Scattering")
    float RayleighScatteringScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rayleigh Scattering")
    FLinearColor RayleighScattering = FLinearColor(0.005802f, 0.013558f, 0.033100f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rayleigh Scattering")
    float RayleighExponentialDistribution = 8.0f;

    // Mie scattering (aerosols, dust)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mie Scattering")
    float MieScatteringScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mie Scattering")
    FLinearColor MieScattering = FLinearColor(0.003996f, 0.003996f, 0.003996f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mie Scattering")
    float MieAnisotropy = 0.8f; // Forward scattering

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mie Scattering")
    float MieExponentialDistribution = 1.2f;

    // Absorption (ozone layer)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Absorption")
    float AbsorptionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Absorption")
    FLinearColor Absorption = FLinearColor(0.000650f, 0.001881f, 0.000085f, 1.0f);

    // Multi-scattering
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Scattering")
    float MultiScattering = 1.0f;

    // Sky luminance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLuminanceFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float AerialPerspectiveDistanceScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FSunLightParameters
{
    GENERATED_BODY()

    // Sun position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
    float SunElevationAngle = 45.0f; // Degrees above horizon

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
    float SunAzimuthAngle = 180.0f; // Degrees from north

    // Sun light properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float SunIntensity = 10.0f; // Lux

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float SunSourceAngle = 0.5f; // Degrees for soft shadows

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float SunTemperature = 5500.0f; // Kelvin

    // Shadow settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    bool bCastCloudShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float CloudShadowStrength = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float CloudShadowExtent = 50000.0f; // Shadow distance in cm
};

USTRUCT(BlueprintType)
struct FSkyLightParameters
{
    GENERATED_BODY()

    // Sky light properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Light")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Light")
    FLinearColor SkyLightColor = FLinearColor::White;

    // Real-time capture settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real-Time Capture")
    bool bRealTimeCapture = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real-Time Capture")
    float CaptureDistance = 1000000.0f; // Very far for sky capture

    // Cloud ambient occlusion
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud AO")
    float CloudAmbientOcclusionStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud AO")
    float CloudAmbientOcclusionExtent = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud AO")
    float CloudAmbientOcclusionMapResolutionScale = 1.0f;
};

UCLASS(ClassGroup=(Lighting), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAtmosphericLightingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAtmosphericLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE ATMOSPHERIC CONTROL ===
    
    /** Initialize the atmospheric lighting system */
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void InitializeAtmosphericSystem();

    /** Update all atmospheric parameters */
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateAtmosphericParameters();

    /** Set atmospheric quality level */
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetAtmosphericQuality(EAtmosphericQuality Quality);

    // === SUN CONTROL ===

    /** Set sun position by elevation and azimuth angles */
    UFUNCTION(BlueprintCallable, Category = "Sun Control")
    void SetSunPosition(float ElevationDegrees, float AzimuthDegrees);

    /** Update sun light properties */
    UFUNCTION(BlueprintCallable, Category = "Sun Control")
    void UpdateSunLight(const FSunLightParameters& SunParams);

    /** Get current sun direction vector */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sun Control")
    FVector GetSunDirection() const;

    // === ATMOSPHERE CONTROL ===

    /** Update atmosphere scattering parameters */
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphereScattering(const FAtmosphereParameters& AtmosphereParams);

    /** Set atmosphere haziness (affects visibility and scattering) */
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphereHaziness(float Haziness);

    // === SKY LIGHT CONTROL ===

    /** Update sky light parameters */
    UFUNCTION(BlueprintCallable, Category = "Sky Light")
    void UpdateSkyLight(const FSkyLightParameters& SkyParams);

    /** Force sky light recapture */
    UFUNCTION(BlueprintCallable, Category = "Sky Light")
    void RecaptureSkyLight();

    // === COMPONENT REFERENCES ===

    /** Get or create sky atmosphere component */
    UFUNCTION(BlueprintCallable, Category = "Components")
    class USkyAtmosphereComponent* GetSkyAtmosphereComponent();

    /** Get or create sun directional light */
    UFUNCTION(BlueprintCallable, Category = "Components")
    class ADirectionalLight* GetSunLight();

    /** Get or create sky light */
    UFUNCTION(BlueprintCallable, Category = "Components")
    class ASkyLight* GetSkyLight();

    // === CONFIGURATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EAtmosphericQuality AtmosphericQuality = EAtmosphericQuality::Console;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FAtmosphereParameters AtmosphereParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FSunLightParameters SunLightParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FSkyLightParameters SkyLightParameters;

    // === PERFORMANCE SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableTemporalUpsampling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TraceSampleCountScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableFastSkyLUT = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAerialPerspective = true;

private:
    // Component references
    UPROPERTY()
    class USkyAtmosphereComponent* SkyAtmosphereComponent;

    UPROPERTY()
    class ADirectionalLight* SunDirectionalLight;

    UPROPERTY()
    class ASkyLight* SkyLightActor;

    // Internal state
    bool bIsInitialized = false;
    float LastUpdateTime = 0.0f;

    // === INTERNAL METHODS ===
    void CreateAtmosphericComponents();
    void ConfigureAtmosphereComponent();
    void ConfigureSunLight();
    void ConfigureSkyLight();
    void ApplyQualitySettings();
    void UpdatePerformanceSettings();
    FRotator CalculateSunRotation(float ElevationDegrees, float AzimuthDegrees) const;
};