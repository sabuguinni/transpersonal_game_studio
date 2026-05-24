#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/VolumetricCloud.h"
#include "TranspersonalGame/Core/SharedTypes.h"
#include "Light_VolumetricAtmosphereSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieAnisotropy = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AbsorptionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SkyLuminanceFactor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float GroundAlbedo = 0.3f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_VolumetricSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float VolumetricScatteringIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float VolumetricFogDistance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float VolumetricFogStaticLightingScatteringIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    bool bOverrideVolumetricFogLightScatteringColor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics", meta = (EditCondition = "bOverrideVolumetricFogLightScatteringColor"))
    FLinearColor VolumetricFogLightScatteringColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CloudSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float LayerBottomAltitude = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float LayerHeight = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float TracingStartMaxDistance = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float TracingMaxDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float ViewSampleCountScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float ReflectionSampleCountScale = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float ShadowSampleCountScale = 0.25f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_VolumetricAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_VolumetricAtmosphereSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Componentes principais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Configurações de atmosfera
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings")
    FLight_AtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Settings")
    FLight_VolumetricSettings VolumetricSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Settings")
    FLight_CloudSettings CloudSettings;

    // Referências para actores de iluminação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    class ASkyLight* MoonLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere References")
    class AVolumetricCloud* VolumetricClouds;

    // Configurações de bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    EBiomeType CurrentBiome = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TMap<EBiomeType, FLight_AtmosphereSettings> BiomeAtmosphereSettings;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ApplyAtmosphereSettings(const FLight_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ApplyVolumetricSettings(const FLight_VolumetricSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ApplyCloudSettings(const FLight_CloudSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Biome Control")
    void SetBiomeAtmosphere(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void UpdateVolumetricScattering(float ScatteringIntensity);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void SetAtmosphericPerspective(bool bEnabled, float StartDistance = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Cloud Control")
    void SetCloudCoverage(float Coverage);

    UFUNCTION(BlueprintCallable, Category = "Cloud Control")
    void SetCloudOpacity(float Opacity);

protected:
    // Funções internas
    void InitializeBiomeSettings();
    void FindLightingActors();
    void ConfigurePrehistoricAtmosphere();
    void SetupVolumetricLighting();
    void ApplyBiomeSpecificSettings();

    // Configurações por bioma
    FLight_AtmosphereSettings GetSwampAtmosphere() const;
    FLight_AtmosphereSettings GetForestAtmosphere() const;
    FLight_AtmosphereSettings GetSavannaAtmosphere() const;
    FLight_AtmosphereSettings GetDesertAtmosphere() const;
    FLight_AtmosphereSettings GetMountainAtmosphere() const;

private:
    // Estado interno
    bool bSystemInitialized = false;
    float AtmosphereUpdateTimer = 0.0f;
    const float AtmosphereUpdateInterval = 1.0f;
};