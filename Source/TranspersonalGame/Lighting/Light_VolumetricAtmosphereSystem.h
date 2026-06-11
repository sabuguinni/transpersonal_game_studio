#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Light_VolumetricAtmosphereSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_AtmospherePreset : uint8
{
    CretaceousClear     UMETA(DisplayName = "Cretaceous Clear"),
    CretaceousOvercast  UMETA(DisplayName = "Cretaceous Overcast"),
    CretaceousStorm     UMETA(DisplayName = "Cretaceous Storm"),
    CretaceousFog       UMETA(DisplayName = "Cretaceous Fog"),
    CretaceousSunset    UMETA(DisplayName = "Cretaceous Sunset"),
    CretaceousNight     UMETA(DisplayName = "Cretaceous Night")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogInscatteringColor = FLinearColor(0.45f, 0.55f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricScatteringIntensity = 1.0f;

    FLight_AtmosphereSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogInscatteringColor = FLinearColor(0.45f, 0.55f, 0.6f, 1.0f);
        CloudCoverage = 0.3f;
        VolumetricScatteringIntensity = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_VolumetricAtmosphereSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_VolumetricAtmosphereSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    ELight_AtmospherePreset CurrentPreset = ELight_AtmospherePreset::CretaceousClear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableVolumetricClouds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float TransitionSpeed = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmospherePreset(ELight_AtmospherePreset NewPreset);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FLight_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void TransitionToPreset(ELight_AtmospherePreset TargetPreset, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FLight_AtmosphereSettings GetPresetSettings(ELight_AtmospherePreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateDirectionalLight();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateSkyAtmosphere();

private:
    UPROPERTY()
    ADirectionalLight* DirectionalLightActor;

    UPROPERTY()
    AActor* SkyAtmosphereActor;

    UPROPERTY()
    AActor* VolumetricCloudActor;

    UPROPERTY()
    AActor* ExponentialHeightFogActor;

    FLight_AtmosphereSettings TargetSettings;
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 2.0f;

    void FindAtmosphereActors();
    void InterpolateSettings(float Alpha);
    FLight_AtmosphereSettings GetCurrentInterpolatedSettings();
};