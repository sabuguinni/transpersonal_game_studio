#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "CretaceousAtmosphere.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    FLinearColor SunColor = FLinearColor(1.0f, 0.85f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    float SunElevation = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    float SunAzimuth = 135.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    float MieScattering = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    float AtmosphereHeight = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousAtmosphere : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousAtmosphere();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    FLight_AtmosphericSettings AtmosphereSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void ConfigureDirectionalLight();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void ConfigureSkyAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void ConfigureHeightFog();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void SetTimeOfDay(float TimeHours);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    FLinearColor GetCretaceousSunColor(float TimeHours) const;

private:
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY()
    AExponentialHeightFog* HeightFogActor;

    bool bAtmosphereConfigured = false;
};