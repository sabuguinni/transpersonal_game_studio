#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "SharedTypes.h"
#include "Light_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeLightingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor AmbientTint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float AtmosphericDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float FogIntensity;

    FLight_BiomeLightingConfig()
    {
        BiomeName = TEXT("Default");
        BiomeCenter = FVector::ZeroVector;
        AmbientTint = FLinearColor::White;
        AtmosphericDensity = 1.0f;
        FogIntensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CretaceousLightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    float ColorTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    FRotator SunAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    float AtmosphericScattering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    float VolumetricFogDensity;

    FLight_CretaceousLightingSettings()
    {
        SunIntensity = 8.5f;
        ColorTemperature = 3200.0f;
        SunColor = FLinearColor(1.0f, 0.85f, 0.65f, 1.0f);
        SunAngle = FRotator(-25.0f, 45.0f, 0.0f);
        AtmosphericScattering = 0.8f;
        VolumetricFogDensity = 0.02f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Lighting")
    FLight_CretaceousLightingSettings CretaceousSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Lighting")
    TArray<FLight_BiomeLightingConfig> BiomeLightingConfigs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric Lighting")
    ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric Lighting")
    AExponentialHeightFog* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric Lighting")
    APostProcessVolume* PostProcessVolume;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ApplyCretaceousLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ConfigureBiomeLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void EnhanceLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetupVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void CreateDinosaurRimLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateAtmosphericPerspective();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting", CallInEditor)
    void InitializeAtmosphericSystem();

private:
    void FindLightingComponents();
    void SetupDefaultBiomeConfigs();
    void CreateBiomeAtmosphericLights();
    void ApplyLumenEnhancements();

    UPROPERTY()
    TArray<AActor*> BiomeAtmosphericLights;

    UPROPERTY()
    TArray<AActor*> DinosaurRimLights;

    float TimeOfDay;
    bool bIsInitialized;
};