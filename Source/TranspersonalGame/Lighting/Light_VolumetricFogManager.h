#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "TranspersonalGame.h"
#include "Light_VolumetricFogManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_VolumetricFogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    FColor FogInscatteringColor = FColor(180, 200, 255, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    FColor VolumetricFogAlbedo = FColor(240, 240, 255, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogExtinctionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float SunVolumetricScatteringIntensity = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float SkyLightVolumetricIntensity = 1.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_VolumetricFogManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_VolumetricFogManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Settings")
    FLight_VolumetricFogSettings VolumetricSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Fog References")
    class AExponentialHeightFog* FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    class ASkyLight* SkyLight;

public:
    UFUNCTION(BlueprintCallable, Category = "Volumetric Fog")
    void InitializeVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Volumetric Fog")
    void UpdateVolumetricSettings(const FLight_VolumetricFogSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Fog")
    void SetFogDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Fog")
    void SetFogHeightFalloff(float NewFalloff);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Fog")
    void SetVolumetricScatteringIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Fog")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Volumetric Fog")
    void SaveAtmosphericSettings();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorApplyVolumetricFog();

protected:
    UFUNCTION()
    void FindExistingLightingActors();

    UFUNCTION()
    void CreateVolumetricFogActor();

    UFUNCTION()
    void ConfigureSunLight();

    UFUNCTION()
    void ConfigureSkyLight();

    UFUNCTION()
    void ApplyFogSettings();
};