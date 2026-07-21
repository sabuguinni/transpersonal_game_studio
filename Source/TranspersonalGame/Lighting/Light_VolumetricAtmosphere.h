#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/PostProcessVolume.h"
#include "SharedTypes.h"
#include "Light_VolumetricAtmosphere.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphereSettings
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
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.78f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float VolumetricScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    FLinearColor VolumetricAlbedo = FLinearColor(0.94f, 0.94f, 0.94f, 1.0f);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_VolumetricAtmosphere : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_VolumetricAtmosphere();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings")
    FLight_AtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AActor* ExponentialHeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AActor* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    APostProcessVolume* PostProcessVolume;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetupLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphereSettings(const FLight_AtmosphereSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FindAtmosphereActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void ApplyAtmosphereInEditor();

private:
    void ConfigureDirectionalLight();
    void ConfigureFogComponent();
    void ConfigureSkyAtmosphere();
    void ConfigurePostProcess();
};