#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "Light_CretaceousAtmosphereManager.generated.h"

/**
 * Manages Cretaceous period atmospheric lighting and weather systems
 * Handles persistent atmospheric settings that survive UE5 editor restarts
 * Integrates with Lumen global illumination for realistic prehistoric lighting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_CretaceousAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_CretaceousAtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Core atmospheric components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class USkyAtmosphereComponent* SkyAtmosphereComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class UExponentialHeightFogComponent* HeightFogComponent;

    // Atmospheric configuration for Cretaceous period
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    FLinearColor CretaceousRayleighScattering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    float CretaceousRayleighScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    FLinearColor CretaceousMieScattering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    float CretaceousMieScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    float CretaceousAbsorptionScale;

    // Fog settings for humid Cretaceous atmosphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float FogHeightFalloff;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    FLinearColor FogInscatteringColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    bool bVolumetricFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float VolumetricFogScatteringDistribution;

    // Directional light references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TObjectPtr<ADirectionalLight> PrimarySunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor CretaceousSunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float CretaceousSunIntensity;

public:
    // Atmospheric management functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyCretaceousSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureSkyAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureHeightFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureDirectionalLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SaveAtmosphericSettings();

    // Atmospheric validation and repair
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    bool ValidateAtmosphericComponents();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void RepairMissingComponents();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void RemoveLegacyAtmosphericFog();

    // Dynamic atmospheric effects
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void SetAtmosphericHumidity(float HumidityLevel);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void SetAtmosphericHaze(float HazeIntensity);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void SetVolumetricFogDensity(float Density);

    // Time of day integration
    UFUNCTION(BlueprintCallable, Category = "Time")
    void UpdateAtmosphereForTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetDawnAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetNoonAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetDuskAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetNightAtmosphere();

protected:
    // Internal helper functions
    void InitializeDefaultCretaceousValues();
    void FindExistingAtmosphericActors();
    void CreateMissingAtmosphericActors();
    void BindToDirectionalLight();

private:
    // Cached references to world atmospheric actors
    TWeakObjectPtr<ASkyAtmosphere> WorldSkyAtmosphere;
    TWeakObjectPtr<AExponentialHeightFog> WorldHeightFog;
    TWeakObjectPtr<ADirectionalLight> WorldDirectionalLight;

    // Atmospheric state tracking
    bool bAtmosphereInitialized;
    bool bComponentsValidated;
    float LastValidationTime;
};