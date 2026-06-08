#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "Light_VolumetricAtmosphereController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AbsorptionScale = 1.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.78f, 0.86f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bVolumetricFog = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_SunSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float Intensity = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor LightColor = FLinearColor(1.0f, 0.96f, 0.84f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float Temperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float VolumetricScatteringIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    bool bCastVolumetricShadow = true;
};

/**
 * Controls volumetric atmospheric lighting for Cretaceous period environments
 * Manages SkyAtmosphere, ExponentialHeightFog, DirectionalLight, and PostProcessVolume
 * Provides realistic prehistoric atmospheric conditions with enhanced volumetric effects
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_VolumetricAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    ALight_VolumetricAtmosphereController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === ATMOSPHERIC CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    FLight_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    FLight_SunSettings SunSettings;

    // === ACTOR REFERENCES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Actors")
    ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Actors")
    ADirectionalLight* SunActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Actors")
    ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Actors")
    AExponentialHeightFog* FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Actors")
    APostProcessVolume* PostProcessActor;

    // === ATMOSPHERIC CONTROL FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ConfigureSkyAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ConfigureDirectionalLight();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ConfigureVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ConfigureSkyLight();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ConfigurePostProcessing();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void FindOrCreateAtmosphericActors();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void SaveAtmosphericSettings();

    // === VALIDATION FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateAtmosphericSetup() const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void LogAtmosphericStatus() const;

protected:
    // === INTERNAL HELPERS ===
    
    void InitializeAtmosphericActors();
    void ApplyAtmosphericSettings();
    bool FindExistingAtmosphericActors();
    void CreateMissingAtmosphericActors();
};