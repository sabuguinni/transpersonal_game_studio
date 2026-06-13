#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/VolumetricFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/VolumetricFog.h"
#include "Engine/SkyAtmosphere.h"
#include "EnvArt_AtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunElevation = -15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunAzimuth = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bCastVolumetricShadows = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float ScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FColor FogAlbedo = FColor(180, 200, 160);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float ExtinctionScale = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float ViewDistance = 50000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MultiScatteringFactor = 1.2f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FEnvArt_TimeOfDaySettings TimeOfDaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FEnvArt_FogSettings FogSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FEnvArt_AtmosphereSettings AtmosphereSettings;

    // References to scene components
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    AVolumetricFog* ForestFog;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    ASkyAtmosphere* SkyAtmosphere;

public:
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
    void CreateForestFog();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
    void EnhanceAtmosphere();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
    void CreateAtmosphericParticles();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void FindSceneReferences();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetTimeOfDay(float Hours24Format);

    UFUNCTION(BlueprintPure, Category = "Environment")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

private:
    float CurrentTimeOfDay = 16.0f; // 4 PM golden hour

    void UpdateSunPosition();
    void UpdateFogDensity();
    void UpdateAtmosphericScattering();
};