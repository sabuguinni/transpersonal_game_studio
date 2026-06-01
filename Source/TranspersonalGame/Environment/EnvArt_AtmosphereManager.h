#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "TranspersonalGame.h"
#include "EnvArt_AtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor FogColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);

    FEnvArt_TimeOfDaySettings()
    {
        SunAngle = 45.0f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunIntensity = 3.0f;
        SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bEnableDynamicTimeOfDay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FEnvArt_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FEnvArt_TimeOfDaySettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FEnvArt_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FEnvArt_TimeOfDaySettings NightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogExtinctionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor VolumetricFogAlbedo = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyTimeOfDayPreset(const FEnvArt_TimeOfDaySettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetMidDayLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetDuskLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableVolumetricFog(bool bEnable);

protected:
    UFUNCTION()
    void FindAndConfigureLights();

    UFUNCTION()
    void FindAndConfigureFog();

    UFUNCTION()
    void FindAndConfigureSkyAtmosphere();

private:
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyLight* SkyLightActor;

    UPROPERTY()
    class AExponentialHeightFog* HeightFogActor;

    float TimeAccumulator = 0.0f;
    
    FEnvArt_TimeOfDaySettings InterpolateSettings(const FEnvArt_TimeOfDaySettings& A, const FEnvArt_TimeOfDaySettings& B, float Alpha);
    FEnvArt_TimeOfDaySettings GetCurrentTimeOfDaySettings();
};